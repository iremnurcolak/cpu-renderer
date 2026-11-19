#ifndef MAT_H
#define MAT_H

#include "vec.h"

/**
 * @brief A 3 x 3 matrix with double-precision elements stored in row-major order.
 * @note Access elements with data[row][column], using indices from 0 to 2.
 */
typedef struct
{
    double data[3][3];
} Mat3;

/** @brief A row-major 4 x 4 matrix for homogeneous coordinate transformations. */
typedef struct
{
    double data[4][4];
} Mat4;

/** @brief Return the 4 x 4 identity matrix. */
Mat4 mat4Identity(void);

/**
 * @brief Compose two 4 x 4 matrices for column vectors.
 * @param[in] left Transformation applied second.
 * @param[in] right Transformation applied first.
 * @return The matrix product left * right.
 */
Mat4 mat4Multiply(Mat4 left, Mat4 right);

/**
 * @brief Transform a homogeneous column vector without dividing by w.
 * @param[in] matrix Row-major transformation matrix.
 * @param[in] v Homogeneous input vector.
 * @return The four components of matrix * v.
 */
Vec4 mat4MultiplyVec4(Mat4 matrix, Vec4 v);

/**
 * @brief Multiply a 3 x 3 matrix by a column vector without modifying either input.
 * @param[in] matrix Row-major matrix.
 * @param[in] v Column vector to transform.
 * @return A vector whose components are the dot products of matrix rows with v.
 */
Vec3 mat3MultiplyVec3(Mat3 matrix, Vec3 v);

#endif /* MAT_H */
