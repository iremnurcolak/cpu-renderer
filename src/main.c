#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "colors.h"
#include "obj_renderer.h"
#include "triangle_renderer.h"

static const int width = 128;
static const int height = 128;

/** @brief Drawing mode selected for OBJ input. */
typedef enum
{
    RENDER_FILLED,
    RENDER_WIREFRAME
} RenderMode;

/** @brief Parsed command-line options with filled perspective defaults. */
typedef struct
{
    const char *modelPath;
    RenderMode renderMode;
    ObjProjection projection;
} ProgramOptions;

/** @brief Whether argument parsing succeeded, requested help, or found an error. */
typedef enum
{
    PARSE_OK,
    PARSE_HELP,
    PARSE_ERROR
} ParseResult;

/**
 * @brief Print command-line usage, options, output filenames, and examples.
 * @param[in] stream Destination stream, such as stdout or stderr.
 * @param[in] program Executable name displayed in usage examples.
 */
static void printHelp(FILE *stream, const char *program)
{
    fprintf(stream,
        "Usage:\n"
        "  %s\n"
        "  %s [--wireframe | --filled] [--orthographic | --perspective] <model.obj>\n"
        "  %s --help\n\n"
        "Options:\n"
        "  -h, --help   Show this help and exit without rendering.\n"
        "  --wireframe  Draw OBJ face boundaries in white.\n"
        "  --filled     Draw filled OBJ triangles (default for model input).\n"
        "  --orthographic  Apply the camera frame and viewport without perspective.\n"
        "  --perspective   Apply the camera frame, perspective, and viewport (default).\n\n"
        "Without arguments, render the RGB triangle example.\n"
        "OBJ camera: eye=(-1,0,2), center=(0,0,0), up=(0,1,0); inset viewport.\n"
        "Mode flags may appear before or after the model path.\n"
        "--wireframe and --filled require a model and cannot be combined.\n"
        "--orthographic and --perspective require a model and cannot be combined.\n\n"
        "Model paths:\n"
        "  Use an explicit path, or a filename searched under obj/ and obj/<stem>/.\n\n"
        "Output files (saved in the working directory):\n"
        "  Example:             framebuffer.tga\n"
        "  Wireframe:           obj_wireframe_<projection>.tga\n"
        "  Filled:              obj_filled_<projection>.tga\n"
        "  Grayscale depth:     obj_zbuffer_<projection>.tga\n"
        "  <projection> is orthographic or perspective.\n\n"
        "Examples:\n"
        "  %s --wireframe --orthographic diablo3_pose.obj\n"
        "  %s --filled --perspective obj/african_head/african_head.obj\n",
        program, program, program, program, program);
}

/**
 * @brief Parse and validate command-line options without rendering.
 * @param[in] argc Number of command-line arguments, including the executable name.
 * @param[in] argv Argument strings; a model path may precede or follow mode flags.
 * @param[out] options Parsed options, defaulting to filled perspective rendering.
 * @return PARSE_OK on success, PARSE_HELP for help, or PARSE_ERROR after reporting an error.
 * @note Help takes precedence over other arguments. Explicit modes require a model.
 */
static ParseResult parseArguments(int argc, char **argv, ProgramOptions *options)
{
    *options = (ProgramOptions){.renderMode = RENDER_FILLED,
        .projection = OBJ_PROJECTION_PERSPECTIVE};
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
            return PARSE_HELP;
    }

    bool renderModeSpecified = false;
    bool projectionSpecified = false;
    for(int i = 1; i < argc; ++i)
    {
        const char *argument = argv[i];
        if(strcmp(argument, "--wireframe") == 0 || strcmp(argument, "--filled") == 0)
        {
            const RenderMode mode = strcmp(argument, "--wireframe") == 0
                ? RENDER_WIREFRAME : RENDER_FILLED;
            if(renderModeSpecified && options->renderMode != mode)
            {
                fprintf(stderr, "Choose either --wireframe or --filled.\n");
                fprintf(stderr, "Run '%s --help' for usage.\n", argv[0]);
                return PARSE_ERROR;
            }
            options->renderMode = mode;
            renderModeSpecified = true;
        }
        else if(strcmp(argument, "--orthographic") == 0 || strcmp(argument, "--perspective") == 0)
        {
            const ObjProjection projection = strcmp(argument, "--orthographic") == 0
                ? OBJ_PROJECTION_ORTHOGRAPHIC : OBJ_PROJECTION_PERSPECTIVE;
            if(projectionSpecified && options->projection != projection)
            {
                fprintf(stderr, "Choose either --orthographic or --perspective.\n");
                fprintf(stderr, "Run '%s --help' for usage.\n", argv[0]);
                return PARSE_ERROR;
            }
            options->projection = projection;
            projectionSpecified = true;
        }
        else if(argument[0] == '-' || options->modelPath)
        {
            fprintf(stderr, "Unexpected argument: %s\n", argument);
            fprintf(stderr, "Run '%s --help' for usage.\n", argv[0]);
            return PARSE_ERROR;
        }
        else
            options->modelPath = argument;
    }
    if((renderModeSpecified || projectionSpecified) && !options->modelPath)
    {
        printHelp(stderr, argv[0]);
        return PARSE_ERROR;
    }
    return PARSE_OK;
}

/**
 * @brief Render the selected OBJ model using parsed drawing and projection modes.
 * @param[in] options Validated options containing a model path.
 * @return 0 on success or 1 if model loading, allocation, or output fails.
 * @note Seeds the random triangle colors once before rendering.
 */
static int renderModel(const ProgramOptions *options)
{
    srand((unsigned)time(NULL));
    if(options->renderMode == RENDER_WIREFRAME)
        return renderObjWireframe(options->modelPath, NULL, options->projection);
    return renderObjFilled(options->modelPath, NULL, NULL, options->projection);
}

/**
 * @brief Render three RGB-interpolated triangles to the 128 x 128 example image.
 * @return 0 after saving framebuffer.tga, or 1 on allocation or output failure.
 */
static int renderExample(void)
{
    TGAImage framebuffer = {0};
    if(!tgaImageInit(&framebuffer, width, height, TGA_RGB))
    {
        fprintf(stderr, "Failed to allocate framebuffer.\n");
        return 1;
    }
    drawTriangleWithBoundingBox(7, 45, 45, 60, 35, 100, red, green, blue, &framebuffer);
    drawTriangleWithBoundingBox(120, 35, 45, 110, 90, 5, blue, red, green, &framebuffer);
    drawTriangleWithBoundingBox(115, 83, 85, 120, 80, 90, green, blue, red, &framebuffer);
    const bool success = tgaImageWrite(&framebuffer, "framebuffer.tga", true, true);
    tgaImageDestroy(&framebuffer);
    if(!success)
        fprintf(stderr, "Failed to write framebuffer.tga.\n");
    else
        printf("Saved image: framebuffer.tga\n");
    return success ? 0 : 1;
}

/**
 * @brief Parse options and dispatch to the OBJ renderer or example scene.
 * @param[in] argc Number of command-line arguments, including the executable name.
 * @param[in] argv Arguments containing optional drawing/projection flags and a model path.
 * @return 0 on success or help; 1 for invalid arguments or rendering failure.
 */
int main(int argc, char **argv)
{
    ProgramOptions options;
    const ParseResult result = parseArguments(argc, argv, &options);
    if(result == PARSE_HELP)
    {
        printHelp(stdout, argv[0]);
        return 0;
    }
    if(result == PARSE_ERROR)
        return 1;
    if(options.modelPath)
        return renderModel(&options);
    return renderExample();
}
