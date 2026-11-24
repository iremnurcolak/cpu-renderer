#include "triangle_renderer.h"
#include "line_renderer.h"
#include "colors.h"

#include <math.h>
#include <stddef.h>
#include "camera.h"

/**
 * @brief Swap two integer coordinates in place.
 * @param[in,out] a First coordinate.
 * @param[in,out] b Second coordinate.
 */
static void swapInt(int *a, int *b)
{
    const int temporary = *a;
    *a = *b;
    *b = temporary;
}

/** @brief Return the smaller of two integer bounds. */
static int minInt(int a, int b) { return a < b ? a : b; }

/** @brief Return the larger of two integer bounds. */
static int maxInt(int a, int b) { return a > b ? a : b; }

/**
 * @brief Find the axis-aligned bounding box of three vertices.
 * @note Corner names assume the y axis increases upward.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 * @param[out] bottomLeftX Minimum vertex x coordinate.
 * @param[out] bottomLeftY Minimum vertex y coordinate.
 * @param[out] topRightX Maximum vertex x coordinate.
 * @param[out] topRightY Maximum vertex y coordinate.
 */
static void findTriangleBoundingBox(double ax, double ay, double bx, double by, double cx, double cy,
    double *bottomLeftX, double *bottomLeftY, double *topRightX, double *topRightY);
/**
 * @brief Calculate the signed area of a triangle.
 * @return Positive for counterclockwise winding with y increasing upward,
 *         negative for clockwise winding, or zero for collinear vertices.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 */
static double calculateTriangleAreaSigned(double ax, double ay, double bx, double by, double cx, double cy);

/**
 * @brief Fill a triangle using floating-point scanline intersections.
 * @pre The three vertex y coordinates must be distinct.
 * @note Vertices may be supplied in any order; horizontal spans include both endpoints.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 * @param[in,out] framebuffer Image receiving the rendered pixels.
 * @param[in] color Pixel color in BGRA order.
 */
void drawTriangleWithMyScanlineV1(int ax, int ay, int bx, int by, int cx, int cy,
                                  TGAImage *framebuffer,
                                  TGAColor color)
{
    // sort the vertices, a,b,c in ascending y order
    if (ay > by)
    {
        swapInt(&ax, &bx);
        swapInt(&ay, &by);
    }
    if (ay > cy)
    {
        swapInt(&ax, &cx);
        swapInt(&ay, &cy);
    }
    if (by > cy)
    {
        swapInt(&bx, &cx);
        swapInt(&by, &cy);
    }

    if(ay == by || by == cy)
        return;

    for (int y = ay; y <= by; y++)
    {
        const float t1 = ((float)(y) - ay) / ((float)(by) - ay);
        float x1 = 0.0F;
        if (t1 >= 0.0F && t1 <= 1.0F)
        {
            x1 = ax + t1 * ((float)(bx) - ax);
            // Intersection at the current scanline.
        }

        const float t2 = ((float)(y) - ay) / ((float)(cy) - ay);
        float x2 = 0.0F;
        if (t2 >= 0.0F && t2 <= 1.0F)
        {
            x2 = ax + t2 * ((float)(cx) - ax);
            // Intersection at the current scanline.
        }

        drawLineInterpolated(x1, y, x2, y, framebuffer, color);
    }

    for (int y = by; y <= cy; y++)
    {
        const float t1 = ((float)(y) - by) / ((float)(cy) - by);
        float x1 = 0.0F;
        if (t1 >= 0.0F && t1 <= 1.0F)
        {
            x1 = bx + t1 * ((float)(cx) - bx);
            // cross: (x, a)
        }

        const float t2 = ((float)(y) - ay) / ((float)(cy) - ay);
        float x2 = 0.0F;
        if (t2 >= 0.0F && t2 <= 1.0F)
        {
            x2 = ax + t2 * ((float)(cx) - ax);
            // cross: (x, a)
        }

        drawLineInterpolated(x1, y, x2, y, framebuffer, color);
    }
}

/**
 * @brief Fill a triangle using integer scanline intersections.
 * @pre Coordinate differences and their products must fit in int.
 * @note Division truncates toward zero; spans exclude the right endpoint.
 * @note Vertices may be supplied in any order; equal vertex y values draw no pixels.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 * @param[in,out] framebuffer Image receiving the rendered pixels.
 * @param[in] color Pixel color in BGRA order.
 */
