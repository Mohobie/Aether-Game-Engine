#include "ai/enemy_ai.h"
#include <cmath>
#include <iostream>

namespace vge {

// ============================================
// EnemyAI Implementation
// ============================================

EnemyAI::EnemyAI(EnemyType type, const Vec3& startPos)
    : currentState(EnemyState::IDLE)
    , enemyType(type)
    , health(100.0f)
    , maxHealth(100.0f)
    , damage(10.0f)
    , speed(3.0f)
    , attackRange(2.0f)
    , attackCooldown(1.0f)
    , attackCooldownTimer(0.0f)
    , detectionRange(15.0f)
    , position(startPos)
    , velocity(0, 0, 0)
    , targetPosition(startPos)
    , pathUpdateTimer(0.0f)
    , pathUpdateInterval(0.5f)
    , idleTimer(0.0f)
    , idleDuration(2.0f)
    , isDead(false)
    , isActive(true)
    , stateTimer(0.0f)
{
    // Configure based on enemy type
    switch (type) {
        case EnemyType::ZOMBIE:
            ConfigureZombie();
            break;
        case EnemyType::SKELETON:
            ConfigureSkeleton();
            break;
    }
}

void EnemyAI::ConfigureZombie() {
    maxHealth = 80.0f;
    health = maxHealth;
    damage = 15.0f;
    speed = 2.0f;      // Slow
    attackRange = 2.0f;
    attackCooldown = 1.2f;
    detectionRange = 12.0f;
}

void EnemyAI::ConfigureSkeleton() {
    maxHealth = 40.0f;
    health = maxHealth;
    damage = 8.0f;
    speed = 4.0f;      // Fast
    attackRange = 10.0f; // Ranged
    attackCooldown = 2.0f;
    detectionRange = 18.0f;
}

void EnemyAI::Update(float deltaTime, const Vec3& playerPos, const NavigationMesh* navMesh) {
    if (!isActive || isDead) return;
    
    // Update cooldowns
    if (attackCooldownTimer > 0.0f) {
        attackCooldownTimer -= deltaTime;
    }
    
    stateTimer += deltaTime;
    
    // State machine
    switch (currentState) {
        case EnemyState::IDLE:
            UpdateIdle(deltaTime, playerPos);
            break;
        case EnemyState::CHASE:
            UpdateChase(deltaTime, playerPos, navMesh);
            break;
        case EnemyState::ATTACK:
            UpdateAttack(deltaTime, playerPos);
            break;
        case EnemyState::DIE:
            UpdateDie(deltaTime);
            break;
    }
}

void EnemyAI::UpdateIdle(float deltaTime, const Vec3& playerPos) {
    idleTimer += deltaTime;
    
    // Wander randomly when idle
    if (idleTimer >= idleDuration) {
        idleTimer = 0.0f;
        idleDuration = 1.0f + (rand() % 30) / 10.0f; // 1-4 seconds
        
        // Pick random direction
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        Vec3 wanderDir(std::cos(angle) * 2.0f, 0, std::sin(angle) * 2.0f);
        targetPosition = position + wanderDir;
    }
    
    // Slow wander movement
    Vec3 dir = targetPosition - position;
    float dist = dir.length();
    if (dist > 0.5f) {
        dir = dir.normalize();
        velocity = dir * (speed * 0.3f);
        position = position + velocity * deltaTime;
    } else {
        velocity = Vec3(0, 0, 0);
    }
    
    // Check if player is within detection range
    if (ShouldChase(playerPos)) {
        SetState(EnemyState::CHASE);
    }
}

void EnemyAI::UpdateChase(float deltaTime, const Vec3& playerPos, const NavigationMesh* navMesh) {
    // Update path periodically
    pathUpdateTimer += deltaTime;
    if (pathUpdateTimer >= pathUpdateInterval) {
        pathUpdateTimer = 0.0f;
        UpdatePath(playerPos, navMesh);
    }
    
    // Follow path or move directly
    if (currentPath.IsValid() && !currentPath.HasReachedEnd()) {
        FollowPath(deltaTime);
    } else {
        MoveToward(playerPos, deltaTime);
    }
    
    // Check if close enough to attack
    if (ShouldAttack(playerPos)) {
        SetState(EnemyState::ATTACK);
        return;
    }
    
    // Check if lost interest
    if (ShouldLoseInterest(playerPos)) {
        SetState(EnemyState::IDLE);
        return;
    }
}

void EnemyAI::UpdateAttack(float deltaTime, const Vec3& playerPos) {
    // Face the player
    targetPosition = playerPos;
    
    // Stop moving while attacking
    velocity = Vec3(0, 0, 0);
    
    // Check if player moved out of attack range
    if (!ShouldAttack(playerPos)) {
        SetState(EnemyState::CHASE);
        return;
    }
    
    // Perform attack if cooldown is ready
    if (CanAttack()) {
        ResetAttackCooldown();
        
        if (onAttack) {
            onAttack();
        }
        
        std::cout << "[EnemyAI] " << (enemyType == EnemyType::ZOMBIE ? "Zombie" : "Skeleton")
                  << " attacks player for " << damage << " damage!" << std::endl;
    }
}

void EnemyAI::UpdateDie(float deltaTime) {
    (void)deltaTime;
    // Death animation or cleanup timer could go here
    // For now, just stay dead
    velocity = Vec3(0, 0, 0);
}

void EnemyAI::SetState(EnemyState newState) {
    if (currentState == newState) return;
    
    currentState = newState;
    stateTimer = 0.0f;
    
    // State entry actions
    switch (newState) {
        case EnemyState::IDLE:
            idleTimer = 0.0f;
            idleDuration = 1.0f + (rand() % 20) / 10.0f;
            currentPath.Reset();
            break;
        case EnemyState::CHASE:
            pathUpdateTimer = pathUpdateInterval; // Force immediate path update
            break;
        case EnemyState::ATTACK:
            velocity = Vec3(0, 0, 0);
            break;
        case EnemyState::DIE:
            velocity = Vec3(0, 0, 0);
            isDead = true;
            break;
    }
}

void EnemyAI::TakeDamage(float amount) {
    if (isDead) return;
    
    health -= amount;
    
    if (onDamageTaken) {
        onDamageTaken(amount);
    }
    
    std::cout << "[EnemyAI] " << (enemyType == EnemyType::ZOMBIE ? "Zombie" : "Skeleton")
              << " takes " << amount << " damage (" << health << "/" << maxHealth << " HP)" << std::endl;
    
    if (health <= 0.0f) {
        health = 0.0f;
        Die();
    }
}

void EnemyAI::Die() {
    if (isDead) return;
    
    SetState(EnemyState::DIE);
    
    if (onDeath) {
        onDeath();
    }
    
    std::cout << "[EnemyAI] " << (enemyType == EnemyType::ZOMBIE ? "Zombie" : "Skeleton")
              << " died!" << std::endl;
}

void EnemyAI::UpdatePath(const Vec3& target, const NavigationMesh* navMesh) {
    if (!navMesh) {
        currentPath.Reset();
        return;
    }
    
    std::vector<Vec3> waypoints = navMesh->FindPath(position, target);
    if (!waypoints.empty()) {
        currentPath.waypoints = waypoints;
        currentPath.Reset();
    } else {
        currentPath.Reset();
    }
}

void EnemyAI::FollowPath(float deltaTime) {
    if (!currentPath.IsValid() || currentPath.HasReachedEnd()) {
        return;
    }
    
    Vec3 waypoint = currentPath.GetCurrentWaypoint();
    Vec3 dir = waypoint - position;
    float dist = dir.length();
    
    // Move to next waypoint if close enough
    if (dist < 1.0f) {
        currentPath.GetNextWaypoint();
        if (!currentPath.HasReachedEnd()) {
            waypoint = currentPath.GetCurrentWaypoint();
            dir = waypoint - position;
            dist = dir.length();
        } else {
            return;
        }
    }
    
    // Move toward waypoint
    if (dist > 0.0f) {
        dir = dir.normalize();
        velocity = dir * speed;
        position = position + velocity * deltaTime;
    }
}

void EnemyAI::MoveToward(const Vec3& target, float deltaTime) {
    Vec3 dir = target - position;
    float dist = dir.length();
    
    if (dist > 0.5f) {
        dir = dir.normalize();
        velocity = dir * speed;
        position = position + velocity * deltaTime;
    } else {
        velocity = Vec3(0, 0, 0);
    }
}

void EnemyAI::StopMoving() {
    velocity = Vec3(0, 0, 0);
}

bool EnemyAI::ShouldChase(const Vec3& playerPos) const {
    float dist = (playerPos - position).length();
    return dist <= detectionRange && dist > attackRange;
}

bool EnemyAI::ShouldAttack(const Vec3& playerPos) const {
    float dist = (playerPos - position).length();
    return dist <= attackRange;
}

bool EnemyAI::ShouldLoseInterest(const Vec3& playerPos) const {
    float dist = (playerPos - position).length();
    return dist > detectionRange * 1.5f; // Lose interest at 1.5x detection range
}

} // namespace vge
