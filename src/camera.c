#include "camera.h"

#include <math.h>

/**
 * @brief Build the tutorial's camera frame with its origin at center.
 * @param[out] modelView Destination matrix, written only on success.
 * @param[in] eye Camera position in world coordinates.
 * @param[in] center Target point and origin of the camera frame.
 * @param[in] up Approximate upward direction in world coordinates.
 * @return false for nonfinite or degenerate camera parameters, or a NULL destination.
 * @note Translation is by -center, not -eye, matching the lesson's perspective model.
 */
bool cameraLookAt(Mat4 *modelView, Vec3 eye, Vec3 center, Vec3 up)
{
    const Vec3 offset = vec3Subtract(eye, center);
    const double distance = vec3Length(offset);
    const double upLength = vec3Length(up);
    if(!modelView || !isfinite(distance) || distance <= 0.0
        || !isfinite(upLength) || upLength <= 0.0) return false;

    const Vec3 backward = vec3Scale(offset, 1.0 / distance);
    const Vec3 rightCross = vec3Cross(vec3Scale(up, 1.0 / upLength), backward);
    const double rightLength = vec3Length(rightCross);
    if(!isfinite(rightLength) || rightLength <= 1e-12) return false;
    const Vec3 right = vec3Scale(rightCross, 1.0 / rightLength);
    const Vec3 vertical = vec3Normalize(vec3Cross(backward, right));
    const Vec3 axes[3] = {right, vertical, backward};
    Mat4 result = mat4Identity();
    for(int row = 0; row < 3; ++row)
    {
        result.data[row][0] = axes[row].x;
        result.data[row][1] = axes[row].y;
        result.data[row][2] = axes[row].z;
        result.data[row][3] = -vec3Dot(axes[row], center);
        if(!isfinite(result.data[row][3])) return false;
    }
    *modelView = result;
    return true;
}

/**
 * @brief Build a perspective matrix whose output w is 1 - z / focalDistance.
 * @param[in] focalDistance Positive finite distance from eye to center.
 * @return A homogeneous perspective matrix; divide its output by w before viewport mapping.
 */
Mat4 cameraPerspective(double focalDistance)
{
    Mat4 result = mat4Identity();
    result.data[3][2] = -1.0 / focalDistance;
    return result;
}

/**
 * @brief Map normalized x/y to a rectangular viewport while preserving depth.
 * @param[in] x Left edge in framebuffer coordinates.
 * @param[in] y Bottom edge with the framebuffer's upward y axis.
 * @param[in] width Positive viewport width.
 * @param[in] height Positive viewport height.
 * @return A homogeneous viewport matrix that leaves z and w unchanged.
 */
Mat4 cameraViewport(int x, int y, int width, int height)
{
    Mat4 result = mat4Identity();
    result.data[0][0] = width * 0.5;
    result.data[1][1] = height * 0.5;
    result.data[0][3] = x + width * 0.5;
    result.data[1][3] = y + height * 0.5;
    return result;
}

/**
 * @brief Divide clip coordinates by w and map them to the viewport.
 * @param[in] clip Vertex after perspective * modelView, before division by w.
 * @param[in] viewport Viewport matrix preserving depth.
 * @param[out] screen Floating-point screen x/y and unscaled depth, written on success.
 * @return false for nonfinite coordinates, nonpositive w, or a NULL destination.
 * @note Near-plane clipping is not performed; invalid vertices must be skipped by the caller.
 */
bool cameraClipToScreen(Vec4 clip, Mat4 viewport, Vec3 *screen)
{
    if(!screen || !isfinite(clip.x) || !isfinite(clip.y)
        || !isfinite(clip.z) || !isfinite(clip.w) || clip.w <= 0.0) return false;
    const Vec4 ndc = {clip.x / clip.w, clip.y / clip.w, clip.z / clip.w, 1.0};
    const Vec4 mapped = mat4MultiplyVec4(viewport, ndc);
    if(!isfinite(mapped.x) || !isfinite(mapped.y) || !isfinite(mapped.z)) return false;
    *screen = (Vec3){mapped.x, mapped.y, mapped.z};
    return true;
}
