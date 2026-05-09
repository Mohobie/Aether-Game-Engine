#pragma once
#include "math/vec3.h"

namespace vge {
struct AABB {
    Vec3 min, max;
    bool Intersects(const AABB& other) const;
};

class Physics {
public:
    static bool RayCast(const Vec3& origin, const Vec3& direction, float maxDistance, Vec3& hitPoint);
};
}