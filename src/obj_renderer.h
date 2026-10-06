#pragma once

#include <string_view>

class ObjRenderer
{
public:
    int render(
        std::string_view objPath,
        std::string_view outputPath = "obj_framebuffer.tga") const;
};
