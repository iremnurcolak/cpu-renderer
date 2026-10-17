#include "triangle_renderer.h"
#include "line_renderer.h"
#include "colors.h"

#include <algorithm>

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
static void findTriangleBoundingBox(int ax, int ay, int bx, int by, int cx, int cy,
                                    int &bottomLeftX,
                                    int &bottomLeftY,
                                    int &topRightX,
                                    int &topRightY);
/**
 * @brief Calculate the signed area of a triangle.
 * @pre Integer intermediate values must fit in int.
 * @return Positive for counterclockwise winding with y increasing upward,
 *         negative for clockwise winding, or zero for collinear vertices.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 */
static double calculateTriangleAreaSigned(int ax, int ay, int bx, int by, int cx, int cy);

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
                                  TGAImage &framebuffer,
                                  TGAColor color)
{
    // sort the vertices, a,b,c in ascending y order
    if (ay > by)
    {
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    if (ay > cy)
    {
        std::swap(ax, cx);
        std::swap(ay, cy);
    }
    if (by > cy)
    {
        std::swap(bx, cx);
        std::swap(by, cy);
    }

    for (int y = ay; y <= by; y++)
    {
        const float t1 = (float(y) - ay) / (float(by) - ay);
        float x1;
        if (t1 >= 0.0F && t1 <= 1.0F)
        {
            x1 = ax + t1 * (float(bx) - ax);
            // Intersection at the current scanline.
        }

        const float t2 = (float(y) - ay) / (float(cy) - ay);
        float x2;
        if (t2 >= 0.0F && t2 <= 1.0F)
        {
            x2 = ax + t2 * (float(cx) - ax);
            // Intersection at the current scanline.
        }

        drawLineInterpolated(x1, y, x2, y, framebuffer, color);
    }

    for (int y = by; y <= cy; y++)
    {
        const float t1 = (float(y) - by) / (float(cy) - by);
        float x1;
        if (t1 >= 0.0F && t1 <= 1.0F)
        {
            x1 = bx + t1 * (float(cx) - bx);
            // cross: (x, a)
        }

        const float t2 = (float(y) - ay) / (float(cy) - ay);
        float x2;
        if (t2 >= 0.0F && t2 <= 1.0F)
        {
            x2 = ax + t2 * (float(cx) - ax);
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
                                  TGAImage &framebuffer,
                                  TGAColor color)
{
    // sort the vertices, a,b,c in ascending y order
    if (ay > by)
    {
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    if (ay > cy)
    {
        std::swap(ax, cx);
        std::swap(ay, cy);
    }
    if (by > cy)
    {
        std::swap(bx, cx);
        std::swap(by, cy);
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
            for (int x = std::min(x1, x2); x < std::max(x1, x2); x++) // draw a horizontal line
                framebuffer.set(x, y, color);
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
            for (int x = std::min(x1, x2); x < std::max(x1, x2); x++) // draw a horizontal line
                framebuffer.set(x, y, color);
        }
    }
}

/**
 * @brief Fill a triangle with barycentrically interpolated RGB vertex colors.
 * @pre Integer intermediate values in the area calculation must fit in int.
 * @note Triangles with signed area below 1 pixel squared are skipped.
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
                                const TGAColor& colorA, const TGAColor& colorB, const TGAColor& colorC,
                                TGAImage &framebuffer)
{
    TGAImage unusedDepth;
    drawTriangleWithBoundingBox(ax, ay, bx, by, cx, cy, 0.0, 0.0, 0.0,
        colorA, colorB, colorC, framebuffer, unusedDepth);
}

/**
 * @brief Fill a colored triangle and write interpolated depth to a grayscale image.
 * @pre Integer intermediate values in the area calculation must fit in int.
 * @note Triangles with signed area below 1 pixel squared are skipped.
 * @note With y increasing upward, only counterclockwise triangles are drawn.
 * @note Triangle edges are included. B, G, and R channels are interpolated separately.
 * @note Output alpha is 255; supply an RGB or RGBA framebuffer for full color.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 * @param[in] az Depth at vertex A, mapped to [0, 255].
 * @param[in] bz Depth at vertex B, mapped to [0, 255].
 * @param[in] cz Depth at vertex C, mapped to [0, 255].
 * @param[in,out] framebuffer Image receiving the rendered pixels.
 * @param[in,out] zbuffer Grayscale image with framebuffer dimensions, or an empty image.
 * @note Depth is clamped to [0, 255] and written without a visibility test.
 * @note Later triangles overwrite earlier depth samples where they overlap.
 * @param[in] colorA Color at vertex A in BGRA order.
 * @param[in] colorB Color at vertex B in BGRA order.
 * @param[in] colorC Color at vertex C in BGRA order.
 */
void drawTriangleWithBoundingBox(int ax, int ay, int bx, int by, int cx, int cy,
    double az, double bz, double cz,
    const TGAColor& colorA, const TGAColor& colorB, const TGAColor& colorC,
    TGAImage& framebuffer, TGAImage& zbuffer)
{
    int bottomLeftX;
    int bottomLeftY;
    int topRightX;
    int topRightY;
    findTriangleBoundingBox(ax, ay, bx, by, cx, cy, bottomLeftX, bottomLeftY, topRightX, topRightY);

    double totalArea = calculateTriangleAreaSigned(ax, ay, bx, by, cx, cy);

    // do not draw back-face triangles and triangles that has area less than 1 pixel
    // negative area means the normal of the triangle points back
    if(totalArea < 1)
    {
        return;
    }
#pragma omp parallel for

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
                //the vertex is not inside the triangle
            }
            
            TGAColor color;
            for(int channel = 0; channel < 3; ++channel)
            {
                const double value = alpha * colorA.bgra[channel]
                    + beta * colorB.bgra[channel]
                    + gamma * colorC.bgra[channel];
                color.bgra[channel] = static_cast<std::uint8_t>(std::clamp(value, 0.0, 255.0));
            }
            color.bgra[3] = 255;
            framebuffer.set(x, y, color);
            if(zbuffer.width() > 0)
            {
                const auto z = static_cast<std::uint8_t>(
                    std::clamp(alpha * az + beta * bz + gamma * cz, 0.0, 255.0));
                if (z <= zbuffer.get(x, y)[0]) continue;
                zbuffer.set(x, y, {z});
            }
        }
    }
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
static void findTriangleBoundingBox(
    int ax,
    int ay,
    int bx,
    int by,
    int cx,
    int cy,
    int &bottomLeftX,
    int &bottomLeftY,
    int &topRightX,
    int &topRightY)
{
    if (ay > by)
    {
        std::swap(ay, by);
    }
    if (ay > cy)
    {
        std::swap(ay, cy);
    }
    if (by > cy)
    {
        std::swap(by, cy);
    }

    if (ax > bx)
    {
        std::swap(ax, bx);
    }
    if (ax > cx)
    {
        std::swap(ax, cx);
    }
    if (bx > cx)
    {
        std::swap(bx, cx);
    }

    bottomLeftX = ax;
    bottomLeftY = ay;
    topRightX = cx;
    topRightY = cy;
}

/**
 * @brief Calculate the signed area of a triangle.
 * @pre Integer intermediate values must fit in int.
 * @return Positive for counterclockwise winding with y increasing upward,
 *         negative for clockwise winding, or zero for collinear vertices.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in] cx X coordinate of vertex c.
 * @param[in] cy Y coordinate of vertex c.
 */
static double calculateTriangleAreaSigned(int ax, int ay, int bx, int by, int cx, int cy)
{
    return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}
