#include "camera.h"
#include "colors.h"
#include "line_renderer.h"
#include "obj_renderer.h"
#include "triangle_renderer.h"
#include "vec.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { \
    if(!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while(0)

/** @brief Compare the used channels of two pixels without struct padding. */
static bool sameColor(TGAColor a, TGAColor b, int channels)
{
    return memcmp(a.bgra, b.bgra, (size_t)channels) == 0;
}

/** @brief Check ownership, pixel bounds, TGA formats, RLE packets, and row origins. */
static void testImages(void)
{
    const TGAFormat formats[] = {TGA_GRAYSCALE, TGA_RGB, TGA_RGBA};
    for(size_t format = 0; format < sizeof(formats) / sizeof(formats[0]); ++format)
    {
        TGAImage original = {0}, loaded = {0};
        CHECK(tgaImageInit(&original, 131, 5, formats[format]));
        for(int y = 0; y < original.height; ++y)
            for(int x = 0; x < original.width; ++x)
            {
                const TGAColor color = {{(uint8_t)(x + y * 13), (uint8_t)(x % 7),
                    (uint8_t)(y * 30), (uint8_t)(100 + y)}};
                tgaImageSet(&original, x, y, x < 129 && y == 0 ? red : color);
            }
        const TGAColor before = tgaImageGet(&original, 0, 0);
        tgaImageSet(&original, -1, 0, blue);
        CHECK(sameColor(tgaImageGet(&original, 0, 0), before, formats[format]));
        CHECK(tgaImageGet(&original, 1000, 0).bgra[0] == 0);
        for(int rle = 0; rle <= 1; ++rle)
            for(int bottomOrigin = 0; bottomOrigin <= 1; ++bottomOrigin)
            {
                CHECK(tgaImageWrite(&original, "roundtrip.tga", bottomOrigin != 0, rle != 0));
                CHECK(tgaImageRead(&loaded, "roundtrip.tga"));
                CHECK(loaded.width == 131 && loaded.height == 5);
                CHECK(loaded.bytesPerPixel == (int)formats[format]);
                for(int y = 0; y < original.height; ++y)
                    for(int x = 0; x < original.width; ++x)
                        CHECK(sameColor(tgaImageGet(&original, x, y),
                            tgaImageGet(&loaded, x, bottomOrigin ? 4 - y : y), formats[format]));
            }
        tgaImageFlipHorizontally(&original);
        tgaImageFlipHorizontally(&original);
        CHECK(sameColor(tgaImageGet(&original, 0, 0), before, formats[format]));
        CHECK(!tgaImageInit(&loaded, 0, 5, formats[format]));
        CHECK(loaded.data != NULL);
        tgaImageDestroy(&original);
        tgaImageDestroy(&loaded);
        tgaImageDestroy(&loaded);
        CHECK(loaded.data == NULL && loaded.width == 0);
    }
    /* Reject a packet that exceeds the declared image size before writing pixels. */
    FILE *file = fopen("invalid.tga", "wb");
    CHECK(file != NULL);
    uint8_t header[18] = {0};
    header[2] = 11; header[12] = 1; header[14] = 1; header[16] = 8;
    CHECK(fwrite(header, 1, sizeof(header), file) == sizeof(header));
    CHECK(fputc(255, file) != EOF);
    CHECK(fputc(123, file) != EOF);
    CHECK(fclose(file) == 0);
    TGAImage retained = {0};
    CHECK(tgaImageInit(&retained, 2, 2, TGA_RGB));
    tgaImageSet(&retained, 0, 0, green);
    CHECK(!tgaImageRead(&retained, "invalid.tga"));
    CHECK(sameColor(tgaImageGet(&retained, 0, 0), green, 3));
    tgaImageDestroy(&retained);
}

/** @brief Check line direction, inclusive endpoints, and coincident points. */
static void testLines(void)
{
    TGAImage forward = {0}, reverse = {0};
    CHECK(tgaImageInit(&forward, 16, 16, TGA_RGB));
    CHECK(tgaImageInit(&reverse, 16, 16, TGA_RGB));
    const int segments[][4] = {{1,1,12,4}, {2,1,4,12}, {12,1,3,13}, {1,7,12,7}};
    for(size_t i = 0; i < sizeof(segments) / sizeof(segments[0]); ++i)
    {
        const int *p = segments[i];
        drawLineInterpolated(p[0], p[1], p[2], p[3], &forward, white);
        drawLineInterpolated(p[2], p[3], p[0], p[1], &reverse, white);
        CHECK(sameColor(tgaImageGet(&forward, p[0], p[1]), white, 3));
        CHECK(sameColor(tgaImageGet(&forward, p[2], p[3]), white, 3));
    }
    CHECK(memcmp(forward.data, reverse.data, 16 * 16 * 3) == 0);
    drawLineInterpolated(8, 8, 8, 8, &forward, red);
    CHECK(sameColor(tgaImageGet(&forward, 8, 8), red, 3));
    drawLineBarycentric(0, 0, 0, 0, &forward, green);
    CHECK(sameColor(tgaImageGet(&forward, 0, 0), green, 3));
    tgaImageDestroy(&forward);
    tgaImageDestroy(&reverse);
}

/** @brief Check RGB interpolation, scanline paths, culling, and depth compatibility. */
static void testTriangles(void)
{
    TGAImage color = {0};
    double depth[12 * 12];
    for(size_t i = 0; i < 12 * 12; ++i) depth[i] = -INFINITY;
    CHECK(tgaImageInit(&color, 12, 12, TGA_RGBA));
    const TGAColor pureBlue = {{255, 0, 0, 255}};
    drawTriangleWithBoundingBoxDepth(1,1,9,1,1,9,1,128,255,
        red,green,pureBlue,&color,depth);
    CHECK(sameColor(tgaImageGet(&color, 1, 1), red, 4));
    CHECK(sameColor(tgaImageGet(&color, 9, 1), green, 4));
    CHECK(sameColor(tgaImageGet(&color, 1, 9), pureBlue, 4));
    const TGAColor middle = {{63, 63, 127, 255}};
    CHECK(sameColor(tgaImageGet(&color, 3, 3), middle, 4));
    CHECK(depth[3 * 12 + 3] == 96.25);
    CHECK(depth[9 * 12 + 1] == 255);
    CHECK(depth[10 * 12 + 10] == -INFINITY);
    drawTriangleWithBoundingBoxDepth(1,1,9,1,1,9,20,20,20,green,green,green,&color,depth);
    CHECK(depth[3 * 12 + 3] == 96.25);
    /* A farther face must not overwrite the interpolated color already present. */
    CHECK(sameColor(tgaImageGet(&color, 3, 3), middle, 4));
    drawTriangleWithBoundingBoxDepth(1,1,9,1,1,9,200,200,200,green,green,green,&color,depth);
    CHECK(depth[3 * 12 + 3] == 200);
    CHECK(sameColor(tgaImageGet(&color, 3, 3), green, 4));
    drawTriangleWithBoundingBoxDepth(1,1,9,1,1,9,20,20,20,red,red,red,&color,depth);
    CHECK(depth[3 * 12 + 3] == 200);
    CHECK(sameColor(tgaImageGet(&color, 3, 3), green, 4));
    drawTriangleWithBoundingBox(1,1,1,9,9,1,red,red,red,&color);
    CHECK(sameColor(tgaImageGet(&color, 3, 3), green, 4));
    /* Depths beyond 255 and fractional differences must retain their ordering. */
    drawTriangleWithBoundingBoxDepth(-1,-1,9,-1,-1,9,280,280,280,red,red,red,&color,depth);
    drawTriangleWithBoundingBoxDepth(-1,-1,9,-1,-1,9,280.5,280.5,280.5,green,green,green,&color,depth);
    drawTriangleWithBoundingBoxDepth(-1,-1,9,-1,-1,9,280.25,280.25,280.25,red,red,red,&color,depth);
    CHECK(depth[3 * 12 + 3] == 280.5);
    CHECK(sameColor(tgaImageGet(&color, 3, 3), green, 4));
    drawTriangleWithMyScanlineV1(1,1,9,3,1,9,&color,white);
    CHECK(sameColor(tgaImageGet(&color, 3, 3), white, 4));
    drawTriangleWithMyScanlineV2(1,1,9,1,1,9,&color,red);
    CHECK(sameColor(tgaImageGet(&color, 3, 3), red, 4));
    tgaImageDestroy(&color);

}

/** @brief Exercise shared OBJ parsing, negative/slash indices, paths, and output errors. */
static void testObj(void)
{
    FILE *file = fopen("mesh.obj", "wb");
    CHECK(file != NULL);
    const char *model = "v -0.5 -0.5 -1\nv 0.5 -0.5 0\nv 0.5 0.5 1\nv -0.5 0.5 0\n"
        "f -4/1/1 -3/2/1 -2/3/1 -1/4/1\nf 0 2 3\nf 999999999999999999999999 2 3\n";
    CHECK(fputs(model, file) >= 0 && fclose(file) == 0);
    CHECK(renderObjWireframe("./mesh.obj", "wire.tga", OBJ_PROJECTION_PERSPECTIVE) == 0);
    CHECK(renderObjFilled("./mesh.obj", "filled.tga", "depth.tga", OBJ_PROJECTION_PERSPECTIVE) == 0);
    TGAImage wire = {0}, filled = {0}, depth = {0};
    CHECK(tgaImageRead(&wire, "wire.tga"));
    CHECK(tgaImageRead(&filled, "filled.tga"));
    CHECK(tgaImageRead(&depth, "depth.tga"));
    CHECK(wire.width == 800 && depth.bytesPerPixel == 1 && filled.bytesPerPixel == 3);
    CHECK(wire.data != NULL && tgaImageGet(&wire, 400, 400).bgra[0] == 0);
    /* The camera maps the first vertex to (159, 265) in the inset viewport.
       Reading the bottom-origin TGA reverses its row to 800 - 1 - 265. */
    CHECK(sameColor(tgaImageGet(&wire, 159, 534), white, 3));
    CHECK(renderObjWireframe("./mesh.obj", "wire-orthographic.tga", OBJ_PROJECTION_ORTHOGRAPHIC) == 0);
    TGAImage orthographic = {0};
    CHECK(tgaImageRead(&orthographic, "wire-orthographic.tga"));
    /* Without perspective, the same camera maps the vertex to (86, 225). */
    CHECK(sameColor(tgaImageGet(&orthographic, 86, 574), white, 3));
    CHECK(memcmp(wire.data, orthographic.data, 800 * 800 * 3) != 0);
    tgaImageDestroy(&orthographic);
    CHECK(renderObjFilled("./mesh.obj", NULL, NULL, OBJ_PROJECTION_ORTHOGRAPHIC) == 0);
    TGAImage orthographicDepth = {0};
    CHECK(tgaImageRead(&orthographicDepth, "obj_zbuffer_orthographic.tga"));
    CHECK(orthographicDepth.bytesPerPixel == 1);
    CHECK(tgaImageGet(&orthographicDepth, 400, 400).bgra[0] > 0);
    tgaImageDestroy(&orthographicDepth);
    CHECK(tgaImageGet(&depth, 400, 400).bgra[0] > 0);
    CHECK(renderObjFilled("./mesh.obj", "without-depth.tga", "", OBJ_PROJECTION_PERSPECTIVE) == 0);
    CHECK(renderObjWireframe("./does-not-exist.obj", "unused.tga", OBJ_PROJECTION_PERSPECTIVE) == 1);
    CHECK(renderObjFilled("./mesh.obj", "missing-directory/out.tga", "", OBJ_PROJECTION_PERSPECTIVE) == 1);
    tgaImageDestroy(&wire);
    tgaImageDestroy(&filled);
    tgaImageDestroy(&depth);
}

/** @brief Check Y rotation orientation, invariant length/axis, and inverse rotation. */
static void testRotation(void)
{
    const double epsilon = 1e-12;
    const Vec3 v = {1.0, 2.0, 3.0};
    const Vec3 unchanged = vec3RotateY(v, 0.0);
    CHECK(fabs(unchanged.x - v.x) < epsilon);
    CHECK(fabs(unchanged.y - v.y) < epsilon);
    CHECK(fabs(unchanged.z - v.z) < epsilon);

    const Vec3 rotated = vec3RotateY(v, 30.0);
    CHECK(fabs(rotated.y - v.y) < epsilon);
    CHECK(fabs(vec3Length(rotated) - vec3Length(v)) < epsilon);
    const Vec3 restored = vec3RotateY(rotated, -30.0);
    CHECK(fabs(restored.x - v.x) < epsilon);
    CHECK(fabs(restored.y - v.y) < epsilon);
    CHECK(fabs(restored.z - v.z) < epsilon);

    const Vec3 quarterTurn = vec3RotateY((Vec3){1.0, 0.0, 0.0}, 90.0);
    CHECK(fabs(quarterTurn.x) < epsilon);
    CHECK(fabs(quarterTurn.y) < epsilon);
    CHECK(fabs(quarterTurn.z + 1.0) < epsilon);
}

/** @brief Check the projection plane and the apparent size of nearer/farther vertices. */
static void testPerspective(void)
{
    const double epsilon = 1e-12;
    const Vec3 onPlane = vec3Perspective((Vec3){1.0, -2.0, 0.0}, 3.0);
    CHECK(fabs(onPlane.x - 1.0) < epsilon);
    CHECK(fabs(onPlane.y + 2.0) < epsilon);
    CHECK(fabs(onPlane.z) < epsilon);

    const Vec3 nearVertex = vec3Perspective((Vec3){1.0, 2.0, 1.0}, 3.0);
    CHECK(fabs(nearVertex.x - 1.5) < epsilon);
    CHECK(fabs(nearVertex.y - 3.0) < epsilon);
    CHECK(fabs(nearVertex.z - 1.5) < epsilon);
    const Vec3 farVertex = vec3Perspective((Vec3){1.0, 2.0, -1.0}, 3.0);
    CHECK(fabs(farVertex.x - 0.75) < epsilon);
    CHECK(fabs(farVertex.y - 1.5) < epsilon);
    CHECK(fabs(farVertex.z + 0.75) < epsilon);
}

/** @brief Check viewport center, independent dimensions, depth, and tutorial boundaries. */
static void testViewportProjection(void)
{
    const ProjectedVertex center = vec3Project((Vec3){0, 0, 0}, 800, 600);
    CHECK(center.x == 400 && center.y == 300 && center.z == 127);
    const ProjectedVertex lower = vec3Project((Vec3){-1, -1, -1}, 800, 600);
    CHECK(lower.x == 0 && lower.y == 0 && lower.z == 0);
    const ProjectedVertex upper = vec3Project((Vec3){1, 1, 1}, 800, 600);
    CHECK(upper.x == 800 && upper.y == 600 && upper.z == 255);
    const ProjectedVertex outside = vec3Project((Vec3){-1.5, 0.5, 0}, 800, 600);
    CHECK(outside.x == -200 && outside.y == 450 && outside.z == 127);
}

/** @brief Check homogeneous matrix composition, camera basis, and viewport depth preservation. */
static void testCamera(void)
{
    const double epsilon = 1e-12;
    const Vec3 eye = {-1, 0, 2};
    const Vec3 center = {0, 0, 0};
    const Vec3 up = {0, 1, 0};
    Mat4 modelView;
    CHECK(cameraLookAt(&modelView, eye, center, up));
    const Vec4 eyeInFrame = mat4MultiplyVec4(modelView, (Vec4){-1, 0, 2, 1});
    CHECK(fabs(eyeInFrame.x) < epsilon && fabs(eyeInFrame.y) < epsilon);
    CHECK(fabs(eyeInFrame.z - sqrt(5.0)) < epsilon && eyeInFrame.w == 1.0);
    /* The camera axes form a right-handed orthonormal frame. */
    const Vec3 right = {modelView.data[0][0], modelView.data[0][1], modelView.data[0][2]};
    const Vec3 vertical = {modelView.data[1][0], modelView.data[1][1], modelView.data[1][2]};
    const Vec3 backward = {modelView.data[2][0], modelView.data[2][1], modelView.data[2][2]};
    CHECK(fabs(vec3Length(right) - 1.0) < epsilon);
    CHECK(fabs(vec3Length(vertical) - 1.0) < epsilon);
    CHECK(fabs(vec3Dot(right, backward)) < epsilon);
    CHECK(vec3Length(vec3Subtract(vec3Cross(right, vertical), backward)) < epsilon);

    Mat4 translatedView;
    CHECK(cameraLookAt(&translatedView, (Vec3){1, 2, 5}, (Vec3){1, 2, 3}, up));
    const Vec4 target = mat4MultiplyVec4(translatedView, (Vec4){1, 2, 3, 1});
    CHECK(target.x == 0 && target.y == 0 && target.z == 0 && target.w == 1);
    CHECK(!cameraLookAt(&translatedView, center, center, up));
    CHECK(!cameraLookAt(&translatedView, eye, center, eye));
    CHECK(!cameraLookAt(&translatedView, eye, center, (Vec3){0, 0, 0}));
    CHECK(!cameraLookAt(&translatedView, (Vec3){NAN, 0, 2}, center, up));

    const Mat4 perspective = cameraPerspective(3.0);
    const Mat4 viewport = cameraViewport(10, 20, 200, 100);
    const Mat4 composed = mat4Multiply(perspective, modelView);
    const Vec4 source = {0.2, 0.4, 0.7, 1.0};
    const Vec4 clip = mat4MultiplyVec4(composed, source);
    const Vec4 separate = mat4MultiplyVec4(perspective, mat4MultiplyVec4(modelView, source));
    CHECK(fabs(clip.x - separate.x) < epsilon && fabs(clip.y - separate.y) < epsilon);
    CHECK(fabs(clip.z - separate.z) < epsilon && fabs(clip.w - separate.w) < epsilon);
    Vec3 screen;
    CHECK(cameraClipToScreen(clip, viewport, &screen));
    const Vec4 view = mat4MultiplyVec4(modelView, source);
    const Vec3 reference = vec3Perspective((Vec3){view.x, view.y, view.z}, 3.0);
    CHECK(fabs(screen.x - (110.0 + 100.0 * reference.x)) < epsilon);
    CHECK(fabs(screen.y - (70.0 + 50.0 * reference.y)) < epsilon);
    CHECK(fabs(screen.z - reference.z) < epsilon);
    CHECK(cameraClipToScreen((Vec4){-1, -1, -2.5, 1}, viewport, &screen));
    CHECK(screen.x == 10 && screen.y == 20 && screen.z == -2.5);
    CHECK(cameraClipToScreen((Vec4){1, 1, 2.5, 1}, viewport, &screen));
    CHECK(screen.x == 210 && screen.y == 120 && screen.z == 2.5);
    CHECK(!cameraClipToScreen((Vec4){0, 0, 0, 0}, viewport, &screen));
    CHECK(!cameraClipToScreen((Vec4){0, 0, 0, -1}, viewport, &screen));
    CHECK(!cameraClipToScreen((Vec4){INFINITY, 0, 0, 1}, viewport, &screen));
}

/** @brief Verify subpixel coverage, homogeneous division, and negative depth testing. */
static void testClipRasterization(void)
{
    TGAImage image = {0};
    CHECK(tgaImageInit(&image, 12, 12, TGA_RGB));
    double depth[12 * 12];
    for(size_t i = 0; i < 12 * 12; ++i) depth[i] = -INFINITY;
    const Mat4 viewport = mat4Identity();
    Vec4 clip[3] = {{1.25, 1.25, -2, 1}, {9.25, 1.25, -2, 1}, {1.25, 9.25, -2, 1}};
    rasterizeTriangle(clip, viewport, red, red, red, &image, depth);
    CHECK(sameColor(tgaImageGet(&image, 1, 1), (TGAColor){{0}}, 3));
    CHECK(sameColor(tgaImageGet(&image, 2, 2), red, 3));
    CHECK(depth[2 * 12 + 2] == -2.0);
    /* Scaling all four components must leave the represented point unchanged. */
    for(int i = 0; i < 3; ++i) clip[i] = vec4Scale(clip[i], 2.0);
    for(int i = 0; i < 3; ++i) clip[i].z = -2.0;
    rasterizeTriangle(clip, viewport, green, green, green, &image, depth);
    CHECK(depth[2 * 12 + 2] == -1.0);
    CHECK(sameColor(tgaImageGet(&image, 2, 2), green, 3));
    clip[0].w = 0.0;
    rasterizeTriangle(clip, viewport, red, red, red, &image, depth);
    CHECK(sameColor(tgaImageGet(&image, 2, 2), green, 3));
    tgaImageDestroy(&image);
}

/** @brief Run C API regression tests and report success. */
int main(void)
{
    testImages();
    testLines();
    testTriangles();
    testObj();
    testRotation();
    testPerspective();
    testViewportProjection();
    testCamera();
    testClipRasterization();
    puts("Renderer C17 tests passed.");
    return 0;
}
