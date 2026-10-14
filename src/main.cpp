#include <cstdlib>
#include <chrono>
#include <iostream>
#include <random>
#include <string_view>
#include "colors.h"
#include "line_renderer.h"
#include "obj_renderer.h"
#include "triangle_renderer.h"
#include "tgaimage.h"

constexpr int width  = 128;
constexpr int height = 128;

/**
 * @brief Run the example scene, OBJ renderer, or random-line benchmark.
 * @note OBJ rendering defaults to filled mode; --wireframe selects face boundaries.
 * @note --benchmark runs the line benchmark and cannot be combined with a model.
 * @return 0 on success; 1 for invalid arguments or an OBJ rendering failure.
 * @param[in] argc Number of command-line arguments, including the executable name.
 * @param[in] argv Arguments containing an optional rendering mode and model path.
 */
int main(int argc, char** argv)
{
    std::string_view modelPath;
    std::string_view renderMode;
    bool benchmark = false;
    for(int i = 1; i < argc; ++i)
    {
        const std::string_view argument(argv[i]);
        if(argument == "--wireframe" || argument == "--filled")
        {
            if(!renderMode.empty() && renderMode != argument)
            {
                std::cerr << "Choose either --wireframe or --filled.\n";
                return 1;
            }
            renderMode = argument;
        }
        else if(argument == "--benchmark")
        {
            benchmark = true;
        }
        else if(argument.starts_with("--") || !modelPath.empty())
        {
            std::cerr << "Unexpected argument: " << argument << '\n';
            return 1;
        }
        else
        {
            modelPath = argument;
        }
    }

    if((benchmark && (!modelPath.empty() || !renderMode.empty()))
        || (!renderMode.empty() && modelPath.empty()))
    {
        std::cerr << "Usage: cpu_renderer [--wireframe|--filled] model.obj\n"
                  << "       cpu_renderer [--benchmark]\n";
        return 1;
    }
    if(!modelPath.empty())
    {
        const ObjRenderer renderer;
        return renderMode == "--wireframe"
            ? renderer.renderWireframe(modelPath)
            : renderer.renderFilled(modelPath);
    }

    TGAImage framebuffer(width, height, TGAImage::RGB);

    if(benchmark)
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

    // int ax =  7, ay =  3;
    // int bx = 12, by = 37;
    // int cx = 62, cy = 53;

    // drawLineInterpolated(ax, ay, bx, by, framebuffer, blue);
    // drawLineInterpolated(cx, cy, bx, by, framebuffer, green);
    // drawLineInterpolated(cx, cy, ax, ay, framebuffer, yellow);
    // drawLineInterpolated(ax, ay, cx, cy, framebuffer, red);

    //drawTriangleWithMyScanlineV2(  7, 45, 35, 100, 45,  60, framebuffer, red);
    //drawTriangleWithMyScanlineV2(120, 35, 90,   5, 45, 110, framebuffer, white);
    //drawTriangleWithMyScanlineV2(115, 83, 80,  90, 85, 120, framebuffer, green);

    // framebuffer.set(ax, ay, white);
    // framebuffer.set(bx, by, white);
    // framebuffer.set(cx, cy, white);

    drawTriangleWithBoundingBox(  7, 45, 35, 100, 45,  60, framebuffer, red);
    drawTriangleWithBoundingBox(120, 35, 90,   5, 45, 110, framebuffer, white);
    drawTriangleWithBoundingBox(115, 83, 80,  90, 85, 120, framebuffer, green);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
