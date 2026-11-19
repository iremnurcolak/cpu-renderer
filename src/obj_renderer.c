#include "obj_renderer.h"
#include "colors.h"
#include "line_renderer.h"
#include "triangle_renderer.h"
#include "vec.h"

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
static const double cameraDistance = 3.0;

typedef struct Vertex { float x, y, z; } Vertex;
typedef struct ScreenPoint { int x, y; double z; } ScreenPoint;
typedef void (*FaceDrawer)(const ScreenPoint *, size_t, TGAImage *, double *);

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
    TGAImage *framebuffer, double *zbuffer)
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
 * @param[in,out] zbuffer Optional row-major double depth buffer, or NULL.
 * @note Pixels failing the depth comparison are skipped before color interpolation.
 */
static void drawFilledFace(const ScreenPoint *points, size_t count,
    TGAImage *framebuffer, double *zbuffer)
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
 * @brief Resolve face indices, rotate vertices, and apply the selected projection.
 * @param[in] text Face tokens following the f record tag.
 * @param[in] vertices Vertices already read from the OBJ stream.
 * @param[in] vertexCount Number of available vertices.
 * @param[in] rotationDegrees Y-axis rotation applied to 3D vertices before projection.
 * @param[in] projection Orthographic or perspective-plus-viewport projection.
 * @param[in,out] points Owned projected-point buffer, initially NULL.
 * @param[in,out] capacity Point buffer capacity.
 * @param[out] count Number of projected vertices; zero for an invalid face.
 * @return false only for allocation failure; invalid faces are skipped.
 * @note Perspective faces on or behind the camera are skipped; near-plane clipping is not implemented.
 */
