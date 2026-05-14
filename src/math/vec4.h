#pragma once
#include "vec3.h"

namespace vge {

struct Vec4 {
    float x, y, z, w;
    
    Vec4(float x = 0, float y = 0, float z = 0, float w = 0) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3& v, float w = 1.0f) : x(v.x), y(v.y), z(v.z), w(w) {}
    
    Vec4 operator+(const Vec4& o) const { return Vec4(x + o.x, y + o.y, z + o.z, w + o.w); }
    Vec4 operator-(const Vec4& o) const { return Vec4(x - o.x, y - o.y, z - o.z, w - o.w); }
    Vec4 operator*(float s) const { return Vec4(x * s, y * s, z * s, w * s); }
    Vec4 operator/(float s) const { return Vec4(x / s, y / s, z / s, w / s); }
    
    bool operator==(const Vec4& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
    bool operator!=(const Vec4& o) const { return !(*this == o); }
    
    float Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    float LengthSquared() const { return x * x + y * y + z * z + w * w; }
    
    Vec4 Normalized() const {
        float len = Length();
        if (len > 0) return *this / len;
        return Vec4(0, 0, 0, 0);
    }
    
    float Dot(const Vec4& o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }
    
    Vec3 ToVec3() const { return Vec3(x, y, z); }
    
    static Vec4 Zero() { return Vec4(0, 0, 0, 0); }
    static Vec4 One() { return Vec4(1, 1, 1, 1); }
};

} // namespace vge
