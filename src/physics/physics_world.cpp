#include "physics_world.h"
#include "physics_system.h"
#include "core/logger.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace vge {

// ============================================
// PhysicsWorld Implementation
// ============================================

PhysicsWorld::PhysicsWorld(const Vec3& gravityVec)
    : gravity(gravityVec)
    , solverIterations(4)
    , sleepThreshold(0.1f)
    , cellSize(10.0f)
{
}

// Body management
Rigidbody* PhysicsWorld::CreateBody(float mass) {
    auto body = std::make_unique<Rigidbody>(mass);
    Rigidbody* ptr = body.get();
    bodies.push_back(std::move(body));
    return ptr;
}

void PhysicsWorld::DestroyBody(Rigidbody* body) {
    bodies.erase(
        std::remove_if(bodies.begin(), bodies.end(),
            [body](const std::unique_ptr<Rigidbody>& b) { return b.get() == body; }),
        bodies.end()
    );
}

// Collider management
BoxCollider* PhysicsWorld::CreateBoxCollider(const Vec3& center, const Vec3& halfExtents) {
    auto collider = std::make_unique<BoxCollider>(center, halfExtents);
    BoxCollider* ptr = collider.get();
    colliders.push_back(std::move(collider));
    return ptr;
}

SphereCollider* PhysicsWorld::CreateSphereCollider(const Vec3& center, float radius) {
    auto collider = std::make_unique<SphereCollider>(center, radius);
    SphereCollider* ptr = collider.get();
    colliders.push_back(std::move(collider));
    return ptr;
}

void PhysicsWorld::DestroyCollider(Collider* collider) {
    colliders.erase(
        std::remove_if(colliders.begin(), colliders.end(),
            [collider](const std::unique_ptr<Collider>& c) { return c.get() == collider; }),
        colliders.end()
    );
}

// Simulation
void PhysicsWorld::Step(float deltaTime) {
    // Update spatial grid for broad-phase
    UpdateSpatialGrid();
    
    // Integrate forces to get new velocities/positions
    UpdateBodies(deltaTime);
    
    // Detect collisions
    DetectCollisions();
    
    // Resolve collisions
    ResolveCollisions();
}

void PhysicsWorld::UpdateBodies(float deltaTime) {
    for (auto& body : bodies) {
        if (body->isKinematic || body->isSleeping) continue;
        
        // Apply gravity
        if (body->useGravity && body->HasFiniteMass()) {
            body->AddForce(gravity * body->mass);
        }
        
        // Integrate acceleration: a = F/m
        body->acceleration = body->forceAccumulator * body->inverseMass;
        
        // Integrate velocity: v += a * dt
        body->velocity = body->velocity + body->acceleration * deltaTime;
        
        // Apply drag
        body->velocity = body->velocity * (1.0f - body->drag * deltaTime);
        
        // Integrate position: p += v * dt
        body->position = body->position + body->velocity * deltaTime;
        
        // Update collider position
        if (body->collider) {
            body->collider->UpdatePosition(body->position);
        }
        
        // Clear forces
        body->forceAccumulator = Vec3(0, 0, 0);
        
        // Sleep check
        if (body->velocity.length() < sleepThreshold && 
            body->acceleration.length() < sleepThreshold) {
            // Could sleep here, but for now keep awake
        }
    }
}

void PhysicsWorld::DetectCollisions() {
    contacts.clear();
    
    // Broad-phase: spatial grid
    for (auto& collider : colliders) {
        if (collider->isStatic) continue;
        
        AABB bounds = collider->GetBounds();
        std::vector<Collider*> nearby = GetNearbyColliders(bounds);
        
        // Narrow-phase: exact tests
        for (Collider* other : nearby) {
            if (other == collider.get()) continue;
            
            Vec3 normal;
            float penetration;
            
            if (collider->TestCollision(other, normal, penetration)) {
                Contact contact;
                contact.colliderA = collider.get();
                contact.colliderB = other;
                contact.normal = normal;
                contact.penetration = penetration;
                contact.isTrigger = collider->isTrigger || other->isTrigger;
                contacts.push_back(contact);
            }
        }
    }
}

