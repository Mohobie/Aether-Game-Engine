#pragma once
#include "vec3.h"

namespace vge {
    struct Mat4 {
        float data[16];
        
        Mat4();
        
        static Mat4 Identity();
        static Mat4 Perspective(float fov, float aspect, float near, float far);
        static Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
        static Mat4 Translate(const Vec3& translation);
        static Mat4 Rotate(float angle, const Vec3& axis);
        static Mat4 Scale(const Vec3& scale);
        
        Mat4 Multiply(const Mat4& other) const;
        Vec3 TransformPoint(const Vec3& point) const;
        
        // Matrix operations
        Mat4 Transpose() const;
        Mat4 Inverse() const;
        
        // Operators
        Mat4 operator*(const Mat4& other) const;
        Vec3 operator*(const Vec3& vec) const;
    };
}