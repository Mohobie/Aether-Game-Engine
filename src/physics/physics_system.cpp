#include "physics_system.h"
#include "collision.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <unordered_set>

namespace vge {

// ============================================
// Box Collider Implementation
// ============================================

AABB BoxCollider::GetBounds() const {
    return AABB(center - halfExtents, center + halfExtents);
}

bool BoxCollider::TestCollision(const Collider* other, Vec3& outNormal, float& outPenetration) const {
    switch (other->type) {
        case CollisionShapeType::Box:
            return TestBoxCollision(static_cast<const BoxCollider*>(other), outNormal, outPenetration);
        case CollisionShapeType::Sphere:
            return TestSphereCollision(static_cast<const SphereCollider*>(other), outNormal, outPenetration);
        default:
            return false;
    }
}

bool BoxCollider::ContainsPoint(const Vec3& point) const {
    Vec3 local = point - center;
    return std::abs(local.x) <= halfExtents.x &&
           std::abs(local.y) <= halfExtents.y &&
           std::abs(local.z) <= halfExtents.z;
}

bool BoxCollider::TestBoxCollision(const BoxCollider* other, Vec3& outNormal, float& outPenetration) const {
    // SAT (Separating Axis Theorem) for AABB
    Vec3 minA = center - halfExtents;
    Vec3 maxA = center + halfExtents;
    Vec3 minB = other->center - other->halfExtents;
    Vec3 maxB = other->center + other->halfExtents;
    
    // Check for separation on each axis
    float overlapX = std::min(maxA.x, maxB.x) - std::max(minA.x, minB.x);
    float overlapY = std::min(maxA.y, maxB.y) - std::max(minA.y, minB.y);
    float overlapZ = std::min(maxA.z, maxB.z) - std::max(minA.z, minB.z);
    
    if (overlapX <= 0 || overlapY <= 0 || overlapZ <= 0) {
        return false; // Separated
    }
    
    // Find minimum overlap axis
    if (overlapX < overlapY && overlapX < overlapZ) {
        outPenetration = overlapX;
        outNormal = Vec3((center.x < other->center.x) ? -1.0f : 1.0f, 0, 0);
    } else if (overlapY < overlapZ) {
        outPenetration = overlapY;
        outNormal = Vec3(0, (center.y < other->center.y) ? -1.0f : 1.0f, 0);
    } else {
        outPenetration = overlapZ;
        outNormal = Vec3(0, 0, (center.z < other->center.z) ? -1.0f : 1.0f);
    }
    
    return true;
}

bool BoxCollider::TestSphereCollision(const SphereCollider* other, Vec3& outNormal, float& outPenetration) const {
    // Find closest point on box to sphere center
    Vec3 closest = center;
    closest.x = std::max(center.x - halfExtents.x, std::min(other->center.x, center.x + halfExtents.x));
    closest.y = std::max(center.y - halfExtents.y, std::min(other->center.y, center.y + halfExtents.y));
    closest.z = std::max(center.z - halfExtents.z, std::min(other->center.z, center.z + halfExtents.z));
    
    Vec3 diff = other->center - closest;
    float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
    float radiusSq = other->radius * other->radius;
    
    if (distSq > radiusSq) {
        return false;
    }
    
    float dist = std::sqrt(distSq);
    if (dist < 0.0001f) {
        // Sphere center is inside box
        outNormal = Vec3(0, 1, 0);
        outPenetration = other->radius;
    } else {
        outNormal = diff.normalize();
        outPenetration = other->radius - dist;
    }
    
    return true;
}

// ============================================
// Sphere Collider Implementation
// ============================================

AABB SphereCollider::GetBounds() const {
    Vec3 r(radius, radius, radius);
    return AABB(center - r, center + r);
}

bool SphereCollider::TestCollision(const Collider* other, Vec3& outNormal, float& outPenetration) const {
    switch (other->type) {
        case CollisionShapeType::Sphere:
            return TestSphereCollision(static_cast<const SphereCollider*>(other), outNormal, outPenetration);
        case CollisionShapeType::Box:
            return TestBoxCollision(static_cast<const BoxCollider*>(other), outNormal, outPenetration);
        default:
            return false;
    }
}

bool SphereCollider::ContainsPoint(const Vec3& point) const {
    Vec3 diff = point - center;
    float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
    return distSq <= radius * radius;
}

bool SphereCollider::TestSphereCollision(const SphereCollider* other, Vec3& outNormal, float& outPenetration) const {
    Vec3 diff = other->center - center;
    float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
    float combinedRadius = radius + other->radius;
    
    if (distSq > combinedRadius * combinedRadius) {
        return false;
    }
    
    float dist = std::sqrt(distSq);
    if (dist < 0.0001f) {
        outNormal = Vec3(0, 1, 0);
        outPenetration = combinedRadius;
    } else {
        outNormal = diff.normalize();
        outPenetration = combinedRadius - dist;
    }
    
    return true;
}

bool SphereCollider::TestBoxCollision(const BoxCollider* other, Vec3& outNormal, float& outPenetration) const {
    // Delegate to box's sphere test with reversed normal
    bool result = other->TestSphereCollision(this, outNormal, outPenetration);
    if (result) {
        outNormal = Vec3(-outNormal.x, -outNormal.y, -outNormal.z);
    }
    return result;
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
    if (isKinematic || !HasFiniteMass()) return;
    forceAccumulator = forceAccumulator + force;
    WakeUp();
}

void Rigidbody::AddImpulse(const Vec3& impulse) {
    if (isKinematic || !HasFiniteMass()) return;
    velocity = velocity + impulse * inverseMass;
    WakeUp();
}

void Rigidbody::AddForceAtPosition(const Vec3& force, const Vec3& position) {
    // For now, just apply as regular force (torque would require rotation)
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

void Rigidbody::UpdateGroundState(const std::vector<Collider*>& colliders) {
    isGrounded = false;
    
    if (!collider) return;
    
    // Cast a ray downward from the bottom of the collider
    Vec3 rayOrigin = position;
    float rayLength = groundCheckDistance;
    
    if (collider->type == CollisionShapeType::Box) {
        BoxCollider* box = static_cast<BoxCollider*>(collider);
        rayOrigin.y = position.y - box->halfExtents.y;
        rayLength += box->halfExtents.y;
    }
    
    // Simple ground check: look for colliders below
    for (auto* other : colliders) {
        if (other == collider) continue;
        if (other->isTrigger) continue;
        
        // Check if this collider is below us
        AABB bounds = other->GetBounds();
        if (bounds.max.y < position.y && 
            bounds.max.x > position.x - 0.5f && bounds.min.x < position.x + 0.5f &&
            bounds.max.z > position.z - 0.5f && bounds.min.z < position.z + 0.5f) {
            float dist = position.y - bounds.max.y;
            if (dist < groundCheckDistance) {
                isGrounded = true;
                return;
            }
        }
    }
}

void Rigidbody::WakeUp() {
    isSleeping = false;
}

void Rigidbody::Sleep() {
    if (velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z < 0.001f) {
        isSleeping = true;
    }
}

// ============================================
// Physics World Implementation
// ============================================

PhysicsWorld::PhysicsWorld(const Vec3& g) 
    : gravity(g)
    , solverIterations(4)
    , sleepThreshold(0.01f)
    , cellSize(16.0f)
{
}

Rigidbody* PhysicsWorld::CreateBody(float mass) {
    auto body = std::make_unique<Rigidbody>(mass);
    Rigidbody* ptr = body.get();
    bodies.push_back(std::move(body));
    return ptr;
}

void PhysicsWorld::DestroyBody(Rigidbody* body) {
    auto it = std::find_if(bodies.begin(), bodies.end(),
        [body](const std::unique_ptr<Rigidbody>& b) { return b.get() == body; });
    if (it != bodies.end()) {
        bodies.erase(it);
    }
}

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
    auto it = std::find_if(colliders.begin(), colliders.end(),
        [collider](const std::unique_ptr<Collider>& c) { return c.get() == collider; });
    if (it != colliders.end()) {
        colliders.erase(it);
    }
}

void PhysicsWorld::Step(float deltaTime) {
    // Sub-step for stability
    const int subSteps = 4;
    float subDelta = deltaTime / subSteps;
    
    for (int i = 0; i < subSteps; ++i) {
        UpdateBodies(subDelta);
        DetectCollisions();
        ResolveCollisions();
    }
    
    // Update spatial grid for next frame
    UpdateSpatialGrid();
}

void PhysicsWorld::UpdateBodies(float deltaTime) {
    for (auto& body : bodies) {
        if (body->isKinematic || body->isSleeping) continue;
        
        // Apply gravity
        if (body->useGravity && !body->isGrounded) {
            body->forceAccumulator = body->forceAccumulator + gravity * body->mass;
        }
        
        // Calculate acceleration: a = F / m
        body->acceleration = body->forceAccumulator * body->inverseMass;
        
        // Integrate velocity: v = v + a * dt
        body->velocity = body->velocity + body->acceleration * deltaTime;
        
        // Apply drag
        body->velocity = body->velocity * (1.0f - body->drag);
        
        // Integrate position: p = p + v * dt
        Vec3 newPos = body->position + body->velocity * deltaTime;
        body->SetPosition(newPos);
        
        // Clear forces
        body->forceAccumulator = Vec3(0, 0, 0);
        
        // Sleep check
        float speedSq = body->velocity.x * body->velocity.x + 
                       body->velocity.y * body->velocity.y + 
                       body->velocity.z * body->velocity.z;
        if (speedSq < sleepThreshold * sleepThreshold) {
            body->Sleep();
        }
    }
}

void PhysicsWorld::DetectCollisions() {
    // Get all active colliders
    std::vector<Collider*> activeColliders;
    for (auto& col : colliders) {
        if (!col->isStatic) {
            activeColliders.push_back(col.get());
        }
    }
    
    // Test all pairs
    for (size_t i = 0; i < activeColliders.size(); ++i) {
        for (size_t j = i + 1; j < colliders.size(); ++j) {
            Collider* a = activeColliders[i];
            Collider* b = colliders[j].get();
            
            if (a == b) continue;
            
            Vec3 normal;
            float penetration;
            
            if (a->TestCollision(b, normal, penetration)) {
                // Handle collision
                if (a->isTrigger || b->isTrigger) {
                    // Trigger event
                    if (a->onTriggerEnter) a->onTriggerEnter(b);
                    if (b->onTriggerEnter) b->onTriggerEnter(a);
                } else {
                    // Physical collision
                    if (a->onCollisionEnter) a->onCollisionEnter(b);
                    if (b->onCollisionEnter) b->onCollisionEnter(a);
                    
                    // Find associated bodies and resolve
                    Rigidbody* bodyA = nullptr;
                    Rigidbody* bodyB = nullptr;
                    
                    for (auto& body : bodies) {
                        if (body->collider == a) bodyA = body.get();
                        if (body->collider == b) bodyB = body.get();
                    }
                    
                    if (bodyA && bodyB) {
                        // Separate bodies
                        float totalInverseMass = bodyA->inverseMass + bodyB->inverseMass;
                        if (totalInverseMass > 0) {
                            Vec3 separation = normal * (penetration / totalInverseMass);
                            bodyA->SetPosition(bodyA->position - separation * bodyA->inverseMass);
                            bodyB->SetPosition(bodyB->position + separation * bodyB->inverseMass);
                            
                            // Apply impulse for bounce
                            Vec3 relativeVel = bodyA->velocity - bodyB->velocity;
                            float velAlongNormal = relativeVel.x * normal.x + 
                                                  relativeVel.y * normal.y + 
                                                  relativeVel.z * normal.z;
                            
                            if (velAlongNormal > 0) continue; // Moving apart
                            
                            float restitution = 0.5f; // Bounciness
                            float impulseScalar = -(1.0f + restitution) * velAlongNormal / totalInverseMass;
                            Vec3 impulse = normal * impulseScalar;
                            
                            bodyA->velocity = bodyA->velocity + impulse * bodyA->inverseMass;
                            bodyB->velocity = bodyB->velocity - impulse * bodyB->inverseMass;
                        }
                    }
                }
            }
        }
    }
}

void PhysicsWorld::ResolveCollisions() {
    // Collision resolution is done in DetectCollisions for simplicity
    // In a more complex system, this would be a separate pass
}

void PhysicsWorld::UpdateSpatialGrid() {
    spatialGrid.clear();
    
    for (auto& collider : colliders) {
        InsertToGrid(collider.get());
    }
}

uint64_t PhysicsWorld::GetGridKey(int x, int y, int z) const {
    // Pack three 21-bit signed integers into 64 bits
    uint64_t ux = static_cast<uint64_t>(x + 1048576); // Offset to make positive
    uint64_t uy = static_cast<uint64_t>(y + 1048576);
    uint64_t uz = static_cast<uint64_t>(z + 1048576);
    return (ux << 42) | (uy << 21) | uz;
}

void PhysicsWorld::InsertToGrid(Collider* collider) {
    AABB bounds = collider->GetBounds();
    
    int minX = static_cast<int>(std::floor(bounds.min.x / cellSize));
    int minY = static_cast<int>(std::floor(bounds.min.y / cellSize));
    int minZ = static_cast<int>(std::floor(bounds.min.z / cellSize));
    int maxX = static_cast<int>(std::floor(bounds.max.x / cellSize));
    int maxY = static_cast<int>(std::floor(bounds.max.y / cellSize));
    int maxZ = static_cast<int>(std::floor(bounds.max.z / cellSize));
    
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
    
    int minX = static_cast<int>(std::floor(bounds.min.x / cellSize));
    int minY = static_cast<int>(std::floor(bounds.min.y / cellSize));
    int minZ = static_cast<int>(std::floor(bounds.min.z / cellSize));
    int maxX = static_cast<int>(std::floor(bounds.max.x / cellSize));
    int maxY = static_cast<int>(std::floor(bounds.max.y / cellSize));
    int maxZ = static_cast<int>(std::floor(bounds.max.z / cellSize));
    
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                uint64_t key = GetGridKey(x, y, z);
                auto it = spatialGrid.find(key);
                if (it != spatialGrid.end()) {
                    for (Collider* col : it->second) {
                        if (unique.insert(col).second) {
                            result.push_back(col);
                        }
                    }
                }
            }
        }
    }
    
    return result;
}

