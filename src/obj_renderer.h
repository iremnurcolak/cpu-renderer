#ifndef OBJ_RENDERER_H
#define OBJ_RENDERER_H

/**
 * @brief Render OBJ face boundaries as white line segments into an 800 x 800 TGA.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA path; NULL uses obj_wireframe.tga.
 * @return 0 on success; 1 for invalid arguments, allocation failures, or file errors.
 * @note Model x/y coordinates map from [-1, 1] without automatic fitting.
 */
int renderObjWireframe(const char *objPath, const char *outputPath);

/**
 * @brief Render OBJ faces as randomly colored filled triangles and optional depth output.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA path; NULL uses obj_filled.tga.
 * @param[in] zbufferPath Depth TGA path; NULL uses obj_zbuffer.tga; "" disables depth.
 * @return 0 on success; 1 for invalid arguments, allocation failures, or file errors.
 * @pre Polygonal faces must be convex for fan triangulation.
 * @note Back-facing triangles and triangles with signed area below 1 are skipped.
 * @note Model x/y coordinates map from [-1, 1]; z maps to grayscale [0, 255].
 * @note Depth retains the largest value, but colors are written before the depth check.
 * @note No lighting or textures are applied.
 */
int renderObjFilled(const char *objPath, const char *outputPath, const char *zbufferPath);

#endif
