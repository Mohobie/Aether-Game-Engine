#include "mob_ai.h"
#include "game/mob_system.h"
#include "voxel/world.h"
#include "core/logger.h"
#include <cmath>

namespace vge {

MobAIController::MobAIController(Mob* mobPtr, World* worldPtr)
    : mob(mobPtr), world(worldPtr), currentState(MobAIState::Idle),
      hasTarget(false), detectionRange(10.0f), attackRange(1.5f),
      loseInterestRange(20.0f), stateTimer(0.0f), attackCooldown(0.0f),
      pathUpdateTimer(0.0f), currentPathIndex(0) {
}

MobAIController::~MobAIController() {
}

void MobAIController::Update(float deltaTime) {
    stateTimer += deltaTime;
    attackCooldown -= deltaTime;
    pathUpdateTimer -= deltaTime;
    
    // Update path periodically
    if (pathUpdateTimer <= 0.0f && hasTarget) {
        UpdatePath();
        pathUpdateTimer = 0.5f;
    }
    
    // State machine
    switch (currentState) {
        case MobAIState::Idle:
            UpdateIdle(deltaTime);
            break;
        case MobAIState::Wander:
            UpdateWander(deltaTime);
            break;
        case MobAIState::Chase:
            UpdateChase(deltaTime);
            break;
        case MobAIState::Attack:
            UpdateAttack(deltaTime);
            break;
        case MobAIState::Flee:
            UpdateFlee(deltaTime);
            break;
        case MobAIState::Patrol:
            UpdatePatrol(deltaTime);
            break;
    }
}

void MobAIController::SetState(MobAIState newState) {
    if (currentState != newState) {
        currentState = newState;
        stateTimer = 0.0f;
        if (onStateChange) onStateChange();
    }
}

void MobAIController::SetTarget(const Vec3& target) {
    targetPosition = target;
    hasTarget = true;
    UpdatePath();
}

void MobAIController::SetTargetEntity(void* entity) {
    targetEntity = entity;
    hasTarget = true;
}

void MobAIController::ClearTarget() {
    hasTarget = false;
    targetEntity = nullptr;
    path.clear();
}

bool MobAIController::HasTarget() const {
    return hasTarget;
}

bool MobAIController::CanSeeTarget() const {
    if (!hasTarget || !mob || !world) return false;
    
    Vec3 mobPos = mob->position;
    Vec3 toTarget = targetPosition - mobPos;
    float dist = toTarget.length();
    
    if (dist > detectionRange) return false;
    
    // Simple line of sight check
    Vec3 direction = toTarget.normalize();
    float step = 0.5f;
    Vec3 current = mobPos;
    
    for (float d = 0; d < dist; d += step) {
        current = current + direction * step;
        int x = static_cast<int>(current.x);
        int y = static_cast<int>(current.y);
        int z = static_cast<int>(current.z);
        
        if (world->GetBlock(x, y, z) != BLOCK_AIR) {
            return false; // Blocked
        }
    }
    
    return true;
}

float MobAIController::GetDistanceToTarget() const {
    if (!hasTarget || !mob) return 9999.0f;
    return (targetPosition - mob->position).length();
}

Vec3 MobAIController::GetNextPathPoint() const {
    if (path.empty() || currentPathIndex >= static_cast<int>(path.size())) {
        return targetPosition;
    }
    return path[currentPathIndex];
}

void MobAIController::UpdateIdle(float deltaTime) {
    // Check for targets
    if (hasTarget && CanSeeTarget()) {
        float dist = GetDistanceToTarget();
        if (dist <= detectionRange) {
            SetState(MobAIState::Chase);
            return;
        }
    }
    
    // Randomly start wandering
    if (stateTimer > 2.0f) {
        if (rand() % 100 < 30) {
            SetState(MobAIState::Wander);
        }
        stateTimer = 0.0f;
    }
}

void MobAIController::UpdateWander(float deltaTime) {
    // Check for targets
    if (hasTarget && CanSeeTarget()) {
        float dist = GetDistanceToTarget();
        if (dist <= detectionRange) {
            SetState(MobAIState::Chase);
            return;
        }
    }
    
    // Wander randomly
    if (stateTimer > 3.0f || path.empty()) {
        // Pick random direction
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
        Vec3 randomTarget(
            mob->position.x + cos(angle) * 5.0f,
            mob->position.y,
            mob->position.z + sin(angle) * 5.0f
        );
        
        SetTarget(randomTarget);
        stateTimer = 0.0f;
    }
    
    // Move along path
    if (!path.empty() && currentPathIndex < static_cast<int>(path.size())) {
        Vec3 nextPoint = path[currentPathIndex];
        Vec3 direction = (nextPoint - mob->position).normalize();
        mob->position = mob->position + direction * 2.0f * deltaTime;
        
        if ((nextPoint - mob->position).length() < 0.5f) {
            currentPathIndex++;
        }
    }
}

void MobAIController::UpdateChase(float deltaTime) {
    if (!hasTarget) {
        SetState(MobAIState::Idle);
        return;
    }
    
    float dist = GetDistanceToTarget();
    
    // Lost interest
    if (dist > loseInterestRange) {
        ClearTarget();
        SetState(MobAIState::Idle);
        if (onTargetLost) onTargetLost();
        return;
    }
    
    // Within attack range
    if (dist <= attackRange) {
        SetState(MobAIState::Attack);
        return;
    }
    
    // Move towards target
    if (!path.empty() && currentPathIndex < static_cast<int>(path.size())) {
        Vec3 nextPoint = path[currentPathIndex];
        Vec3 direction = (nextPoint - mob->position).normalize();
        mob->position = mob->position + direction * 2.0f * deltaTime;
        
        if ((nextPoint - mob->position).length() < 0.5f) {
            currentPathIndex++;
        }
    } else {
        // Direct chase if no path
        Vec3 direction = (targetPosition - mob->position).normalize();
        mob->position = mob->position + direction * 2.0f * deltaTime;
    }
}

void MobAIController::UpdateAttack(float deltaTime) {
    if (!hasTarget) {
        SetState(MobAIState::Idle);
        return;
    }
    
    float dist = GetDistanceToTarget();
    
    // Target moved out of range
    if (dist > attackRange * 1.5f) {
        SetState(MobAIState::Chase);
        return;
    }
    
    // Attack
    if (attackCooldown <= 0.0f) {
        if (onAttack) onAttack();
        attackCooldown = 1.0f; // 1 second between attacks
    }
}

void MobAIController::UpdateFlee(float deltaTime) {
    if (!hasTarget) {
        SetState(MobAIState::Idle);
        return;
    }
    
    // Run away from target
    Vec3 direction = (mob->position - targetPosition).normalize();
    mob->position = mob->position + direction * 2.0f * 1.5f * deltaTime;
    
    // Stop fleeing when far enough
    if (GetDistanceToTarget() > detectionRange * 1.5f) {
        ClearTarget();
        SetState(MobAIState::Idle);
    }
}

void MobAIController::UpdatePatrol(float deltaTime) {
    // Similar to wander but follows a predefined path
    UpdateWander(deltaTime);
}

void MobAIController::UpdatePath() {
    if (!mob || !world) return;
    
    path.clear();
    currentPathIndex = 0;
    
    // Simple straight-line path
    Vec3 start = mob->position;
    Vec3 end = targetPosition;
    Vec3 direction = (end - start).normalize();
    float dist = (end - start).length();
    float step = 1.0f;
    
    for (float d = 0; d < dist; d += step) {
        Vec3 point = start + direction * d;
        
        // Check if point is valid (not inside blocks)
        int x = static_cast<int>(point.x);
        int y = static_cast<int>(point.y);
        int z = static_cast<int>(point.z);
        
        if (world->GetBlock(x, y, z) == BLOCK_AIR) {
            path.push_back(point);
        } else {
            // Try to go around (simple: go up)
            point.y += 1.0f;
            path.push_back(point);
        }
    }
    
    path.push_back(end);
}

bool MobAIController::FindPath(const Vec3& start, const Vec3& end) {
    // Simple straight-line pathfinding
    UpdatePath();
    return !path.empty();
}

bool MobAIController::IsPathClear(const Vec3& from, const Vec3& to) const {
    if (!world) return false;
    
    Vec3 direction = (to - from).normalize();
    float dist = (to - from).length();
    float step = 0.5f;
    Vec3 current = from;
    
    for (float d = 0; d < dist; d += step) {
        current = current + direction * step;
        int x = static_cast<int>(current.x);
        int y = static_cast<int>(current.y);
        int z = static_cast<int>(current.z);
        
        if (world->GetBlock(x, y, z) != BLOCK_AIR) {
            return false;
        }
    }
    
    return true;
}

} // namespace vge
