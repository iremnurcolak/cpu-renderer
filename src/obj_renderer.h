#pragma once

#include <string_view>

class ObjRenderer
{
public:
    /**
     * @brief Render OBJ face boundaries as white line segments.
     * @note Output is an 800 x 800 TGA image.
     * @note Model x/y coordinates map from [-1, 1] without automatic fitting.
     * @return 0 on success; 1 if the input cannot be opened or output cannot be saved.
     * @param[in] objPath OBJ path or filename resolved under obj/.
     * @param[in] outputPath Destination TGA file path.
     */
    int renderWireframe(
        std::string_view objPath,
        std::string_view outputPath = "obj_wireframe.tga") const;

    /**
     * @brief Render OBJ faces as randomly colored filled triangles.
     * @pre Polygonal faces must be convex for fan triangulation.
     * @note Back-facing triangles and triangles with signed area below 1 are skipped.
     * @note A grayscale image maps z from [-1, 1] to [0, 255]; larger z is brighter.
     * @note No depth testing, lighting, or textures are applied.
     * @note Model x/y coordinates map from [-1, 1] without automatic fitting.
     * @return 0 on success; 1 if the input cannot be opened or output cannot be saved.
     * @param[in] objPath OBJ path or filename resolved under obj/.
     * @param[in] outputPath Destination TGA file path.
     * @param[in] zbufferPath Destination grayscale depth TGA; empty disables depth output.
     */
    int renderFilled(
        std::string_view objPath,
        std::string_view outputPath = "obj_filled.tga",
        std::string_view zbufferPath = "obj_zbuffer.tga") const;
};
