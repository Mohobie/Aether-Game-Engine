#pragma once
#include <cmath>

namespace vge {

struct Vec2 {
    float x, y;
    
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    
    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
    
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return !(*this == o); }
    
    float Length() const { return std::sqrt(x * x + y * y); }
    float LengthSquared() const { return x * x + y * y; }
    
    Vec2 Normalized() const {
        float len = Length();
        if (len > 0) return *this / len;
        return Vec2(0, 0);
    }
    
    float Dot(const Vec2& o) const { return x * o.x + y * o.y; }
    
    static Vec2 Zero() { return Vec2(0, 0); }
    static Vec2 One() { return Vec2(1, 1); }
};

} // namespace vge
