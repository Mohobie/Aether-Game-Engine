#pragma once
#include "aabb.h"
#include "math/vec3.h"

namespace vge {

class Physics {
public:
    static bool RayCast(const Vec3& origin, const Vec3& direction, float maxDistance, Vec3& hitPoint);
    
    // Ray-AABB intersection for specific box
    static bool RayCastAABB(const Vec3& origin, const Vec3& direction, const AABB& box, float maxDist, Vec3& hit, float& outDist);
};
}