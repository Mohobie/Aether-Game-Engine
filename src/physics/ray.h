#pragma once
#include "core/math.h"
namespace aether {
struct Ray {
    Vec3 origin;
    Vec3 direction;
    Ray() : origin(0,0,0), direction(0,0,-1) {}
    Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction) {}
    Vec3 at(float t) const { return origin + direction * t; }
};
struct RaycastHit {
    Vec3 point;
    Vec3 normal;
    float distance;
    bool hit;
};
} // namespace aether
