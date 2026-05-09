#pragma once
#include "types.h"
#include <cmath>
namespace aether {
struct Vec2 {
    float x, y;
    Vec2(float x=0, float y=0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return Vec2(x+o.x, y+o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x-o.x, y-o.y); }
    Vec2 operator*(float s) const { return Vec2(x*s, y*s); }
    float length() const { return std::sqrt(x*x + y*y); }
    float dot(const Vec2& o) const { return x*o.x + y*o.y; }
};
struct Vec3 {
    float x, y, z;
    Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return Vec3(x+o.x, y+o.y, z+o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x-o.x, y-o.y, z-o.z); }
    Vec3 operator*(float s) const { return Vec3(x*s, y*s, z*s); }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    float dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
};
struct Mat4 {
    float m[16];
    static Mat4 identity();
    static Mat4 translate(const Vec3& t);
    static Mat4 scale(const Vec3& s);
    static Mat4 perspective(float fov, float aspect, float near, float far);
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
    Mat4 operator*(const Mat4& o) const;
};
} // namespace aether
