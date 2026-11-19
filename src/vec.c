#include "vec.h"
#include "mat.h"
#include <math.h>

/**
 * @brief Map a transformed 3D vertex to integer screen coordinates and depth.
 * @param[in] v Vertex after rotation and optional perspective division.
 * @param[in] width Positive framebuffer width.
 * @param[in] height Positive framebuffer height.
 * @return Integer coordinates ((v.x + 1) * width / 2,
 *         (v.y + 1) * height / 2, (v.z + 1) * 255 / 2), truncated toward zero.
 * @pre Mapped x, y, and z must be finite and representable as int.
 * @note No perspective division or clamping is performed, matching the tutorial.
 * @note The positive viewport boundary maps to width/height, outside valid pixel indices.
 */
ProjectedVertex vec3Project(Vec3 v, int width, int height)
{
    const ProjectedVertex projected = {
        (int)((v.x + 1.0) * width / 2.0),
        (int)((v.y + 1.0) * height / 2.0),
        (int)((v.z + 1.0) * 255.0 / 2.0)
    };
    return projected;
}

/**
 * @brief Apply the tutorial's perspective division to a 3D vertex.
 * @param[in] v Vertex after model rotation, before viewport mapping.
 * @param[in] cameraDistance Positive camera position on the Z axis; use 3 for the tutorial.
 * @pre v.z must be less than cameraDistance; vertices on or behind the camera are invalid.
 * @return v / (1 - v.z / cameraDistance), including the transformed z component.
 * @note This operation does not perform clipping or convert coordinates to pixels.
 */
Vec3 vec3Perspective(Vec3 v, double cameraDistance)
{
    const double denominator = 1.0 - v.z / cameraDistance;
    return vec3Scale(v, 1.0 / denominator);
}

/**
 * @brief Rotate a vector about the Y axis through the origin.
 * @param[in] v Vector to rotate without modifying it.
 * @param[in] angleDegrees Rotation angle in degrees, following the right-hand rule.
 * @return The rotated vector, with its length preserved.
 * @note Degrees are converted to radians internally; use 30 for the tutorial rotation.
 */
Vec3 vec3RotateY(Vec3 v, double angleDegrees)
{
    const double angleRadians = angleDegrees * (acos(-1.0) / 180.0);
    const double cosine = cos(angleRadians);
    const double sine = sin(angleRadians);
    const Mat3 rotation = {{{cosine, 0.0, sine},
                            {0.0, 1.0, 0.0},
                            {-sine, 0.0, cosine}}};
    return mat3MultiplyVec3(rotation, v);
}

/**
 * @brief Add two vectors component by component without modifying either input.
 * @param[in] a First vector.
 * @param[in] b Second vector.
 * @return A vector with components (a.x + b.x, a.y + b.y).
 */
Vec2 vec2Add(Vec2 a, Vec2 b)
{
    Vec2 res = {0, 0};
    res.x = a.x + b.x;
    res.y = a.y + b.y;
    return res;
}

/**
 * @brief Subtract the second vector from the first without modifying either input.
 * @param[in] a Vector to subtract from.
 * @param[in] b Vector to subtract.
 * @return A vector with components (a.x - b.x, a.y - b.y).
 */
Vec2 vec2Subtract(Vec2 a, Vec2 b)
{
    Vec2 res = {0, 0};
    res.x = a.x - b.x;
    res.y = a.y - b.y;
    return res;
}

/**
 * @brief Multiply each vector component by a scalar without modifying the input.
 * @param[in] v Vector to scale.
 * @param[in] scalar Multiplier applied to both components.
 * @return A vector with components (v.x * scalar, v.y * scalar).
 */
Vec2 vec2Scale(Vec2 v, double scalar)
{
    Vec2 res = {0, 0};
    res.x = v.x * scalar;
    res.y = v.y * scalar;
    return res;
}

/**
 * @brief Compute the dot product of two vectors without modifying either input.
 * @param[in] a First vector.
 * @param[in] b Second vector.
 * @return The scalar a.x * b.x + a.y * b.y.
 */
