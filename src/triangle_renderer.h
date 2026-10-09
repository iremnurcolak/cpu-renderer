#pragma once

#include "tgaimage.h"

// Draw a filled triangle using three endpoints in framebuffer coordinates.
// Scaffold only: the implementation does not draw pixels yet.
void drawTriangleWithMyScanlineV1(
    int ax,
    int ay,
    int bx,
    int by,
    int cx,
    int cy,
    TGAImage& framebuffer,
    TGAColor color);

void drawTriangleWithMyScanlineV2(
    int ax,
    int ay,
    int bx,
    int by,
    int cx,
    int cy,
    TGAImage& framebuffer,
    TGAColor color);
