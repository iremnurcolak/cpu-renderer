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
 * @note Transformed x/y map from [-1, 1] without automatic fitting.
 * @note Vertices are rotated 30 degrees about Y; perspective mode uses camera distance 3.
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
 * @note Back-facing triangles and triangles with signed area below 1 are skipped.
 * @note Vertices are rotated 30 degrees about Y; perspective mode uses camera distance 3.
 * @note Transformed x/y map to the viewport and z maps to grayscale depth.
 * @note Only pixels with a larger depth update both the color and depth images.
 * @note No lighting or textures are applied.
 */
int renderObjFilled(const char *objPath, const char *outputPath, const char *zbufferPath,
    ObjProjection projection);

#endif
