#include <cmath>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <random>
#include <string_view>
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};


void drawLineBarycentric(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color)
{
    for(float t = 0.0; t <= 1; t+=.02)
    {
        int x = std::round(((1-t) * ax) + (t * bx));
        int y = std::round(((1-t) * ay) + (t * by));
        framebuffer.set(x, y, color);
    }
}

void drawLineInterpolated(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color)
{
    if(ax == bx && ay == by)
    {
        framebuffer.set(ax, ay, color);
        return;
    }
    bool isSteep = std::abs(ax-bx) < std::abs(ay-by);
    if(isSteep)
    {
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if(ax>bx)
    {
        std::swap(ax, bx);
        std::swap(ay, by);
    }

    for(int x = ax; x <= bx; x++)
    {
        float t = (x - ax) / static_cast<float>(bx - ax);
        int y = std::round(((1-t) * ay) + (t * by));
        if(isSteep)
        {
            framebuffer.set(y, x, color);
        }
        else
        {
            framebuffer.set(x, y, color);
        }
    }
}

int main(int argc, char** argv)
{
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    if(argc > 1 && std::string_view(argv[1]) == "--benchmark")
    {
        constexpr int lineCount = 16'000'000;
        std::mt19937 random(42);
        std::uniform_int_distribution<int> randomX(0, width - 1);
        std::uniform_int_distribution<int> randomY(0, height - 1);
        std::srand(42);

        const auto start = std::chrono::steady_clock::now();
        for(int i = 0; i < lineCount; ++i)
        {
            const int ax = randomX(random);
            const int ay = randomY(random);
            const int bx = randomX(random);
            const int by = randomY(random);
            drawLineInterpolated(ax, ay, bx, by, framebuffer, {
                static_cast<std::uint8_t>(std::rand() % 255),
                static_cast<std::uint8_t>(std::rand() % 255),
                static_cast<std::uint8_t>(std::rand() % 255),
                static_cast<std::uint8_t>(std::rand() % 255)
            });
        }
        const auto end = std::chrono::steady_clock::now();
        const double seconds = std::chrono::duration<double>(end - start).count();
        std::cout << lineCount << " lines in " << seconds << " seconds\n";
        framebuffer.write_tga_file("framebuffer.tga");
        return 0;
    }

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    drawLineInterpolated(ax, ay, bx, by, framebuffer, blue);
    drawLineInterpolated(cx, cy, bx, by, framebuffer, green);
    drawLineInterpolated(cx, cy, ax, ay, framebuffer, yellow);
    drawLineInterpolated(ax, ay, cx, cy, framebuffer, red);

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
