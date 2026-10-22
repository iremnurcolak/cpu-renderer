#include "obj_renderer.h"
#include "colors.h"
#include "line_renderer.h"
#include "triangle_renderer.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int framebufferWidth = 800;
static const int framebufferHeight = 800;

typedef struct Vertex { float x, y, z; } Vertex;
typedef struct ScreenPoint { int x, y; double z; } ScreenPoint;
typedef void (*FaceDrawer)(const ScreenPoint *, size_t, TGAImage *, TGAImage *);

/**
 * @brief Grow an owned array geometrically, retaining its storage on failure.
 * @param[in] data Current allocation, or NULL.
 * @param[in,out] capacity Current element capacity, updated only on success.
 * @param[in] needed Required element count.
 * @param[in] elementSize Size of each element in bytes.
 * @return Array storage, or NULL if capacity overflows or allocation fails.
 */
static void *growArray(void *data, size_t *capacity, size_t needed, size_t elementSize)
{
    if(needed <= *capacity)
        return data;
    size_t next = *capacity ? *capacity : 16;
    while(next < needed)
    {
        if(next > SIZE_MAX / 2)
            return NULL;
        next *= 2;
    }
    if(next > SIZE_MAX / elementSize)
        return NULL;
    void *grown = realloc(data, next * elementSize);
    if(grown)
        *capacity = next;
    return grown;
}

/**
 * @brief Read one complete OBJ line without a fixed line-length limit.
 * @param[in] file Input stream.
 * @param[in,out] line Owned line buffer, initially NULL.
 * @param[in,out] capacity Buffer capacity in bytes.
 * @return 1 for a line, 0 at end of file, or -1 for an allocation or read failure.
 */
static int readObjLine(FILE *file, char **line, size_t *capacity)
{
    size_t length = 0;
    int character;
    while((character = fgetc(file)) != EOF)
    {
        if(length > SIZE_MAX - 2)
            return -1;
        char *grown = growArray(*line, capacity, length + 2, sizeof(char));
        if(!grown)
            return -1;
        *line = grown;
        if(character == '\n')
            break;
        (*line)[length++] = (char)character;
    }
    if(ferror(file))
        return -1;
    if(character == EOF && length == 0)
        return 0;
    (*line)[length] = '\0';
    return 1;
}

/**
 * @brief Copy a null-terminated string into newly allocated storage.
 * @param[in] text Source string.
 * @return Owned copy or NULL on allocation failure.
 */
static char *copyString(const char *text)
{
    const size_t length = strlen(text) + 1;
    char *copy = malloc(length);
    if(copy)
        memcpy(copy, text, length);
    return copy;
}

/**
 * @brief Resolve explicit paths or search obj/ and obj/<stem>/ for a filename.
 * @param[in] filename Model filename or explicit relative/absolute path.
 * @return Owned path, or NULL on allocation failure.
 */
static char *resolveModelPath(const char *filename)
{
    if(strchr(filename, '/') || strchr(filename, '\\'))
        return copyString(filename);
    const size_t length = strlen(filename);
    if(length > (SIZE_MAX - 6) / 2)
        return NULL;
    char *path = malloc(length + 5);
    if(!path)
        return NULL;
    snprintf(path, length + 5, "obj/%s", filename);
    FILE *probe = fopen(path, "rb");
    if(probe)
    {
        fclose(probe);
        return path;
    }
    free(path);
    const char *extension = strrchr(filename, '.');
    const size_t stemLength = extension && extension != filename
        ? (size_t)(extension - filename) : length;
    path = malloc(4 + stemLength + 1 + length + 1);
    if(path)
    {
        memcpy(path, "obj/", 4);
        memcpy(path + 4, filename, stemLength);
        path[4 + stemLength] = '/';
        memcpy(path + 5 + stemLength, filename, length + 1);
    }
    return path;
}

/**
 * @brief Draw a closed polygon boundary in white.
 * @param[in] points Ordered projected face vertices.
 * @param[in] count Number of points, at least three.
 * @param[in,out] framebuffer Destination image.
 * @param[in,out] zbuffer Unused in wireframe mode; may be NULL.
 */
