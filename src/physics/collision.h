#pragma once

#include "math/vec3.h"

namespace VoxelEngine {
    struct AABB {
        Vec3 min;
        Vec3 max;
        bool Intersects(const AABB& other);
    };
    
    class PhysicsWorld {
    public:
        void Update(float deltaTime);
        bool Raycast(const Vec3& origin, const Vec3& direction, float maxDistance);
    };
}
