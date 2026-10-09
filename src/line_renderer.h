#pragma once

#include "tgaimage.h"

// Sample the segment at parameter increments of 0.02 and round to pixel coordinates.
// This approximate method can leave gaps on long segments.
void drawLineBarycentric(
    int ax,
    int ay,
    int bx,
    int by,
    TGAImage& framebuffer,
    TGAColor color);

// Draw a segment in any direction using integer error accumulation.
// Both endpoints are included; coincident endpoints draw a single pixel.
void drawLineInterpolated(
    int ax,
    int ay,
    int bx,
    int by,
    TGAImage& framebuffer,
    TGAColor color);
