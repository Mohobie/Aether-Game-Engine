#include "rendering/camera.h"
#include "math/mat4.h"
#include <cmath>

namespace vge {

Camera::Camera() 
    : position(0, 0, 0), rotation(0, 0, 0),
      fov(70.0f), aspectRatio(16.0f/9.0f), nearPlane(0.1f), farPlane(1000.0f) {}

Camera::~Camera() {}

void Camera::SetPosition(const Vec3& pos) {
    position = pos;
}

void Camera::Move(const Vec3& delta) {
    position = position + delta;
}

void Camera::SetRotation(float yaw, float pitch, float roll) {
    rotation = Vec3(yaw, pitch, roll);
}

void Camera::Rotate(float deltaYaw, float deltaPitch, float deltaRoll) {
    rotation = rotation + Vec3(deltaYaw, deltaPitch, deltaRoll);
}

Mat4 Camera::GetViewMatrix() const {
    // Use LookAt for proper view matrix
    Vec3 forward = GetForward();
    Vec3 center = position + forward;
    Vec3 up = GetUp();
    
    return Mat4::LookAt(position, center, up);
}

Mat4 Camera::GetProjectionMatrix() const {
    return Mat4::Perspective(fov * 3.14159f / 180.0f, aspectRatio, nearPlane, farPlane);
}

Vec3 Camera::GetForward() const {
    float yaw = rotation.x * 3.14159f / 180.0f;
    float pitch = rotation.y * 3.14159f / 180.0f;
    
    return Vec3(
        std::cos(pitch) * std::sin(yaw),
        std::sin(pitch),
        std::cos(pitch) * std::cos(yaw)
    );
}

Vec3 Camera::GetRight() const {
    Vec3 forward = GetForward();
    Vec3 up(0, 1, 0);
    // Cross product
    return Vec3(
        forward.y * up.z - forward.z * up.y,
        forward.z * up.x - forward.x * up.z,
        forward.x * up.y - forward.y * up.x
    );
}

Vec3 Camera::GetUp() const {
    return Vec3(0, 1, 0);
}

} // namespace vge