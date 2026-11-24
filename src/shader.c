#include "shader.h"

#include <stdlib.h>

/**
 * @brief Store an eye-space triangle corner and return its clip coordinates.
 * @param[in,out] context RandomShader data.
 * @param[in] position Object-space position.
 * @param[in] vertexIndex Corner index from 0 through 2.
 * @return The position transformed by perspective * modelView, without division by w.
 */
static Vec4 randomShaderVertex(void *context, Vec3 position, int vertexIndex)
{
    RandomShader *data = context;
    const Vec4 vertex = {position.x, position.y, position.z, 1.0};
    const Vec4 eye = mat4MultiplyVec4(data->modelView, vertex);
    data->triangle[vertexIndex] = (Vec3){eye.x, eye.y, eye.z};
    return mat4MultiplyVec4(data->viewProjection, vertex);
}

/**
 * @brief Return the current triangle's random color without discarding the fragment.
 * @param[in] context RandomShader data.
 * @param[in] barycentric Unused screen-space interpolation weights.
 * @return The triangle's constant opaque color and a false discard flag.
 */
static FragmentResult randomShaderFragment(const void *context, Vec3 barycentric)
{
    const RandomShader *data = context;
    (void)barycentric;
    return (FragmentResult){false, data->color};
}

/**
 * @brief Interpolate B/G/R channels using screen-space barycentric coordinates.
 * @param[in] context InterpolatedColorShader data.
 * @param[in] barycentric Weights corresponding to colors A, B, and C.
 * @return An opaque interpolated color and a false discard flag.
 */
static FragmentResult interpolatedColorFragment(const void *context, Vec3 barycentric)
{
    const InterpolatedColorShader *data = context;
    TGAColor color = {{0, 0, 0, 255}};
    for(int channel = 0; channel < 3; ++channel)
    {
        const double value = barycentric.x * data->colors[0].bgra[channel]
            + barycentric.y * data->colors[1].bgra[channel]
            + barycentric.z * data->colors[2].bgra[channel];
        color.bgra[channel] = (uint8_t)(value < 0.0 ? 0.0 : value > 255.0 ? 255.0 : value);
    }
    return (FragmentResult){false, color};
}

/**
 * @brief Initialize a random-color shader and bind its vertex and fragment callbacks.
 * @param[out] data Caller-owned shader storage, which must outlive the returned interface.
 * @param[in] modelView Object-to-camera transformation.
 * @param[in] perspective Camera-to-clip transformation; identity for orthographic mode.
 * @return An interface pointing to data; call randomShaderBeginTriangle before drawing.
 */
Shader randomShaderInit(RandomShader *data, Mat4 modelView, Mat4 perspective)
{
    *data = (RandomShader){.modelView = modelView,
        .viewProjection = mat4Multiply(perspective, modelView)};
    return (Shader){data, randomShaderVertex, randomShaderFragment};
}

/**
 * @brief Assign one random opaque RGB color to the next triangle.
 * @param[in,out] data Initialized shader data, not currently being rasterized.
 * @note Uses rand(); the application owns random-number seeding.
 */
void randomShaderBeginTriangle(RandomShader *data)
{
    data->color = (TGAColor){{(uint8_t)(rand() % 256), (uint8_t)(rand() % 256),
        (uint8_t)(rand() % 256), 255}};
}

/**
 * @brief Initialize a fragment shader that interpolates three colors in screen space.
 * @param[out] data Caller-owned storage, which must outlive the returned interface.
 * @param[in] colorA Color at triangle corner A.
 * @param[in] colorB Color at triangle corner B.
 * @param[in] colorC Color at triangle corner C.
 * @return A fragment-only interface; B/G/R are interpolated and output alpha is 255.
 */
Shader interpolatedColorShaderInit(InterpolatedColorShader *data,
    TGAColor colorA, TGAColor colorB, TGAColor colorC)
{
    *data = (InterpolatedColorShader){{colorA, colorB, colorC}};
    return (Shader){data, NULL, interpolatedColorFragment};
}
