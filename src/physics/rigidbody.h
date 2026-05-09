#pragma once
#include "math/vec3.h"
#include "collision.h"

namespace vge {
    class RigidBody {
    private:
        Vec3 position;
        Vec3 velocity;
        Vec3 acceleration;
        float mass;
        bool useGravity;
        bool isStatic;
        
    public:
        RigidBody();
        ~RigidBody();
        
        void SetPosition(const Vec3& pos);
        void SetVelocity(const Vec3& vel);
        void ApplyForce(const Vec3& force);
        void Update(float deltaTime);
        
        AABB GetBounds() const;
        
        Vec3 GetPosition() const { return position; }
        Vec3 GetVelocity() const { return velocity; }
        float GetMass() const { return mass; }
        bool IsStatic() const { return isStatic; }
        void SetStatic(bool static_) { isStatic = static_; }
    };
}