void PhysicsWorld::ResolveCollisions() {
    for (int i = 0; i < solverIterations; ++i) {
        for (const auto& contact : contacts) {
            if (contact.isTrigger) {
                // Trigger - just call callbacks
                if (contact.colliderA->onTriggerEnter) {
                    contact.colliderA->onTriggerEnter(contact.colliderB);
                }
                continue;
            }
            
            // Find associated bodies
            Rigidbody* bodyA = nullptr;
            Rigidbody* bodyB = nullptr;
            
            for (auto& body : bodies) {
                if (body->collider == contact.colliderA) bodyA = body.get();
                if (body->collider == contact.colliderB) bodyB = body.get();
            }
            
            if (!bodyA && !bodyB) continue;
            
            // Relative velocity
            Vec3 relativeVel = (bodyA ? bodyA->velocity : Vec3(0,0,0)) - 
                              (bodyB ? bodyB->velocity : Vec3(0,0,0));
            
            float velAlongNormal = relativeVel.dot(contact.normal);
            
            // Don't resolve if velocities are separating
            if (velAlongNormal > 0) continue;
            
            // Restitution (bounciness)
            float restitution = 0.5f;
            
            // Impulse scalar
            float j = -(1.0f + restitution) * velAlongNormal;
            float totalInverseMass = (bodyA ? bodyA->inverseMass : 0) + 
                                    (bodyB ? bodyB->inverseMass : 0);
            if (totalInverseMass <= 0) continue;
            
            j /= totalInverseMass;
            
            // Apply impulse
            Vec3 impulse = contact.normal * j;
            if (bodyA) bodyA->AddImpulse(impulse);
            if (bodyB) bodyB->AddImpulse(impulse * -1.0f);
            
            // Positional correction (prevent sinking)
            float percent = 0.2f; // Penetration percentage to correct
            float slop = 0.01f;   // Penetration allowance
            Vec3 correction = contact.normal * std::max(contact.penetration - slop, 0.0f) / totalInverseMass * percent;
            
            if (bodyA) bodyA->position = bodyA->position + correction * bodyA->inverseMass;
            if (bodyB) bodyB->position = bodyB->position - correction * bodyB->inverseMass;
        }
    }
}

// Spatial grid
void PhysicsWorld::UpdateSpatialGrid() {
    spatialGrid.clear();
    for (auto& collider : colliders) {
        InsertToGrid(collider.get());
    }
}

uint64_t PhysicsWorld::GetGridKey(int x, int y, int z) const {
    // Simple hash for 3D grid cell
    const uint64_t p1 = 73856093;
    const uint64_t p2 = 19349663;
    const uint64_t p3 = 83492791;
    return (uint64_t)(x * p1) ^ (uint64_t)(y * p2) ^ (uint64_t)(z * p3);
}

void PhysicsWorld::InsertToGrid(Collider* collider) {
    AABB bounds = collider->GetBounds();
    int minX = (int)std::floor(bounds.min.x / cellSize);
    int minY = (int)std::floor(bounds.min.y / cellSize);
    int minZ = (int)std::floor(bounds.min.z / cellSize);
    int maxX = (int)std::floor(bounds.max.x / cellSize);
    int maxY = (int)std::floor(bounds.max.y / cellSize);
    int maxZ = (int)std::floor(bounds.max.z / cellSize);
    
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                uint64_t key = GetGridKey(x, y, z);
                spatialGrid[key].push_back(collider);
            }
        }
    }
}

std::vector<Collider*> PhysicsWorld::GetNearbyColliders(const AABB& bounds) {
    std::vector<Collider*> result;
    std::unordered_set<Collider*> unique;
    
    int minX = (int)std::floor(bounds.min.x / cellSize);
    int minY = (int)std::floor(bounds.min.y / cellSize);
    int minZ = (int)std::floor(bounds.min.z / cellSize);
    int maxX = (int)std::floor(bounds.max.x / cellSize);
    int maxY = (int)std::floor(bounds.max.y / cellSize);
    int maxZ = (int)std::floor(bounds.max.z / cellSize);
    
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                uint64_t key = GetGridKey(x, y, z);
                auto it = spatialGrid.find(key);
                if (it != spatialGrid.end()) {
                    for (Collider* collider : it->second) {
                        if (unique.insert(collider).second) {
                            result.push_back(collider);
                        }
                    }
                }
            }
        }
    }
    
    return result;
}

