#include "mat.h"

/** @brief Return the 4 x 4 identity matrix. */
Mat4 mat4Identity(void)
{
    Mat4 result = {0};
    for(int i = 0; i < 4; ++i) result.data[i][i] = 1.0;
    return result;
}

/**
 * @brief Compose two 4 x 4 matrices for column vectors.
 * @param[in] left Transformation applied second.
 * @param[in] right Transformation applied first.
 * @return The matrix product left * right.
 */
Mat4 mat4Multiply(Mat4 left, Mat4 right)
{
    Mat4 result = {0};
    for(int row = 0; row < 4; ++row)
        for(int column = 0; column < 4; ++column)
            for(int k = 0; k < 4; ++k)
                result.data[row][column] += left.data[row][k] * right.data[k][column];
    return result;
}

/**
 * @brief Transform a homogeneous column vector without dividing by w.
 * @param[in] matrix Row-major transformation matrix.
 * @param[in] v Homogeneous input vector.
 * @return The four components of matrix * v.
 */
Vec4 mat4MultiplyVec4(Mat4 matrix, Vec4 v)
{
    const double input[4] = {v.x, v.y, v.z, v.w};
    double result[4] = {0};
    for(int row = 0; row < 4; ++row)
        for(int column = 0; column < 4; ++column)
            result[row] += matrix.data[row][column] * input[column];
    return (Vec4){result[0], result[1], result[2], result[3]};
}

/**
 * @brief Multiply a 3 x 3 matrix by a column vector without modifying either input.
 * @param[in] matrix Row-major matrix.
 * @param[in] v Column vector to transform.
 * @return A vector whose components are the dot products of matrix rows with v.
 */
Vec3 mat3MultiplyVec3(Mat3 matrix, Vec3 v)
{
    Vec3 result;
    result.x = matrix.data[0][0] * v.x + matrix.data[0][1] * v.y + matrix.data[0][2] * v.z;
    result.y = matrix.data[1][0] * v.x + matrix.data[1][1] * v.y + matrix.data[1][2] * v.z;
    result.z = matrix.data[2][0] * v.x + matrix.data[2][1] * v.y + matrix.data[2][2] * v.z;
    return result;
}
