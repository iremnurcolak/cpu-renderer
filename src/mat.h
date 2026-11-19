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

/**
 * @brief Multiply a 3 x 3 matrix by a column vector without modifying either input.
 * @param[in] matrix Row-major matrix.
 * @param[in] v Column vector to transform.
 * @return A vector whose components are the dot products of matrix rows with v.
 */
Vec3 mat3MultiplyVec3(Mat3 matrix, Vec3 v);

#endif /* MAT_H */
