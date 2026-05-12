#pragma once
#include "core/math.h"
namespace vge {
class Camera {
public:
    Vec3 position = Vec3(0, 10, 0);
    Vec3 forward = Vec3(0, 0, -1);
    Vec3 up = Vec3(0, 1, 0);
    float fov = 1.047f;
    float aspect = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix() const;
};
} // namespace vge
