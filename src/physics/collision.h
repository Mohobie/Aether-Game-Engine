#pragma once
#include "math/vec3.h"

namespace vge {
struct AABB {
    Vec3 min, max;
    
    AABB() : min(0, 0, 0), max(0, 0, 0) {}
    AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}
    
    bool Intersects(const AABB& other) const;
};

class Physics {
public:
    static bool RayCast(const Vec3& origin, const Vec3& direction, float maxDistance, Vec3& hitPoint);
    
    // Ray-AABB intersection for specific box
    static bool RayCastAABB(const Vec3& origin, const Vec3& direction, const AABB& box, float maxDist, Vec3& hit, float& outDist);
};
}