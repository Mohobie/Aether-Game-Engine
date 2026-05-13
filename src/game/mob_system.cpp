#include "mob_system.h"
#include "voxel/world.h"
#include "voxel/block_registry.h"
#include <cmath>
#include <cstdlib>

namespace vge {

MobSystem::MobSystem()
    : minSpawnRadius(20.0f), maxSpawnRadius(50.0f), maxMobCount(20), spawnTimer(0.0f) {
}

MobSystem::~MobSystem() {
}

void MobSystem::RegisterMob(const MobDef& def) {
    definitions.push_back(def);
}

Mob* MobSystem::SpawnMob(const std::string& mobId, const Vec3& position) {
    // Find definition
    const MobDef* def = nullptr;
    for (const auto& d : definitions) {
        if (d.id == mobId) {
            def = &d;
            break;
        }
    }
    if (!def) return nullptr;
    
    if (mobs.size() >= static_cast<size_t>(maxMobCount)) {
        return nullptr;
    }
    
    Mob mob;
    mob.defId = mobId;
    mob.position = position;
    mob.health = def->maxHealth;
    mob.alive = true;
    mob.aggro = false;
    
    mobs.push_back(mob);
    return &mobs.back();
}

void MobSystem::DespawnMob(int index) {
    if (index >= 0 && index < static_cast<int>(mobs.size())) {
        mobs.erase(mobs.begin() + index);
    }
}

void MobSystem::Update(float deltaTime, const Vec3& playerPos, float dayNightBlend) {
    // Update spawn timer
    spawnTimer += deltaTime;
    if (spawnTimer >= 5.0f) { // Check spawning every 5 seconds
        spawnTimer = 0.0f;
        UpdateSpawning(playerPos, dayNightBlend);
    }
    
    // Update all mobs
    for (size_t i = 0; i < mobs.size(); ++i) {
        Mob& mob = mobs[i];
        if (!mob.alive) continue;
        
        // Find definition
        const MobDef* def = nullptr;
        for (const auto& d : definitions) {
            if (d.id == mob.defId) {
                def = &d;
                break;
            }
        }
        if (!def) continue;
        
        // Update based on type
        switch (def->type) {
            case MobType::Passive:
                UpdatePassiveMob(mob, deltaTime);
                break;
            case MobType::Hostile:
                UpdateHostileMob(mob, deltaTime, playerPos);
                break;
            default:
                UpdateWandering(mob, deltaTime);
                break;
        }
        
        // Update movement
        UpdateMovement(mob, deltaTime);
        
        // Update cooldowns
        if (mob.attackCooldown > 0.0f) {
            mob.attackCooldown -= deltaTime;
        }
    }
    
    // Remove dead mobs
    mobs.erase(std::remove_if(mobs.begin(), mobs.end(),
        [](const Mob& m) { return !m.alive; }), mobs.end());
}

void MobSystem::UpdatePassiveMob(Mob& mob, float deltaTime) {
    // Passive mobs just wander and flee when hit
    if (mob.aggro) {
        // Flee from target
        Vec3 fleeDir = mob.position - mob.targetPosition;
        float len = fleeDir.length();
        if (len > 0.1f) {
            fleeDir = fleeDir / len;
            mob.velocity = fleeDir * 3.0f; // Run away
        }
        
        // Calm down after 5 seconds
        mob.wanderTimer += deltaTime;
        if (mob.wanderTimer >= 5.0f) {
            mob.aggro = false;
            mob.wanderTimer = 0.0f;
        }
    } else {
        UpdateWandering(mob, deltaTime);
    }
}

void MobSystem::UpdateHostileMob(Mob& mob, float deltaTime, const Vec3& playerPos) {
    float distToPlayer = (playerPos - mob.position).length();
    
    // Detection range
    if (distToPlayer < 15.0f) {
        mob.aggro = true;
        mob.targetPosition = playerPos;
    }
    
    if (mob.aggro) {
        if (distToPlayer > 25.0f) {
            // Lost interest
            mob.aggro = false;
        } else if (distToPlayer > 1.5f) {
            // Chase player
            Vec3 chaseDir = playerPos - mob.position;
            chaseDir.y = 0; // Keep on ground
            float len = chaseDir.length();
            if (len > 0.1f) {
                chaseDir = chaseDir / len;
                mob.velocity = chaseDir * 2.5f;
            }
        } else {
            // Attack player
            mob.velocity = Vec3(0, 0, 0);
            if (mob.attackCooldown <= 0.0f) {
                // Deal damage to player
                mob.attackCooldown = 1.0f; // 1 second between attacks
            }
        }
    } else {
        UpdateWandering(mob, deltaTime);
    }
}

void MobSystem::UpdateWandering(Mob& mob, float deltaTime) {
    mob.wanderTimer += deltaTime;
    
    if (mob.wanderTimer >= 3.0f) { // Change direction every 3 seconds
        mob.wanderTimer = 0.0f;
        
        // Random direction
        float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
        mob.velocity.x = std::cos(angle) * 1.0f;
        mob.velocity.z = std::sin(angle) * 1.0f;
        mob.velocity.y = 0;
    }
}

void MobSystem::UpdateMovement(Mob& mob, float deltaTime) {
    // Simple movement with gravity
    mob.position.x += mob.velocity.x * deltaTime;
    mob.position.z += mob.velocity.z * deltaTime;
    
    // Apply gravity
    mob.velocity.y -= 20.0f * deltaTime;
    mob.position.y += mob.velocity.y * deltaTime;
    
    // Ground collision (simple)
    if (mob.position.y < 10.0f) {
        mob.position.y = 10.0f;
        mob.velocity.y = 0;
    }
    
    // Friction
    mob.velocity.x *= 0.9f;
    mob.velocity.z *= 0.9f;
}

void MobSystem::DamageMob(int index, float damage) {
    if (index >= 0 && index < static_cast<int>(mobs.size())) {
        Mob& mob = mobs[index];
        mob.health -= damage;
        mob.aggro = true; // Angry when hit
        mob.targetPosition = mob.position; // Will flee from here
        
        if (mob.health <= 0.0f) {
            KillMob(index);
        }
    }
}

void MobSystem::KillMob(int index) {
    if (index >= 0 && index < static_cast<int>(mobs.size())) {
        mobs[index].alive = false;
        // TODO: Drop items
    }
}

void MobSystem::UpdateSpawning(const Vec3& playerPos, float dayNightBlend) {
    if (mobs.size() >= static_cast<size_t>(maxMobCount)) return;
    
    // Try to spawn a mob
    for (const auto& def : definitions) {
        // Check spawn conditions
        bool canSpawn = false;
        if (def.spawnsAtDay && dayNightBlend > 0.3f) canSpawn = true;
        if (def.spawnsAtNight && dayNightBlend < 0.3f) canSpawn = true;
        
        if (!canSpawn) continue;
        
        // Random position around player
        float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
        float dist = minSpawnRadius + static_cast<float>(rand()) / RAND_MAX * (maxSpawnRadius - minSpawnRadius);
        
        Vec3 spawnPos;
        spawnPos.x = playerPos.x + std::cos(angle) * dist;
        spawnPos.z = playerPos.z + std::sin(angle) * dist;
        spawnPos.y = 15.0f; // Spawn above ground
        
        if (CanSpawnHere(spawnPos)) {
            SpawnMob(def.id, spawnPos);
            break; // Spawn one mob per check
        }
    }
}

void MobSystem::SetSpawnRadius(float minRadius, float maxRadius) {
    minSpawnRadius = minRadius;
    maxSpawnRadius = maxRadius;
}

void MobSystem::SetMaxMobs(int maxCount) {
    maxMobCount = maxCount;
}

bool MobSystem::CanSpawnHere(const Vec3& pos) {
    // Simple spawn check - ensure not too close to player
    // In full implementation, check blocks, light level, etc.
    return true;
}

} // namespace vge
