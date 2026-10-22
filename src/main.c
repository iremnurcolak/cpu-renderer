#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "colors.h"
#include "line_renderer.h"
#include "obj_renderer.h"
#include "triangle_renderer.h"

static const int width = 128;
static const int height = 128;

/**
 * @brief Run the RGB triangle example, OBJ renderer, or line benchmark.
 * @note Without arguments, save three color-interpolated triangles to framebuffer.tga.
 * @note The example and benchmark use a 128 x 128 RGB framebuffer.
 * @note OBJ rendering defaults to filled mode; --wireframe selects face boundaries.
 * @note --benchmark runs the line benchmark and cannot be combined with a model.
 * @return 0 on success; 1 for invalid arguments, allocation failure, or file errors.
 * @param[in] argc Number of command-line arguments, including the executable name.
 * @param[in] argv Arguments containing an optional rendering mode and model path.
 */
int main(int argc, char **argv)
{
    const char *modelPath = NULL;
    const char *renderMode = NULL;
    bool benchmark = false;
    for(int i = 1; i < argc; ++i)
    {
        const char *argument = argv[i];
        if(strcmp(argument, "--wireframe") == 0 || strcmp(argument, "--filled") == 0)
        {
            if(renderMode && strcmp(renderMode, argument) != 0)
            {
                fprintf(stderr, "Choose either --wireframe or --filled.\n");
                return 1;
            }
            renderMode = argument;
        }
        else if(strcmp(argument, "--benchmark") == 0)
            benchmark = true;
        else if(strncmp(argument, "--", 2) == 0 || modelPath)
        {
            fprintf(stderr, "Unexpected argument: %s\n", argument);
            return 1;
        }
        else
            modelPath = argument;
    }
    if((benchmark && (modelPath || renderMode)) || (renderMode && !modelPath))
    {
        fprintf(stderr, "Usage: cpu_renderer [--wireframe|--filled] model.obj\n"
                        "       cpu_renderer [--benchmark]\n");
        return 1;
    }
    if(modelPath)
    {
        srand((unsigned)time(NULL));
        return renderMode && strcmp(renderMode, "--wireframe") == 0
            ? renderObjWireframe(modelPath, NULL)
            : renderObjFilled(modelPath, NULL, NULL);
    }
    TGAImage framebuffer = {0};
    if(!tgaImageInit(&framebuffer, width, height, TGA_RGB))
    {
        fprintf(stderr, "Failed to allocate framebuffer.\n");
        return 1;
    }
    if(benchmark)
    {
        const int lineCount = 16000000;
        srand(42);
        const clock_t start = clock();
        for(int i = 0; i < lineCount; ++i)
        {
            const int ax = rand() % width;
            const int ay = rand() % height;
            const int bx = rand() % width;
            const int by = rand() % height;
            const TGAColor color = {{(uint8_t)(rand() % 255), (uint8_t)(rand() % 255),
                (uint8_t)(rand() % 255), (uint8_t)(rand() % 255)}};
            drawLineInterpolated(ax, ay, bx, by, &framebuffer, color);
        }
        const double seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
        printf("%d lines in %.6f CPU seconds\n", lineCount, seconds);
    }
    else
    {
        drawTriangleWithBoundingBox(7,45,45,60,35,100,red,green,blue,&framebuffer);
        drawTriangleWithBoundingBox(120,35,45,110,90,5,blue,red,green,&framebuffer);
        drawTriangleWithBoundingBox(115,83,85,120,80,90,green,blue,red,&framebuffer);
    }
    const bool success = tgaImageWrite(&framebuffer, "framebuffer.tga", true, true);
    tgaImageDestroy(&framebuffer);
    if(!success)
        fprintf(stderr, "Failed to write framebuffer.tga.\n");
    return success ? 0 : 1;
}