static void drawWireframeFace(const ScreenPoint *points, size_t count,
    TGAImage *framebuffer, TGAImage *zbuffer)
{
    (void)zbuffer;
    ScreenPoint previous = points[0];
    for(size_t i = 1; i < count; ++i)
    {
        drawLineInterpolated(previous.x, previous.y, points[i].x, points[i].y, framebuffer, white);
        previous = points[i];
    }
    drawLineInterpolated(previous.x, previous.y, points[0].x, points[0].y, framebuffer, white);
}

/**
 * @brief Triangulate a convex face as a fan with a random RGB color per triangle.
 * @param[in] points Ordered projected vertices with mapped depth values.
 * @param[in] count Number of points, at least three.
 * @param[in,out] framebuffer Destination RGB image.
 * @param[in,out] zbuffer Optional grayscale depth image, or NULL.
 * @note Colors are written before depth comparisons, preserving the previous renderer.
 */
static void drawFilledFace(const ScreenPoint *points, size_t count,
    TGAImage *framebuffer, TGAImage *zbuffer)
{
    for(size_t i = 1; i + 1 < count; ++i)
    {
        const TGAColor color = {{(uint8_t)(rand() % 256), (uint8_t)(rand() % 256),
            (uint8_t)(rand() % 256), 255}};
        drawTriangleWithBoundingBoxDepth(points[0].x, points[0].y,
            points[i].x, points[i].y, points[i + 1].x, points[i + 1].y,
            points[0].z, points[i].z, points[i + 1].z,
            color, color, color, framebuffer, zbuffer);
    }
}

/**
 * @brief Resolve face indices and project vertices into a reusable point buffer.
 * @param[in] text Face tokens following the f record tag.
 * @param[in] vertices Vertices already read from the OBJ stream.
 * @param[in] vertexCount Number of available vertices.
 * @param[in,out] points Owned projected-point buffer, initially NULL.
 * @param[in,out] capacity Point buffer capacity.
 * @param[out] count Number of projected vertices; zero for an invalid face.
 * @return false only for allocation failure; invalid faces are skipped.
 */
static bool parseFace(const char *text, const Vertex *vertices, size_t vertexCount,
    ScreenPoint **points, size_t *capacity, size_t *count)
{
    *count = 0;
    while(*text)
    {
        while(isspace((unsigned char)*text))
            ++text;
        if(!*text || *text == '#')
            break;
        char *end;
        errno = 0;
        const long index = strtol(text, &end, 10);
        if(end == text || errno == ERANGE || index == 0
            || (*end && *end != '/' && !isspace((unsigned char)*end)))
            goto invalid;
        size_t resolved;
        if(index > 0)
        {
            if((unsigned long)index > vertexCount)
                goto invalid;
            resolved = (size_t)index - 1;
        }
        else
        {
            const unsigned long distance = (unsigned long)(-(index + 1)) + 1;
            if(distance > vertexCount)
                goto invalid;
            resolved = vertexCount - (size_t)distance;
        }
        const Vertex vertex = vertices[resolved];
        const float xScale = (framebufferWidth - 1) * 0.5F;
        const float yScale = (framebufferHeight - 1) * 0.5F;
        const double x = (vertex.x + 1.0F) * xScale;
        const double y = (vertex.y + 1.0F) * yScale;
        if(!isfinite(x) || !isfinite(y) || !isfinite(vertex.z)
            || x < INT_MIN || x >= INT_MAX || y < INT_MIN || y >= INT_MAX)
            goto invalid;
        ScreenPoint *grown = growArray(*points, capacity, *count + 1, sizeof(**points));
        if(!grown)
            return false;
        *points = grown;
        (*points)[(*count)++] = (ScreenPoint){(int)x, (int)y, ((double)vertex.z + 1.0) * 127.5};
        text = end;
        while(*text && !isspace((unsigned char)*text))
            ++text;
    }
    return true;
invalid:
    *count = 0;
    return true;
}

/**
 * @brief Share model loading, projection, drawing, and output between OBJ modes.
 * @param[in] objPath Explicit path or filename searched under obj/.
 * @param[in] outputPath Color TGA destination.
 * @param[in] drawFace Face drawing callback.
 * @param[in] zbufferPath Depth TGA destination; NULL or empty disables depth output.
 * @return 0 on success or 1 for input, allocation, or output failure.
 */
