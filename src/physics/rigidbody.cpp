#include "rigidbody.h"
#include <iostream>

namespace vge {

RigidBody::RigidBody() 
    : position(0, 0, 0), velocity(0, 0, 0), acceleration(0, 0, 0),
      mass(1.0f), useGravity(true), isStatic(false) {}

RigidBody::~RigidBody() {}

void RigidBody::SetPosition(const Vec3& pos) {
    position = pos;
}

void RigidBody::SetVelocity(const Vec3& vel) {
    velocity = vel;
}

void RigidBody::ApplyForce(const Vec3& force) {
    acceleration = acceleration + (force * (1.0f / mass));
}

void RigidBody::Update(float deltaTime) {
    if (isStatic) return;
    
    if (useGravity) {
        ApplyForce(Vec3(0, -9.81f * mass, 0));
    }
    
    velocity = velocity + (acceleration * deltaTime);
    position = position + (velocity * deltaTime);
    acceleration = Vec3(0, 0, 0); // Reset acceleration
}

AABB RigidBody::GetBounds() const {
    return AABB(position, Vec3(1.0f, 1.0f, 1.0f)); // Default 1x1x1 bounds
}

} // namespace vge
