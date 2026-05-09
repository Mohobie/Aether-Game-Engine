#include "collision.h"
#include "math/vec3.h"

namespace vge {
bool AABB::Intersects(const AABB& other) const {
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
}

bool Physics::RayCast(const Vec3& origin, const Vec3& direction, float maxDist, Vec3& hit) {
    // Stub: simple forward ray march
    Vec3 pos = origin;
    Vec3 dir = direction.normalize();
    for (float t = 0; t < maxDist; t += 0.1f) {
        pos = origin + dir * t;
        // Would check against world blocks here
    }
    hit = pos;
    return false; // No hit in stub
}
}