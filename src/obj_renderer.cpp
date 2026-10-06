#include <iostream>

#include "line_renderer.h"
#include "tgaimage.h"

namespace
{
constexpr int framebufferWidth = 800;
constexpr int framebufferHeight = 800;
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cerr << "Usage: obj_renderer <model.obj>\n";
        return 1;
    }

    const char* objPath = argv[1];
    TGAImage framebuffer(framebufferWidth, framebufferHeight, TGAImage::RGB);

    // TODO: Read vertices and faces from objPath, project each vertex to the
    // framebuffer, and draw the face edges with drawLineInterpolated.
    static_cast<void>(objPath);

    if(!framebuffer.write_tga_file("obj_framebuffer.tga"))
    {
        std::cerr << "Failed to write obj_framebuffer.tga\n";
        return 1;
    }

    return 0;
}
