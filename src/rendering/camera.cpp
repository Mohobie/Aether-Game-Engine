#include "camera.h"
#include <cmath>

namespace vge {

Camera::Camera() 
    : position(0.0f, 0.0f, 5.0f),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      fov(45.0f),
      aspectRatio(16.0f / 9.0f),
      nearPlane(0.1f),
      farPlane(1000.0f),
      yaw(-90.0f),
      pitch(0.0f) {}

Mat4 Camera::GetViewMatrix() const {
    return Mat4::LookAt(position, target, up);
}

Mat4 Camera::GetProjectionMatrix() const {
    return Mat4::Perspective(fov * 3.14159f / 180.0f, aspectRatio, nearPlane, farPlane);
}

void Camera::SetPosition(const Vec3& pos) {
    position = pos;
    UpdateTarget();
}

void Camera::Rotate(float deltaYaw, float deltaPitch) {
    yaw += deltaYaw;
    pitch += deltaPitch;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    UpdateTarget();
}

void Camera::UpdateTarget() {
    float yawRad = yaw * 3.14159f / 180.0f;
    float pitchRad = pitch * 3.14159f / 180.0f;
    
    Vec3 front;
    front.x = std::cos(yawRad) * std::cos(pitchRad);
    front.y = std::sin(pitchRad);
    front.z = std::sin(yawRad) * std::cos(pitchRad);
    
    target = position + front.normalize();
}

} // namespace vge
