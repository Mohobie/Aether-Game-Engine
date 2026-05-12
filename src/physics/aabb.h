#pragma once
#include "core/math.h"
namespace vge {
struct AABB {
    Vec3 min;
    Vec3 max;
    AABB() : min(0,0,0), max(0,0,0) {}
    AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}
    bool intersects(const AABB& other) const;
    bool contains(const Vec3& point) const;
    Vec3 getCenter() const;
    Vec3 getSize() const;
    void expand(const Vec3& point);
};
} // namespace vge