static bool parseFace(const char *text, const Vertex *vertices, size_t vertexCount,
    double rotationDegrees, ObjProjection projection,
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
        const Vertex source = vertices[resolved];
        Vec3 vertex = vec3RotateY((Vec3){source.x, source.y, source.z}, rotationDegrees);
        if(!isfinite(vertex.x) || !isfinite(vertex.y) || !isfinite(vertex.z))
            goto invalid;
        if(projection == OBJ_PROJECTION_PERSPECTIVE)
        {
            if(vertex.z >= cameraDistance)
                goto invalid;
            vertex = vec3Perspective(vertex, cameraDistance);
        }

        /* Validate the viewport values before vec3Project converts them to int. */
        const double x = (vertex.x + 1.0) * framebufferWidth / 2.0;
        const double y = (vertex.y + 1.0) * framebufferHeight / 2.0;
        const double z = (vertex.z + 1.0) * 255.0 / 2.0;
        if(!isfinite(x) || !isfinite(y) || !isfinite(z)
            || x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX
            || z < INT_MIN || z > INT_MAX)
            goto invalid;
        const ProjectedVertex projected = vec3Project(vertex, framebufferWidth, framebufferHeight);
        ScreenPoint *grown = growArray(*points, capacity, *count + 1, sizeof(**points));
        if(!grown)
            return false;
        *points = grown;
        (*points)[(*count)++] = (ScreenPoint){projected.x, projected.y, z};
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
 * @param[in] zbufferPath Depth TGA destination; NULL or empty disables depth image output.
 * @param[in] projection Selected projection for all face vertices.
 * @return 0 on success or 1 for input, allocation, or output failure.
 */
static int renderObj(const char *objPath, const char *outputPath,
    FaceDrawer drawFace, const char *zbufferPath, ObjProjection projection)
{
    if(!objPath || !*objPath || !outputPath
        || (projection != OBJ_PROJECTION_ORTHOGRAPHIC && projection != OBJ_PROJECTION_PERSPECTIVE))
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
    TGAImage depthImage = {0};
    double *zbuffer = NULL;
    Vertex *vertices = NULL;
    ScreenPoint *points = NULL;
    char *line = NULL;
    size_t vertexCount = 0, vertexCapacity = 0, pointCapacity = 0, lineCapacity = 0, faceCount = 0;
    const bool depthEnabled = zbufferPath && *zbufferPath;
    const double rotationDegrees = 30.0;
    int result = 1;
    if(!tgaImageInit(&framebuffer, framebufferWidth, framebufferHeight, TGA_RGB)
        || (depthEnabled && !tgaImageInit(&depthImage, framebufferWidth, framebufferHeight, TGA_GRAYSCALE)))
        goto cleanup;
    if(drawFace == drawFilledFace)
    {
        const size_t pixelCount = (size_t)framebufferWidth * framebufferHeight;
        zbuffer = malloc(pixelCount * sizeof(*zbuffer));
        if(!zbuffer) goto cleanup;
        for(size_t i = 0; i < pixelCount; ++i) zbuffer[i] = -INFINITY;
    }
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
            if(!parseFace(line + 1, vertices, vertexCount, rotationDegrees, projection,
                &points, &pointCapacity, &count))
                goto cleanup;
            if(count >= 3)
            {
                ++faceCount;
                drawFace(points, count, &framebuffer, zbuffer);
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
    printf("Saved image: %s\n", outputPath);
    if(depthEnabled)
    {
        for(int y = 0; y < framebufferHeight; ++y)
            for(int x = 0; x < framebufferWidth; ++x)
            {
                const double depth = zbuffer[(size_t)y * framebufferWidth + x];
                const uint8_t shade = (uint8_t)fmin(255.0, fmax(0.0, depth));
                tgaImageSet(&depthImage, x, y, (TGAColor){{shade, 0, 0, 0}});
            }
        if(!tgaImageWrite(&depthImage, zbufferPath, true, true))
        {
            fprintf(stderr, "Failed to write %s\n", zbufferPath);
            goto cleanup;
        }
        printf("Saved image: %s\n", zbufferPath);
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
    free(zbuffer);
    tgaImageDestroy(&depthImage);
    return result;
}

/**
 * @brief Render OBJ face boundaries as white line segments into an 800 x 800 TGA.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA path; NULL uses obj_wireframe_<projection>.tga.
 * @param[in] projection Orthographic or perspective-plus-viewport projection.
 * @return 0 on success; 1 for invalid arguments, allocation failures, or file errors.
 * @note Transformed x/y map from [-1, 1] without automatic fitting.
 * @note Vertices are rotated 30 degrees about Y; perspective mode uses camera distance 3.
 */
int renderObjWireframe(const char *objPath, const char *outputPath, ObjProjection projection)
{
    const char *defaultPath = projection == OBJ_PROJECTION_ORTHOGRAPHIC
        ? "obj_wireframe_orthographic.tga" : "obj_wireframe_perspective.tga";
    return renderObj(objPath, outputPath ? outputPath : defaultPath, drawWireframeFace, NULL, projection);
}

/**
 * @brief Render OBJ faces as randomly colored filled triangles and optional depth output.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA path; NULL uses obj_filled_<projection>.tga.
 * @param[in] zbufferPath Depth TGA path; NULL uses obj_zbuffer_<projection>.tga; "" disables depth.
 * @param[in] projection Orthographic or perspective-plus-viewport projection.
 * @return 0 on success; 1 for invalid arguments, allocation failures, or file errors.
 * @pre Polygonal faces must be convex for fan triangulation.
 * @note Back-facing triangles and triangles with signed area below 1 are skipped.
 * @note Vertices are rotated 30 degrees about Y; perspective mode uses camera distance 3.
 * @note Transformed x/y map to the viewport and unclamped z is retained in a double depth buffer.
 * @note Larger depth wins even without depth image output; visualization alone is clamped.
 * @note No lighting or textures are applied.
 */
int renderObjFilled(const char *objPath, const char *outputPath, const char *zbufferPath,
    ObjProjection projection)
{
    const char *defaultColor = projection == OBJ_PROJECTION_ORTHOGRAPHIC
        ? "obj_filled_orthographic.tga" : "obj_filled_perspective.tga";
    const char *defaultDepth = projection == OBJ_PROJECTION_ORTHOGRAPHIC
        ? "obj_zbuffer_orthographic.tga" : "obj_zbuffer_perspective.tga";
    return renderObj(objPath, outputPath ? outputPath : defaultColor, drawFilledFace,
        zbufferPath ? zbufferPath : defaultDepth, projection);
}
