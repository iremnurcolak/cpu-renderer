#include "line_renderer.h"
#include "colors.h"

#include <math.h>
#include <stdlib.h>

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
    TGAImage * framebuffer,
    TGAColor color)
{
    for(float t = 0.0F; t <= 1.0F; t += 0.02F)
    {
        const int x = roundf(((1.0F - t) * ax) + (t * bx));
        const int y = roundf(((1.0F - t) * ay) + (t * by));
        tgaImageSet(framebuffer, x, y, color);
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
    TGAImage * framebuffer,
    TGAColor color)
{
    if(ax == bx && ay == by)
    {
        tgaImageSet(framebuffer, ax, ay, color);
        return;
    }

    const bool isSteep = abs(ax - bx) < abs(ay - by);

    if(isSteep)
    {
        swapInt(&ax, &ay);
        swapInt(&bx, &by);
    }
    if(ax > bx)
    {
        swapInt(&ax, &bx);
        swapInt(&ay, &by);
    }

    int y = ay;
    int ierror = 0;
    for(int x = ax; x <= bx; ++x)
    {
        if(isSteep)
        {
            tgaImageSet(framebuffer, y, x, color);
        }
        else
        {
            tgaImageSet(framebuffer, x, y, color);
        }
        ierror += 2 * abs(by - ay);

        // Convert the threshold comparison to 0 or 1 to avoid an explicit branch.
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx - ax) * (ierror > bx - ax);
    }
}
