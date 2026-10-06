#include <cstdlib>
#include <chrono>
#include <iostream>
#include <random>
#include <string_view>
#include "line_renderer.h"
#include "obj_renderer.h"
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};
int main(int argc, char** argv)
{
    if(argc > 1 && std::string_view(argv[1]) != "--benchmark")
    {
        return ObjRenderer{}.render(argv[1]);
    }

    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    if(argc > 1)
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
