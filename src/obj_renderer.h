#ifndef OBJ_RENDERER_H
#define OBJ_RENDERER_H

/** @brief Projection applied after model rotation and before viewport mapping. */
typedef enum
{
    OBJ_PROJECTION_ORTHOGRAPHIC, /**< Viewport mapping without perspective division. */
    OBJ_PROJECTION_PERSPECTIVE   /**< Perspective division followed by viewport mapping. */
} ObjProjection;

/**
 * @brief Render OBJ face boundaries as white line segments into an 800 x 800 TGA.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA path; NULL uses obj_wireframe_<projection>.tga.
 * @param[in] projection Orthographic or perspective-plus-viewport projection.
 * @return 0 on success; 1 for invalid arguments, allocation failures, or file errors.
 * @note Normalized x/y map to an inset viewport without automatic fitting.
 * @note Camera eye=(-1,0,2), center=(0,0,0), up=(0,1,0); focal distance is |eye-center|.
 */
int renderObjWireframe(const char *objPath, const char *outputPath, ObjProjection projection);

/**
 * @brief Render OBJ faces as randomly colored filled triangles and optional depth output.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA path; NULL uses obj_filled_<projection>.tga.
 * @param[in] zbufferPath Depth TGA path; NULL uses obj_zbuffer_<projection>.tga; "" disables depth.
 * @param[in] projection Orthographic or perspective-plus-viewport projection.
 * @return 0 on success; 1 for invalid arguments, allocation failures, or file errors.
 * @pre Polygonal faces must be convex for fan triangulation.
 * @note Back-facing triangles and triangles with projected determinant below 1 are skipped.
 * @note Camera eye=(-1,0,2), center=(0,0,0), up=(0,1,0); focal distance is |eye-center|.
 * @note Viewport mapping preserves z; the double depth buffer stores unscaled z/w.
 * @note Larger depth wins even without depth image output; visualization alone is clamped.
 * @note No lighting or textures are applied.
 */
int renderObjFilled(const char *objPath, const char *outputPath, const char *zbufferPath,
    ObjProjection projection);

#endif
