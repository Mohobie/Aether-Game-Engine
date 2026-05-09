#include "rendering/camera.h"
#include "math/mat4.h"

// Stub implementation
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
    // Would create look-at matrix
    return Mat4::Identity();
}

Mat4 Camera::GetProjectionMatrix() const {
    // Would create perspective matrix
    return Mat4::Identity();
}

Vec3 Camera::GetForward() const {
    return Vec3(0, 0, -1);
}

Vec3 Camera::GetRight() const {
    return Vec3(1, 0, 0);
}

Vec3 Camera::GetUp() const {
    return Vec3(0, 1, 0);
}

} // namespace vge