void drawTriangleWithMyScanlineV2(int ax, int ay, int bx, int by, int cx, int cy,
                                  TGAImage *framebuffer,
                                  TGAColor color)
{
    // sort the vertices, a,b,c in ascending y order
    if (ay > by)
    {
        swapInt(&ax, &bx);
        swapInt(&ay, &by);
    }
    if (ay > cy)
    {
        swapInt(&ax, &cx);
        swapInt(&ay, &cy);
    }
    if (by > cy)
    {
        swapInt(&bx, &cx);
        swapInt(&by, &cy);
    }

    if (ay != by)
    {
        for (int y = ay; y <= by; y++)
        {
            int x1 = ax + (((y - ay) * (bx - ax)) / (by - ay));
            // cross: (x1, a)

            int x2 = ax + (((y - ay) * (cx - ax)) / (cy - ay));
            // cross: (x2, a)

            // these two methods give different results
            // drawLineInterpolated(x1, y, x2, y, framebuffer, color);
            for (int x = minInt(x1, x2); x < maxInt(x1, x2); x++) // draw a horizontal line
                tgaImageSet(framebuffer, x, y, color);
        }
    }

    if (by != cy)
    {
        for (int y = by; y <= cy; y++)
        {
            int x1 = bx + (((y - by) * (cx - bx)) / (cy - by));
            // First scanline intersection.

            int x2 = ax + (((y - ay) * (cx - ax)) / (cy - ay));
            // Second scanline intersection.

            // these two methods give different results
            // drawLineInterpolated(x1, y, x2, y, framebuffer, color);
            for (int x = minInt(x1, x2); x < maxInt(x1, x2); x++) // draw a horizontal line
                tgaImageSet(framebuffer, x, y, color);
        }
    }
}

/**
 * @brief Fill a triangle with barycentrically interpolated RGB vertex colors.
 * @note Triangles with signed area below 0.5 pixel squared are skipped.
 * @note With y increasing upward, only counterclockwise triangles are drawn.
 * @note Triangle edges are included. B, G, and R channels are interpolated separately.
 * @note Output alpha is 255; supply an RGB or RGBA framebuffer for full color.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 * @param[in,out] framebuffer Image receiving the rendered pixels.
 * @param[in] colorA Color at vertex A in BGRA order.
 * @param[in] colorB Color at vertex B in BGRA order.
 * @param[in] colorC Color at vertex C in BGRA order.
 */
void drawTriangleWithBoundingBox(int ax, int ay, int bx, int by, int cx, int cy,
                                TGAColor  colorA, TGAColor  colorB, TGAColor  colorC,
                                TGAImage *framebuffer)
{
    drawTriangleWithBoundingBoxDepth(ax, ay, bx, by, cx, cy, 0.0, 0.0, 0.0,
        colorA, colorB, colorC, framebuffer, NULL);
}

/**
 * @brief Rasterize screen-space vertices and invoke a shader for accepted fragments.
 * @param[in] screen Three subpixel screen positions with unscaled depth.
 * @param[in] shader Fragment callback and caller-owned, read-only context.
 * @param[in,out] framebuffer Destination image.
 * @param[in,out] zbuffer Optional double depth array, initialized to -INFINITY.
 * @note Coverage and depth tests precede shading; discarded fragments do not write depth.
 */
static void rasterizeScreenTriangle(const Vec3 screen[3], const Shader *shader,
    TGAImage *framebuffer, double *zbuffer)
{
    const double ax = screen[0].x, ay = screen[0].y, az = screen[0].z;
    const double bx = screen[1].x, by = screen[1].y, bz = screen[1].z;
    const double cx = screen[2].x, cy = screen[2].y, cz = screen[2].z;
    if(!shader || !shader->fragment || !framebuffer || !framebuffer->data || !isfinite(ax) || !isfinite(ay)
        || !isfinite(bx) || !isfinite(by) || !isfinite(cx) || !isfinite(cy)
        || !isfinite(az) || !isfinite(bz) || !isfinite(cz)) return;
    double minX, minY, maxX, maxY;
    findTriangleBoundingBox(ax, ay, bx, by, cx, cy, &minX, &minY, &maxX, &maxY);
    minX = fmax(ceil(minX), 0.0);
    minY = fmax(ceil(minY), 0.0);
    maxX = fmin(floor(maxX), framebuffer->width - 1.0);
    maxY = fmin(floor(maxY), framebuffer->height - 1.0);
    if(minX > maxX || minY > maxY) return;
    const int bottomLeftX = (int)minX;
    const int bottomLeftY = (int)minY;
    const int topRightX = (int)maxX;
    const int topRightY = (int)maxY;

    double totalArea = calculateTriangleAreaSigned(ax, ay, bx, by, cx, cy);

    // Skip back-facing triangles and projected determinants below one.
    // negative area means the normal of the triangle points back
    if(!isfinite(totalArea) || totalArea < 0.5)
    {
        return;
    }
#ifdef _OPENMP
#pragma omp parallel for
#endif

    for (int x = bottomLeftX; x <= topRightX; x++)
    {
        for (int y = bottomLeftY; y <= topRightY; y++)
        {
            // calculate barycentric coordinates
            double alpha = calculateTriangleAreaSigned(x, y, bx, by, cx, cy) / totalArea;
            double beta = calculateTriangleAreaSigned(x, y, cx, cy, ax, ay) / totalArea;
            double gamma = calculateTriangleAreaSigned(x, y, ax, ay, bx, by) / totalArea;
            if(alpha < 0 || beta < 0 || gamma < 0)
            {
                continue;
            }

            const double z = alpha * az + beta * bz + gamma * cz;
            const size_t index = (size_t)y * framebuffer->width + x;
            if(zbuffer && z <= zbuffer[index]) continue;

            const FragmentResult result = shader->fragment(shader->context,
                (Vec3){alpha, beta, gamma});
            if(result.discard) continue;

            if(zbuffer) zbuffer[index] = z;
            tgaImageSet(framebuffer, x, y, result.color);
        }
    }
}

