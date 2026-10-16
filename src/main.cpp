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
 * @brief Run the RGB triangle example, OBJ renderer, or line benchmark.
 * @note Without arguments, save three color-interpolated triangles to framebuffer.tga.
 * @note The example and benchmark use a 128 x 128 RGB framebuffer.
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


    drawTriangleWithBoundingBox(  7, 45, 45,  60, 35, 100, red, green, blue, framebuffer);
    drawTriangleWithBoundingBox(120, 35, 45, 110, 90,   5, blue, red, green, framebuffer);
    drawTriangleWithBoundingBox(115, 83, 85, 120, 80,  90, green, blue, red, framebuffer);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
