#pragma once

#include "tgaimage.h"

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
void drawTriangleWithMyScanlineV1(
    int ax,
    int ay,
    int bx,
    int by,
    int cx,
    int cy,
    TGAImage& framebuffer,
    TGAColor color);

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
void drawTriangleWithMyScanlineV2(
    int ax,
    int ay,
    int bx,
    int by,
    int cx,
    int cy,
    TGAImage& framebuffer,
    TGAColor color);

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
    TGAImage& framebuffer);
