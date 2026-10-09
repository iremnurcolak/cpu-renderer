#pragma once

#include <string_view>

class ObjRenderer
{
public:
    // Render OBJ face edges as a wireframe and save the result to a TGA file.
    // Map x/y coordinates from [-1, 1] to the viewport without fitting the model.
    // Return 0 on success or 1 if the model cannot be read or the image saved.
    int render(
        std::string_view objPath,
        std::string_view outputPath = "obj_framebuffer.tga") const;
};