std::vector<Collider*> PhysicsWorld::OverlapSphere(const Vec3& center, float radius) {
    std::vector<Collider*> result;
    Vec3 r(radius, radius, radius);
    AABB queryBounds(center - r, center + r);
    
    auto nearby = GetNearbyColliders(queryBounds);
    for (Collider* col : nearby) {
        Vec3 normal;
        float penetration;
        SphereCollider sphere(center, radius);
        if (col->TestCollision(&sphere, normal, penetration)) {
            result.push_back(col);
        }
    }
    
    return result;
}

std::vector<Collider*> PhysicsWorld::OverlapBox(const Vec3& center, const Vec3& halfExtents) {
    std::vector<Collider*> result;
    AABB queryBounds(center - halfExtents, center + halfExtents);
    
    auto nearby = GetNearbyColliders(queryBounds);
    for (Collider* col : nearby) {
        Vec3 normal;
        float penetration;
        BoxCollider box(center, halfExtents);
        if (col->TestCollision(&box, normal, penetration)) {
            result.push_back(col);
        }
    }
    
    return result;
}

bool PhysicsWorld::Raycast(const Vec3& origin, const Vec3& direction, float maxDistance, Vec3& outHit, Vec3& outNormal) {
    Vec3 dir = direction.normalize();
    float closest = maxDistance;
    bool hit = false;
    
    // Get nearby colliders using spatial grid
    Vec3 end = origin + dir * maxDistance;
    AABB rayBounds(
        Vec3(std::min(origin.x, end.x), std::min(origin.y, end.y), std::min(origin.z, end.z)),
        Vec3(std::max(origin.x, end.x), std::max(origin.y, end.y), std::max(origin.z, end.z))
    );
    
    auto nearby = GetNearbyColliders(rayBounds);
    
    for (Collider* col : nearby) {
        // Simple ray-AABB test first
        AABB bounds = col->GetBounds();
        
        // Slab method for ray-AABB intersection
        float tmin = 0.0f;
        float tmax = maxDistance;
        
        for (int i = 0; i < 3; ++i) {
            float o = (i == 0) ? origin.x : (i == 1) ? origin.y : origin.z;
            float d = (i == 0) ? dir.x : (i == 1) ? dir.y : dir.z;
            float bmin = (i == 0) ? bounds.min.x : (i == 1) ? bounds.min.y : bounds.min.z;
            float bmax = (i == 0) ? bounds.max.x : (i == 1) ? bounds.max.y : bounds.max.z;
            
            if (std::abs(d) < 0.0001f) {
                if (o < bmin || o > bmax) {
                    tmin = tmax + 1; // Miss
                    break;
                }
            } else {
                float t1 = (bmin - o) / d;
                float t2 = (bmax - o) / d;
                if (t1 > t2) std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) break;
            }
        }
        
        if (tmin <= tmax && tmin < closest) {
            closest = tmin;
            outHit = origin + dir * tmin;
            
            // Calculate normal based on which face was hit
            Vec3 center = (bounds.min + bounds.max) * 0.5f;
            Vec3 diff = outHit - center;
            Vec3 size = bounds.max - bounds.min;
            
            float maxComponent = std::max(std::abs(diff.x) / size.x * 2,
                                         std::max(std::abs(diff.y) / size.y * 2,
                                                 std::abs(diff.z) / size.z * 2));
            
            if (maxComponent == std::abs(diff.x) / size.x * 2) {
                outNormal = Vec3(diff.x > 0 ? 1.0f : -1.0f, 0, 0);
            } else if (maxComponent == std::abs(diff.y) / size.y * 2) {
                outNormal = Vec3(0, diff.y > 0 ? 1.0f : -1.0f, 0);
            } else {
                outNormal = Vec3(0, 0, diff.z > 0 ? 1.0f : -1.0f);
            }
            
            hit = true;
        }
    }
    
    return hit;
}

void PhysicsWorld::Clear() {
    bodies.clear();
    colliders.clear();
    spatialGrid.clear();
}

} // namespace vge