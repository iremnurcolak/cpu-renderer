#pragma once

#include "tgaimage.h"

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
    TGAColor color);

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
    TGAColor color);