static int renderObj(const char *objPath, const char *outputPath,
    FaceDrawer drawFace, const char *zbufferPath)
{
    if(!objPath || !*objPath || !outputPath)
        return 1;
    char *modelPath = resolveModelPath(objPath);
    if(!modelPath)
        return 1;
    FILE *file = fopen(modelPath, "rb");
    if(!file)
    {
        fprintf(stderr, "Failed to open OBJ file: %s\n", modelPath);
        free(modelPath);
        return 1;
    }
    TGAImage framebuffer = {0};
    TGAImage zbuffer = {0};
    Vertex *vertices = NULL;
    ScreenPoint *points = NULL;
    char *line = NULL;
    size_t vertexCount = 0, vertexCapacity = 0, pointCapacity = 0, lineCapacity = 0, faceCount = 0;
    const bool depthEnabled = zbufferPath && *zbufferPath;
    int result = 1;
    if(!tgaImageInit(&framebuffer, framebufferWidth, framebufferHeight, TGA_RGB)
        || (depthEnabled && !tgaImageInit(&zbuffer, framebufferWidth, framebufferHeight, TGA_GRAYSCALE)))
        goto cleanup;
    int readResult;
    while((readResult = readObjLine(file, &line, &lineCapacity)) > 0)
    {
        if(line[0] == 'v' && isspace((unsigned char)line[1]))
        {
            Vertex vertex;
            if(sscanf(line + 1, "%f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3)
            {
                Vertex *grown = growArray(vertices, &vertexCapacity, vertexCount + 1, sizeof(*vertices));
                if(!grown)
                    goto cleanup;
                vertices = grown;
                vertices[vertexCount++] = vertex;
            }
        }
        else if(line[0] == 'f' && isspace((unsigned char)line[1]))
        {
            size_t count;
            if(!parseFace(line + 1, vertices, vertexCount, &points, &pointCapacity, &count))
                goto cleanup;
            if(count >= 3)
            {
                ++faceCount;
                drawFace(points, count, &framebuffer, depthEnabled ? &zbuffer : NULL);
            }
        }
    }
    if(readResult < 0)
        goto cleanup;
    printf("Loaded %zu vertices and %zu faces\n", vertexCount, faceCount);
    if(!tgaImageWrite(&framebuffer, outputPath, true, true))
    {
        fprintf(stderr, "Failed to write %s\n", outputPath);
        goto cleanup;
    }
    if(depthEnabled && !tgaImageWrite(&zbuffer, zbufferPath, true, true))
    {
        fprintf(stderr, "Failed to write %s\n", zbufferPath);
        goto cleanup;
    }
    result = 0;
cleanup:
    if(result)
        fprintf(stderr, "OBJ rendering failed: %s\n", modelPath);
    free(modelPath);
    free(vertices);
    free(points);
    free(line);
    fclose(file);
    tgaImageDestroy(&framebuffer);
    tgaImageDestroy(&zbuffer);
    return result;
}

/**
 * @brief Render OBJ face boundaries as white line segments into an 800 x 800 TGA.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA path; NULL uses obj_wireframe.tga.
 * @return 0 on success; 1 for invalid arguments, allocation failures, or file errors.
 * @note Model x/y coordinates map from [-1, 1] without automatic fitting.
 */
int renderObjWireframe(const char *objPath, const char *outputPath)
{
    return renderObj(objPath, outputPath ? outputPath : "obj_wireframe.tga", drawWireframeFace, NULL);
}

/**
 * @brief Render OBJ faces as randomly colored filled triangles and optional depth output.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA path; NULL uses obj_filled.tga.
 * @param[in] zbufferPath Depth TGA path; NULL uses obj_zbuffer.tga; "" disables depth.
 * @return 0 on success; 1 for invalid arguments, allocation failures, or file errors.
 * @pre Polygonal faces must be convex for fan triangulation.
 * @note Back-facing triangles and triangles with signed area below 1 are skipped.
 * @note Model x/y coordinates map from [-1, 1]; z maps to grayscale [0, 255].
 * @note Depth retains the largest value, but colors are written before the depth check.
 * @note No lighting or textures are applied.
 */
int renderObjFilled(const char *objPath, const char *outputPath, const char *zbufferPath)
{
    return renderObj(objPath, outputPath ? outputPath : "obj_filled.tga", drawFilledFace,
        zbufferPath ? zbufferPath : "obj_zbuffer.tga");
}
