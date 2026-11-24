#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include "mat.h"
#include "tgaimage.h"

/** @brief A fragment's proposed color and whether it should be discarded. */
typedef struct
{
    bool discard;
    TGAColor color;
} FragmentResult;

/**
 * @brief Programmable vertex and fragment stages with caller-owned data.
 * @note vertex writes per-triangle data; fragment must only read that data so
 *       fragments can run in parallel. Do not modify context during rasterization.
 * @note fragment is required for rasterization; vertex may be NULL for screen-space inputs.
 * @note Keep context alive until all vertex and fragment calls have finished.
 */
typedef struct
{
    void *context; /**< Shader-specific matrices, triangle attributes, and material data. */
    /**
     * @brief Transform one vertex and store attributes for the current triangle.
     * @param[in,out] context Shader-specific data.
     * @param[in] position Object-space vertex position.
     * @param[in] vertexIndex Triangle corner index, from 0 through 2.
     * @return Homogeneous clip coordinates, before division by w.
     */
    Vec4 (*vertex)(void *context, Vec3 position, int vertexIndex);
    /**
     * @brief Choose the color of a fragment that passed coverage and depth tests.
     * @param[in] context Shader-specific data, read-only during rasterization.
     * @param[in] barycentric Screen-space weights for triangle corners A, B, and C.
     * @return A color and discard flag; discarded fragments update neither image nor depth.
     */
    FragmentResult (*fragment)(const void *context, Vec3 barycentric);
} Shader;

/** @brief Matrices, eye-space triangle positions, and color for the random-color shader. */
typedef struct
{
    Mat4 modelView;
    Mat4 viewProjection;
    Vec3 triangle[3]; /**< Positions in the camera frame, available for later lighting. */
    TGAColor color;
} RandomShader;

/** @brief Three vertex colors for screen-space barycentric color interpolation. */
typedef struct
{
    TGAColor colors[3];
} InterpolatedColorShader;

/**
 * @brief Initialize a random-color shader and bind its vertex and fragment callbacks.
 * @param[out] data Caller-owned shader storage, which must outlive the returned interface.
 * @param[in] modelView Object-to-camera transformation.
 * @param[in] perspective Camera-to-clip transformation; identity for orthographic mode.
 * @return An interface pointing to data; call randomShaderBeginTriangle before drawing.
 */
Shader randomShaderInit(RandomShader *data, Mat4 modelView, Mat4 perspective);

/**
 * @brief Assign one random opaque RGB color to the next triangle.
 * @param[in,out] data Initialized shader data, not currently being rasterized.
 * @note Uses rand(); the application owns random-number seeding.
 */
void randomShaderBeginTriangle(RandomShader *data);

/**
 * @brief Initialize a fragment shader that interpolates three colors in screen space.
 * @param[out] data Caller-owned storage, which must outlive the returned interface.
 * @param[in] colorA Color at triangle corner A.
 * @param[in] colorB Color at triangle corner B.
 * @param[in] colorC Color at triangle corner C.
 * @return A fragment-only interface; B/G/R are interpolated and output alpha is 255.
 */
Shader interpolatedColorShaderInit(InterpolatedColorShader *data,
    TGAColor colorA, TGAColor colorB, TGAColor colorC);

#endif /* SHADER_H */