double vec2Dot(Vec2 a, Vec2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

/**
 * @brief Compute the Euclidean length of a vector without modifying it.
 * @param[in] v Vector whose length is measured.
 * @return sqrt(v.x * v.x + v.y * v.y), or zero for the zero vector.
 */
double vec2Length(Vec2 v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}

/**
 * @brief Create a unit-length vector with the same direction as the input.
 * @param[in] v Vector to normalize without modifying it.
 * @return The normalized vector, or the zero vector if the input has zero length.
 */
Vec2 vec2Normalize(Vec2 v)
{
    Vec2 res = {0, 0};
    double len = vec2Length(v);

    if(len != 0)
    {
        res.x = v.x / len;
        res.y = v.y / len;
    }
    return res;
}

/**
 * @brief Add two vectors component by component without modifying either input.
 * @param[in] a First vector.
 * @param[in] b Second vector.
 * @return A vector with components (a.x + b.x, a.y + b.y, a.z + b.z).
 */
Vec3 vec3Add(Vec3 a, Vec3 b)
{
    Vec3 res = {0, 0, 0};
    res.x = a.x + b.x;
    res.y = a.y + b.y;
    res.z = a.z + b.z;
    return res;
}

/**
 * @brief Subtract the second vector from the first without modifying either input.
 * @param[in] a Vector to subtract from.
 * @param[in] b Vector to subtract.
 * @return A vector with components (a.x - b.x, a.y - b.y, a.z - b.z).
 */
Vec3 vec3Subtract(Vec3 a, Vec3 b)
{
    Vec3 res = {0, 0, 0};
    res.x = a.x - b.x;
    res.y = a.y - b.y;
    res.z = a.z - b.z;
    return res;
}

/**
 * @brief Multiply each vector component by a scalar without modifying the input.
 * @param[in] v Vector to scale.
 * @param[in] scalar Multiplier applied to all three components.
 * @return A vector with components (v.x * scalar, v.y * scalar, v.z * scalar).
 */
Vec3 vec3Scale(Vec3 v, double scalar)
{
    Vec3 res = {0, 0, 0};
    res.x = v.x * scalar;
    res.y = v.y * scalar;
    res.z = v.z * scalar;
    return res;
}

/**
 * @brief Compute the dot product of two vectors without modifying either input.
 * @param[in] a First vector.
 * @param[in] b Second vector.
 * @return The scalar a.x * b.x + a.y * b.y + a.z * b.z.
 */
double vec3Dot(Vec3 a, Vec3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

/**
 * @brief Compute the Euclidean length of a vector without modifying it.
 * @param[in] v Vector whose length is measured.
 * @return sqrt(v.x * v.x + v.y * v.y + v.z * v.z), or zero for the zero vector.
 */
double vec3Length(Vec3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * @brief Create a unit-length vector with the same direction as the input.
 * @param[in] v Vector to normalize without modifying it.
 * @return The normalized vector, or the zero vector if the input has zero length.
 */
Vec3 vec3Normalize(Vec3 v)
{
    Vec3 res = {0, 0, 0};
    double len = vec3Length(v);

    if(len != 0)
    {
        res.x = v.x / len;
        res.y = v.y / len;
        res.z = v.z / len;
    }
    return res;
}

/**
 * @brief Compute the right-handed cross product a x b without modifying either input.
 * @param[in] a First vector.
 * @param[in] b Second vector.
 * @return A vector perpendicular to both inputs, or zero for parallel inputs.
 * @note Reversing the input order negates the result. The result is not normalized.
 */
Vec3 vec3Cross(Vec3 a, Vec3 b)
{
    Vec3 res = {0, 0, 0};
    res.x = (a.y * b.z) - (a.z * b.y);
    res.y = (a.z * b.x) - (a.x * b.z);
    res.z = (a.x * b.y) - (a.y * b.x);

    return res;
}

/**
 * @brief Add two vectors component by component without modifying either input.
 * @param[in] a First vector.
 * @param[in] b Second vector.
 * @return A vector with components (a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w).
 */
Vec4 vec4Add(Vec4 a, Vec4 b)
{
    Vec4 res = {0, 0, 0, 0};
    res.x = a.x + b.x;
    res.y = a.y + b.y;
    res.z = a.z + b.z;
    res.w = a.w + b.w;
    return res;
}

/**
 * @brief Subtract the second vector from the first without modifying either input.
 * @param[in] a Vector to subtract from.
 * @param[in] b Vector to subtract.
 * @return A vector with components (a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w).
 */
Vec4 vec4Subtract(Vec4 a, Vec4 b)
{
    Vec4 res = {0, 0, 0, 0};
    res.x = a.x - b.x;
    res.y = a.y - b.y;
    res.z = a.z - b.z;
    res.w = a.w - b.w;
    return res;
}

/**
 * @brief Multiply each vector component by a scalar without modifying the input.
 * @param[in] v Vector to scale.
 * @param[in] scalar Multiplier applied to all four components.
 * @return A vector with components (v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar).
 */
Vec4 vec4Scale(Vec4 v, double scalar)
{
    Vec4 res = {0, 0, 0, 0};
    res.x = v.x * scalar;
    res.y = v.y * scalar;
    res.z = v.z * scalar;
    res.w = v.w * scalar;
    return res;
}

/**
 * @brief Compute the dot product of two vectors without modifying either input.
 * @param[in] a First vector.
 * @param[in] b Second vector.
 * @return The scalar a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w.
 */
double vec4Dot(Vec4 a, Vec4 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

/**
 * @brief Compute the Euclidean length of a vector without modifying it.
 * @param[in] v Vector whose length is measured.
 * @return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w), or zero for the zero vector.
 */
double vec4Length(Vec4 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

/**
 * @brief Create a unit-length four-dimensional vector with the same direction.
 * @param[in] v Vector to normalize without modifying it.
 * @return The normalized vector, or the zero vector if the input has zero length.
 * @note This is a Euclidean 4D normalization, not a homogeneous divide by w.
 */
Vec4 vec4Normalize(Vec4 v)
{
    Vec4 res = {0, 0, 0, 0};
    double len = vec4Length(v);

    if(len != 0)
    {
        res.x = v.x / len;
        res.y = v.y / len;
        res.z = v.z / len;
        res.w = v.w / len;
    }
    return res;
}
