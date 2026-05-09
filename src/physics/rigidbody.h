#pragma once
#include "math/vec3.h"

namespace VoxelEngine {
    struct RigidBody {
        Vec3 position;
        Vec3 velocity;
        Vec3 acceleration;
        float mass;
        bool isStatic;
        
        void ApplyForce(const Vec3& force);
        void Update(float deltaTime);
    };
}
