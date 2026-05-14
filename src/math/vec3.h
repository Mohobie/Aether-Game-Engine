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
        Vec3 operator/(float scalar) const;
        Vec3 operator*(const Vec3& other) const;
        
        Vec3& operator+=(const Vec3& other);
        Vec3& operator-=(const Vec3& other);
        Vec3& operator*=(float scalar);
        Vec3& operator/=(float scalar);
        
        float length() const;
        float lengthSquared() const;
        Vec3 normalize() const;
        float dot(const Vec3& other) const;
        Vec3 cross(const Vec3& other) const;
        
        static Vec3 Zero() { return Vec3(0, 0, 0); }
        static Vec3 One() { return Vec3(1, 1, 1); }
        static Vec3 Up() { return Vec3(0, 1, 0); }
        static Vec3 Right() { return Vec3(1, 0, 0); }
        static Vec3 Forward() { return Vec3(0, 0, -1); }
    };
}
