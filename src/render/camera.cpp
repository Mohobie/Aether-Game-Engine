#include "camera.h"
namespace vge {
Mat4 Camera::getViewMatrix() const {
    return Mat4::lookAt(position, position + forward, up);
}
Mat4 Camera::getProjectionMatrix() const {
    return Mat4::perspective(fov, aspect, nearPlane, farPlane);
}
} // namespace vge
