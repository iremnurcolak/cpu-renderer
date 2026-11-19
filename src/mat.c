#include "mat.h"

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
