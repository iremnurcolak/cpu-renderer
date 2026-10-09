#pragma once

#include "tgaimage.h"

// Fill a triangle with floating-point scanline intersections and inclusive spans.
// Vertices may be supplied in any order, but their y coordinates must be distinct.
void drawTriangleWithMyScanlineV1(
    int ax,
    int ay,
    int bx,
    int by,
    int cx,
    int cy,
    TGAImage& framebuffer,
    TGAColor color);

// Fill a triangle with integer scanline intersections, truncating division toward zero.
// Vertices may be supplied in any order. Spans exclude the right endpoint;
// triangles with equal y coordinates at all three vertices draw no pixels.
// Coordinate differences and their products must fit in int.
void drawTriangleWithMyScanlineV2(
    int ax,
    int ay,
    int bx,
    int by,
    int cx,
    int cy,
    TGAImage& framebuffer,
    TGAColor color);
