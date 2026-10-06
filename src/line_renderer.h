#pragma once

#include "tgaimage.h"

void drawLineBarycentric(
    int ax,
    int ay,
    int bx,
    int by,
    TGAImage& framebuffer,
    TGAColor color);

void drawLineInterpolated(
    int ax,
    int ay,
    int bx,
    int by,
    TGAImage& framebuffer,
    TGAColor color);
