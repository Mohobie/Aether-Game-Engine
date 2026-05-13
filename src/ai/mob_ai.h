#pragma once
#include "math/vec3.h"
#include <vector>
#include <functional>

namespace vge {

// Forward declarations
class World;
class Mob;

// ============================================
// Mob AI States
// ============================================
enum class MobAIState {
    Idle,       // Standing still
    Wander,     // Random walking
    Chase,      // Following target
    Attack,     // Attacking target
    Flee,       // Running away
    Patrol      // Following a path
};

// ============================================
// Mob AI Controller
// ============================================
class MobAIController {
public:
    MobAIController(Mob* mob, World* world);
    ~MobAIController();

    // Update AI
    void Update(float deltaTime);
    
    // State changes
    void SetState(MobAIState newState);
    MobAIState GetState() const { return currentState; }
    
    // Target management
    void SetTarget(const Vec3& target);
    void SetTargetEntity(void* entity);
    void ClearTarget();
    bool HasTarget() const;
    
    // Detection
    void SetDetectionRange(float range) { detectionRange = range; }
    void SetAttackRange(float range) { attackRange = range; }
    float GetDetectionRange() const { return detectionRange; }
    float GetAttackRange() const { return attackRange; }
    
    // Pathfinding
    bool CanSeeTarget() const;
    float GetDistanceToTarget() const;
    Vec3 GetNextPathPoint() const;
    
    // Callbacks
    std::function<void()> onStateChange;
    std::function<void()> onAttack;
    std::function<void()> onTargetLost;

private:
    Mob* mob;
    World* world;
    MobAIState currentState;
    
    // Target
    Vec3 targetPosition;
    void* targetEntity;
    bool hasTarget;
    
    // Ranges
    float detectionRange;
    float attackRange;
    float loseInterestRange;
    
    // Timing
    float stateTimer;
    float attackCooldown;
    float pathUpdateTimer;
    
    // Pathfinding
    std::vector<Vec3> path;
    int currentPathIndex;
    
    // State handlers
    void UpdateIdle(float deltaTime);
    void UpdateWander(float deltaTime);
    void UpdateChase(float deltaTime);
    void UpdateAttack(float deltaTime);
    void UpdateFlee(float deltaTime);
    void UpdatePatrol(float deltaTime);
    
    // Pathfinding
    void UpdatePath();
    bool FindPath(const Vec3& start, const Vec3& end);
    bool IsPathClear(const Vec3& from, const Vec3& to) const;
};

} // namespace vge
