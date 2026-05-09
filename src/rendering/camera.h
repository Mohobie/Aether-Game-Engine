#pragma once
#include "math/vec3.h"
#include "math/mat4.h"

namespace VoxelEngine {
    class Camera {
    public:
        Vec3 position;
        Vec3 forward;
        Vec3 up;
        
        Mat4 GetViewMatrix() const;
        Mat4 GetProjectionMatrix(float fov, float aspect, float near, float far) const;
        void Move(const Vec3& delta);
        void Rotate(float yaw, float pitch);
    };
}
