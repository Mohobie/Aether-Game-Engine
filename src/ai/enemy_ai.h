#pragma once
#include "math/vec3.h"
#include "ai/ai_system.h"
#include <vector>
#include <string>
#include <functional>

namespace vge {

// ============================================
// Enemy AI States
// ============================================
enum class EnemyState {
    IDLE,
    CHASE,
    ATTACK,
    DIE
};

// ============================================
// Enemy Types
// ============================================
enum class EnemyType {
    ZOMBIE,
    SKELETON
};

// ============================================
// Enemy AI Component
// ============================================
class EnemyAI {
private:
    EnemyState currentState;
    EnemyType enemyType;
    
    // Stats
    float health;
    float maxHealth;
    float damage;
    float speed;
    float attackRange;
    float attackCooldown;
    float attackCooldownTimer;
    float detectionRange;
    
    // Position and movement
    Vec3 position;
    Vec3 velocity;
    Vec3 targetPosition;
    
    // Pathfinding
    AIPath currentPath;
    float pathUpdateTimer;
    float pathUpdateInterval;
    
    // State timing
    float idleTimer;
    float idleDuration;
    bool isDead;
    bool isActive;
    
    // Animation state
    float stateTimer;
    
public:
    EnemyAI(EnemyType type, const Vec3& startPos);
    
    // Core update
    void Update(float deltaTime, const Vec3& playerPos, const NavigationMesh* navMesh = nullptr);
    
    // State management
    void SetState(EnemyState newState);
    EnemyState GetState() const { return currentState; }
    EnemyType GetType() const { return enemyType; }
    
    // Combat
    void TakeDamage(float amount);
    void Die();
    bool CanAttack() const { return attackCooldownTimer <= 0.0f; }
    void ResetAttackCooldown() { attackCooldownTimer = attackCooldown; }
    float GetDamage() const { return damage; }
    bool IsDead() const { return isDead; }
    bool IsActive() const { return isActive; }
    void SetActive(bool active) { isActive = active; }
    
    // Health
    float GetHealth() const { return health; }
    float GetMaxHealth() const { return maxHealth; }
    float GetHealthPercent() const { return health / maxHealth; }
    bool IsAlive() const { return health > 0.0f && !isDead; }
    
    // Position
    Vec3 GetPosition() const { return position; }
    void SetPosition(const Vec3& pos) { position = pos; }
    Vec3 GetVelocity() const { return velocity; }
    
    // Stats
    float GetSpeed() const { return speed; }
    float GetAttackRange() const { return attackRange; }
    float GetDetectionRange() const { return detectionRange; }
    
    // Type-specific configuration
    void ConfigureZombie();
    void ConfigureSkeleton();
    
    // Callbacks
    std::function<void()> onDeath;
    std::function<void()> onAttack;
    std::function<void(float)> onDamageTaken;
    
private:
    // State handlers
    void UpdateIdle(float deltaTime, const Vec3& playerPos);
    void UpdateChase(float deltaTime, const Vec3& playerPos, const NavigationMesh* navMesh);
    void UpdateAttack(float deltaTime, const Vec3& playerPos);
    void UpdateDie(float deltaTime);
    
    // Pathfinding
    void UpdatePath(const Vec3& target, const NavigationMesh* navMesh);
    void FollowPath(float deltaTime);
    
    // Movement
    void MoveToward(const Vec3& target, float deltaTime);
    void StopMoving();
    
    // State transitions
    bool ShouldChase(const Vec3& playerPos) const;
    bool ShouldAttack(const Vec3& playerPos) const;
    bool ShouldLoseInterest(const Vec3& playerPos) const;
};

} // namespace vge
