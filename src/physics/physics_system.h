#pragma once
#include "math/vec3.h"
#include "collision.h"
#include <vector>
#include <memory>
#include <functional>

namespace vge {

// Forward declarations
class Rigidbody;
class Collider;
class PhysicsWorld;

// ============================================
// Collision Shape Types
// ============================================
enum class CollisionShapeType {
    Box,
    Sphere,
    Capsule,
    Mesh
};

// ============================================
// Base Collider
// ============================================
class Collider {
public:
    CollisionShapeType type;
    bool isTrigger;      // If true, doesn't block movement but detects overlap
    bool isStatic;       // If true, doesn't move (optimization)
    
    // Callbacks
    std::function<void(Collider* other)> onCollisionEnter;
    std::function<void(Collider* other)> onCollisionStay;
    std::function<void(Collider* other)> onCollisionExit;
    std::function<void(Collider* other)> onTriggerEnter;
    std::function<void(Collider* other)> onTriggerStay;
    std::function<void(Collider* other)> onTriggerExit;
    
    Collider(CollisionShapeType t) : type(t), isTrigger(false), isStatic(false) {}
    virtual ~Collider() = default;
    
    // Get bounding box for broad-phase
    virtual AABB GetBounds() const = 0;
    
    // Test collision with another collider
    virtual bool TestCollision(const Collider* other, Vec3& outNormal, float& outPenetration) const = 0;
    
    // Test point inside
    virtual bool ContainsPoint(const Vec3& point) const = 0;
    
    // Update position (for dynamic colliders)
    virtual void UpdatePosition(const Vec3& position) = 0;
};

// ============================================
// Box Collider
// ============================================
class BoxCollider : public Collider {
public:
    Vec3 center;
    Vec3 halfExtents;  // Half size in each axis
    
    BoxCollider(const Vec3& center = Vec3(0,0,0), const Vec3& halfExtents = Vec3(0.5f, 0.5f, 0.5f))
        : Collider(CollisionShapeType::Box), center(center), halfExtents(halfExtents) {}
    
    AABB GetBounds() const override;
    bool TestCollision(const Collider* other, Vec3& outNormal, float& outPenetration) const override;
    bool ContainsPoint(const Vec3& point) const override;
    void UpdatePosition(const Vec3& position) override { center = position; }
    
    // Box-specific collision tests
    bool TestBoxCollision(const BoxCollider* other, Vec3& outNormal, float& outPenetration) const;
    bool TestSphereCollision(const class SphereCollider* other, Vec3& outNormal, float& outPenetration) const;
};

// ============================================
// Sphere Collider
// ============================================
class SphereCollider : public Collider {
public:
    Vec3 center;
    float radius;
    
    SphereCollider(const Vec3& center = Vec3(0,0,0), float radius = 0.5f)
        : Collider(CollisionShapeType::Sphere), center(center), radius(radius) {}
    
    AABB GetBounds() const override;
    bool TestCollision(const Collider* other, Vec3& outNormal, float& outPenetration) const override;
    bool ContainsPoint(const Vec3& point) const override;
    void UpdatePosition(const Vec3& position) override { center = position; }
    
    // Sphere-specific collision tests
    bool TestSphereCollision(const SphereCollider* other, Vec3& outNormal, float& outPenetration) const;
    bool TestBoxCollision(const BoxCollider* other, Vec3& outNormal, float& outPenetration) const;
};

// ============================================
// Rigidbody - Physics Object
// ============================================
class Rigidbody {
public:
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Vec3 forceAccumulator;
    
    float mass;
    float inverseMass;
    float drag;           // Air resistance
    float angularDrag;    // Rotation resistance
    
    bool useGravity;
    bool isKinematic;     // If true, not affected by forces
    bool isSleeping;      // Optimization: don't simulate if not moving
    
    // Grounding
    bool isGrounded;
    float groundCheckDistance;
    
    Collider* collider;
    
    Rigidbody(float mass = 1.0f);
    
    // Force application
    void AddForce(const Vec3& force);
    void AddImpulse(const Vec3& impulse);
    void AddForceAtPosition(const Vec3& force, const Vec3& position);
    
    // Velocity helpers
    void SetVelocity(const Vec3& vel) { velocity = vel; }
    Vec3 GetVelocity() const { return velocity; }
    
    // Position helpers
    void SetPosition(const Vec3& pos);
    Vec3 GetPosition() const { return position; }
    
    // Mass helpers
    void SetMass(float m);
    float GetMass() const { return mass; }
    bool HasFiniteMass() const { return inverseMass > 0; }
    
    // Ground check
    void UpdateGroundState(const std::vector<Collider*>& colliders);
    
    // Sleep management
    void WakeUp();
    void Sleep();
};

// ============================================
// Physics World - Main Simulation
// ============================================
class PhysicsWorld {
private:
    std::vector<std::unique_ptr<Rigidbody>> bodies;
    std::vector<std::unique_ptr<Collider>> colliders;
    
    Vec3 gravity;
    int solverIterations;
    float sleepThreshold;
    
    // Spatial partitioning (simple grid)
    float cellSize;
    std::unordered_map<uint64_t, std::vector<Collider*>> spatialGrid;
    
public:
    PhysicsWorld(const Vec3& gravity = Vec3(0, -9.81f, 0));
    
    // Body management
    Rigidbody* CreateBody(float mass = 1.0f);
    void DestroyBody(Rigidbody* body);
    
    // Collider management
    BoxCollider* CreateBoxCollider(const Vec3& center, const Vec3& halfExtents);
    SphereCollider* CreateSphereCollider(const Vec3& center, float radius);
    void DestroyCollider(Collider* collider);
    
    // Simulation
    void Step(float deltaTime);
    void UpdateBodies(float deltaTime);
    void DetectCollisions();
    void ResolveCollisions();
    void UpdateSpatialGrid();
    
    // Queries
    std::vector<Collider*> OverlapSphere(const Vec3& center, float radius);
    std::vector<Collider*> OverlapBox(const Vec3& center, const Vec3& halfExtents);
    bool Raycast(const Vec3& origin, const Vec3& direction, float maxDistance, Vec3& outHit, Vec3& outNormal);
    
    // Settings
    void SetGravity(const Vec3& g) { gravity = g; }
    Vec3 GetGravity() const { return gravity; }
    
    // Stats
    size_t GetBodyCount() const { return bodies.size(); }
    size_t GetColliderCount() const { return colliders.size(); }
    
    void Clear();
    
private:
    // Spatial hash key
    uint64_t GetGridKey(int x, int y, int z) const;
    void InsertToGrid(Collider* collider);
    std::vector<Collider*> GetNearbyColliders(const AABB& bounds);
};

// ============================================
// Collision Contact
// ============================================
struct Contact {
    Collider* colliderA;
    Collider* colliderB;
    Vec3 point;
    Vec3 normal;
    float penetration;
    bool isTrigger;
};

} // namespace vge