/**
 * @brief Fill a colored triangle using a double-precision depth buffer.
 * @note Triangles with signed area below 0.5 pixel squared are skipped.
 * @note With y increasing upward, only counterclockwise triangles are drawn.
 * @note Triangle edges are included. B, G, and R channels are interpolated separately.
 * @note Output alpha is 255; supply an RGB or RGBA framebuffer for full color.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 * @param[in] az Unclamped depth at vertex A.
 * @param[in] bz Unclamped depth at vertex B.
 * @param[in] cz Unclamped depth at vertex C.
 * @param[in,out] framebuffer Image receiving the rendered pixels.
 * @param[in,out] zbuffer Row-major double array matching the framebuffer, or NULL to disable depth.
 * @note Initialize every depth sample to -INFINITY; the largest value is retained.
 * @note Depth testing precedes color interpolation and pixel writes; larger z is nearer.
 * @param[in] colorA Color at vertex A in BGRA order.
 * @param[in] colorB Color at vertex B in BGRA order.
 * @param[in] colorC Color at vertex C in BGRA order.
 */
void drawTriangleWithBoundingBoxDepth(double ax, double ay, double bx, double by, double cx, double cy,
    double az, double bz, double cz,
    TGAColor colorA, TGAColor colorB, TGAColor colorC,
    TGAImage *framebuffer, double *zbuffer)
{
    InterpolatedColorShader data;
    const Shader shader = interpolatedColorShaderInit(&data, colorA, colorB, colorC);
    const Vec3 screen[3] = {{ax, ay, az}, {bx, by, bz}, {cx, cy, cz}};
    rasterizeScreenTriangle(screen, &shader, framebuffer, zbuffer);
}

/**
 * @brief Rasterize a clip-space triangle after homogeneous division and viewport mapping.
 * @param[in] clip Three vertices transformed by perspective * modelView.
 * @param[in] viewport Screen mapping matrix preserving depth.
 * @param[in] shader Fragment shader; all three vertex-stage calls must already be complete.
 * @param[in,out] framebuffer Destination RGB or RGBA image.
 * @param[in,out] zbuffer Row-major double depth array, initialized to -INFINITY, or NULL.
 * @note Faces with nonfinite vertices or nonpositive w are skipped without near-plane clipping.
 * @note Depth is written only after the fragment shader accepts the sample.
 */
void rasterizeTriangle(const Vec4 clip[3], Mat4 viewport, const Shader *shader,
    TGAImage *framebuffer, double *zbuffer)
{
    Vec3 screen[3];
    for(int i = 0; i < 3; ++i)
        if(!cameraClipToScreen(clip[i], viewport, &screen[i])) return;
    rasterizeScreenTriangle(screen, shader, framebuffer, zbuffer);
}

/**
 * @brief Find the axis-aligned bounding box of three vertices.
 * @note Corner names assume the y axis increases upward.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 * @param[out] bottomLeftX Minimum vertex x coordinate.
 * @param[out] bottomLeftY Minimum vertex y coordinate.
 * @param[out] topRightX Maximum vertex x coordinate.
 * @param[out] topRightY Maximum vertex y coordinate.
 */
static void findTriangleBoundingBox(double ax, double ay, double bx, double by, double cx, double cy,
    double *bottomLeftX, double *bottomLeftY, double *topRightX, double *topRightY)
{
    *bottomLeftX = fmin(ax, fmin(bx, cx));
    *bottomLeftY = fmin(ay, fmin(by, cy));
    *topRightX = fmax(ax, fmax(bx, cx));
    *topRightY = fmax(ay, fmax(by, cy));
}

/**
 * @brief Calculate the signed area of a triangle.
 * @return Positive for counterclockwise winding with y increasing upward,
 *         negative for clockwise winding, or zero for collinear vertices.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 */
static double calculateTriangleAreaSigned(double ax, double ay, double bx, double by, double cx, double cy)
{
    return 0.5 * ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax));
}
