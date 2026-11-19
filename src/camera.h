#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include "mat.h"

/**
 * @brief Build the tutorial's camera frame with its origin at center.
 * @param[out] modelView Destination matrix, written only on success.
 * @param[in] eye Camera position in world coordinates.
 * @param[in] center Target point and origin of the camera frame.
 * @param[in] up Approximate upward direction in world coordinates.
 * @return false for nonfinite or degenerate camera parameters, or a NULL destination.
 * @note Translation is by -center, not -eye, matching the lesson's perspective model.
 */
bool cameraLookAt(Mat4 *modelView, Vec3 eye, Vec3 center, Vec3 up);

/**
 * @brief Build a perspective matrix whose output w is 1 - z / focalDistance.
 * @param[in] focalDistance Positive finite distance from eye to center.
 * @return A homogeneous perspective matrix; divide its output by w before viewport mapping.
 */
Mat4 cameraPerspective(double focalDistance);

/**
 * @brief Map normalized x/y to a rectangular viewport while preserving depth.
 * @param[in] x Left edge in framebuffer coordinates.
 * @param[in] y Bottom edge with the framebuffer's upward y axis.
 * @param[in] width Positive viewport width.
 * @param[in] height Positive viewport height.
 * @return A homogeneous viewport matrix that leaves z and w unchanged.
 */
Mat4 cameraViewport(int x, int y, int width, int height);

/**
 * @brief Divide clip coordinates by w and map them to the viewport.
 * @param[in] clip Vertex after perspective * modelView, before division by w.
 * @param[in] viewport Viewport matrix preserving depth.
 * @param[out] screen Floating-point screen x/y and unscaled depth, written on success.
 * @return false for nonfinite coordinates, nonpositive w, or a NULL destination.
 * @note Near-plane clipping is not performed; invalid vertices must be skipped by the caller.
 */
bool cameraClipToScreen(Vec4 clip, Mat4 viewport, Vec3 *screen);

#endif /* CAMERA_H */
