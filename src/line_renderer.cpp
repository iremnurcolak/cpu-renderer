#include "line_renderer.h"
#include "colors.h"

#include <cmath>
#include <utility>

/**
 * @brief Draw a segment using fixed-step floating-point sampling.
 * @note Parameter increments of 0.02 can leave gaps on long segments.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in,out] framebuffer Image receiving the rendered pixels.
 * @param[in] color Pixel color in BGRA order.
 */
void drawLineBarycentric(
    int ax,
    int ay,
    int bx,
    int by,
    TGAImage& framebuffer,
    TGAColor color)
{
    for(float t = 0.0F; t <= 1.0F; t += 0.02F)
    {
        const int x = std::round(((1.0F - t) * ax) + (t * bx));
        const int y = std::round(((1.0F - t) * ay) + (t * by));
        framebuffer.set(x, y, color);
    }
}

/**
 * @brief Draw a segment using integer error accumulation.
 * @note Both endpoints are included; coincident endpoints draw one pixel.
 * @pre Coordinate differences and accumulated error must fit in int.
 * @param[in] ax X coordinate of vertex a.
 * @param[in] ay Y coordinate of vertex a.
 * @param[in] bx X coordinate of vertex b.
 * @param[in] by Y coordinate of vertex b.
 * @param[in,out] framebuffer Image receiving the rendered pixels.
 * @param[in] color Pixel color in BGRA order.
 */
void drawLineInterpolated(
    int ax,
    int ay,
    int bx,
    int by,
    TGAImage& framebuffer,
    TGAColor color)
{
    if(ax == bx && ay == by)
    {
        framebuffer.set(ax, ay, color);
        return;
    }

    const bool isSteep = std::abs(ax - bx) < std::abs(ay - by);

    if(isSteep)
    {
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if(ax > bx)
    {
        std::swap(ax, bx);
        std::swap(ay, by);
    }

    int y = ay;
    int ierror = 0;
    for(int x = ax; x <= bx; ++x)
    {
        if(isSteep)
        {
            framebuffer.set(y, x, color);
        }
        else
        {
            framebuffer.set(x, y, color);
        }
        ierror += 2 * std::abs(by - ay);

        // Convert the threshold comparison to 0 or 1 to avoid an explicit branch.
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx - ax) * (ierror > bx - ax);
    }
}
