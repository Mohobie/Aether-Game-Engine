#pragma once
#include "math/vec3.h"
#include "math/mat4.h"

namespace vge {
    class Camera {
    private:
        Vec3 position;
        Vec3 rotation;
        float fov;
        float aspectRatio;
        float nearPlane;
        float farPlane;
        
    public:
        Camera();
        ~Camera();
        
        void SetPosition(const Vec3& pos);
        void Move(const Vec3& delta);
        
        void SetRotation(float yaw, float pitch, float roll);
        void Rotate(float deltaYaw, float deltaPitch, float deltaRoll);
        
        Mat4 GetViewMatrix() const;
        Mat4 GetProjectionMatrix() const;
        
        Vec3 GetForward() const;
        Vec3 GetRight() const;
        Vec3 GetUp() const;
        
        Vec3 GetPosition() const { return position; }
        Vec3 GetRotation() const { return rotation; }
    };
}