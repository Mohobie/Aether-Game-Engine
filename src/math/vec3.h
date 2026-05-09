#pragma once
#include <cmath>

namespace vge {
    struct Vec3 {
        float x, y, z;
        
        Vec3();
        Vec3(float x, float y, float z);
        
        Vec3 operator+(const Vec3& other) const;
        Vec3 operator-(const Vec3& other) const;
        Vec3 operator*(float scalar) const;
        
        float length() const;
        Vec3 normalize() const;
        float dot(const Vec3& other) const;
        Vec3 cross(const Vec3& other) const;
    };
}