// Queries
std::vector<Collider*> PhysicsWorld::OverlapSphere(const Vec3& center, float radius) {
    std::vector<Collider*> result;
    for (auto& collider : colliders) {
        Vec3 normal;
        float penetration;
        SphereCollider sphere(center, radius);
        if (collider->TestCollision(&sphere, normal, penetration)) {
            result.push_back(collider.get());
        }
    }
    return result;
}

std::vector<Collider*> PhysicsWorld::OverlapBox(const Vec3& center, const Vec3& halfExtents) {
    std::vector<Collider*> result;
    for (auto& collider : colliders) {
        Vec3 normal;
        float penetration;
        BoxCollider box(center, halfExtents);
        if (collider->TestCollision(&box, normal, penetration)) {
            result.push_back(collider.get());
        }
    }
    return result;
}

bool PhysicsWorld::Raycast(const Vec3& origin, const Vec3& direction, float maxDistance, Vec3& outHit, Vec3& outNormal) {
    float closestDist = maxDistance;
    bool hit = false;
    
    for (auto& collider : colliders) {
        // Simple sphere/box raycast approximation
        // TODO: Implement proper ray-AABB/ray-sphere tests
        Vec3 toCollider = collider->GetBounds().center() - origin;
        float dist = toCollider.dot(direction);
        if (dist > 0 && dist < closestDist) {
            if (collider->ContainsPoint(origin + direction * dist)) {
                closestDist = dist;
                outHit = origin + direction * dist;
                outNormal = (origin + direction * dist - toCollider).normalize();
                hit = true;
            }
        }
    }
    
    return hit;
}

void PhysicsWorld::Clear() {
    bodies.clear();
    colliders.clear();
    contacts.clear();
    spatialGrid.clear();
}

// ============================================
// Rigidbody Implementation
// ============================================

Rigidbody::Rigidbody(float m)
    : position(0, 0, 0)
    , velocity(0, 0, 0)
    , acceleration(0, 0, 0)
    , forceAccumulator(0, 0, 0)
    , mass(m)
    , inverseMass(m > 0 ? 1.0f / m : 0)
    , drag(0.01f)
    , angularDrag(0.05f)
    , useGravity(true)
    , isKinematic(false)
    , isSleeping(false)
    , isGrounded(false)
    , groundCheckDistance(0.1f)
    , collider(nullptr)
{
}

void Rigidbody::AddForce(const Vec3& force) {
    if (isKinematic) return;
    forceAccumulator = forceAccumulator + force;
}

void Rigidbody::AddImpulse(const Vec3& impulse) {
    if (isKinematic || inverseMass <= 0) return;
    velocity = velocity + impulse * inverseMass;
    isSleeping = false;
}

void Rigidbody::AddForceAtPosition(const Vec3& force, const Vec3& position) {
    // For now, just apply as central force
    // TODO: Add torque calculation
    AddForce(force);
}

void Rigidbody::SetPosition(const Vec3& pos) {
    position = pos;
    if (collider) {
        collider->UpdatePosition(pos);
    }
}

void Rigidbody::SetMass(float m) {
    mass = m;
    inverseMass = m > 0 ? 1.0f / m : 0;
}

void Rigidbody::WakeUp() {
    isSleeping = false;
}

void Rigidbody::Sleep() {
    isSleeping = true;
    velocity = Vec3(0, 0, 0);
    acceleration = Vec3(0, 0, 0);
}

void Rigidbody::UpdateGroundState(const std::vector<Collider*>& colliders) {
    isGrounded = false;
    Vec3 checkPos = position - Vec3(0, groundCheckDistance, 0);
    
    for (auto* collider : colliders) {
        if (collider->ContainsPoint(checkPos)) {
            isGrounded = true;
            break;
        }
    }
}

} // namespace vge
