#include "ai/enemy_spawner.h"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace vge {

// ============================================
// EnemySpawner Implementation
// ============================================

EnemySpawner::EnemySpawner()
    : maxEnemies(20)
    , minSpawnRadius(20.0f)
    , maxSpawnRadius(40.0f)
    , spawnCooldown(3.0f)
    , spawnCooldownTimer(0.0f)
    , nightStartTime(0.75f)
    , nightEndTime(0.25f)
    , rng(std::random_device{}())
    , angleDist(0.0f, 2.0f * 3.14159f)
    , radiusDist(0.0f, 1.0f)
    , typeDist(0, 1)
    , currentTime(0.5f)
    , totalSpawned(0)
    , totalKilled(0)
{
}

void EnemySpawner::Update(float deltaTime, const Vec3& playerPos, const NavigationMesh* navMesh) {
    // Update spawn cooldown
    if (spawnCooldownTimer > 0.0f) {
        spawnCooldownTimer -= deltaTime;
    }
    
    // Update all enemies
    for (auto& spawned : enemies) {
        if (!spawned->enemy || spawned->enemy->IsDead()) {
            continue;
        }
        
        spawned->lifetime += deltaTime;
        spawned->enemy->Update(deltaTime, playerPos, navMesh);
        
        // Check if enemy died this frame
        if (spawned->enemy->IsDead() && !spawned->markedForRemoval) {
            spawned->markedForRemoval = true;
            totalKilled++;
            
            if (onEnemyKilled) {
                onEnemyKilled(spawned->enemy.get());
            }
        }
    }
    
    // Try spawning at night
    if (IsNightTime() && GetActiveEnemyCount() < maxEnemies && spawnCooldownTimer <= 0.0f) {
        if (TrySpawnRandomEnemy(playerPos, navMesh)) {
            spawnCooldownTimer = spawnCooldown;
        }
    }
    
    // Clean up dead enemies periodically
    RemoveDeadEnemies();
}

bool EnemySpawner::TrySpawnEnemy(const Vec3& playerPos, EnemyType type, const NavigationMesh* navMesh) {
    if (GetActiveEnemyCount() >= maxEnemies) {
        return false;
    }
    
    if (!IsNightTime()) {
        return false;
    }
    
    Vec3 spawnPos = FindSpawnPosition(playerPos);
    if (!IsValidSpawnPosition(spawnPos, playerPos)) {
        return false;
    }
    
    // Create enemy
    auto enemy = std::make_unique<EnemyAI>(type, spawnPos);
    
    // Set up death callback
    enemy->onDeath = [this, enemyPtr = enemy.get()]() {
        totalKilled++;
        if (onEnemyKilled) {
            onEnemyKilled(enemyPtr);
        }
    };
    
    if (onEnemySpawned) {
        onEnemySpawned(enemy.get());
    }
    
    enemies.push_back(std::make_unique<SpawnedEnemy>(std::move(enemy)));
    totalSpawned++;
    
    std::cout << "[EnemySpawner] Spawned " << (type == EnemyType::ZOMBIE ? "Zombie" : "Skeleton")
              << " at (" << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ")"
              << " [" << GetActiveEnemyCount() << "/" << maxEnemies << "]" << std::endl;
    
    return true;
}

bool EnemySpawner::TrySpawnRandomEnemy(const Vec3& playerPos, const NavigationMesh* navMesh) {
    // 60% zombie, 40% skeleton
    int roll = rand() % 100;
    EnemyType type = (roll < 60) ? EnemyType::ZOMBIE : EnemyType::SKELETON;
    
    return TrySpawnEnemy(playerPos, type, navMesh);
}

void EnemySpawner::ForceSpawnEnemy(const Vec3& position, EnemyType type) {
    auto enemy = std::make_unique<EnemyAI>(type, position);
    
    enemy->onDeath = [this, enemyPtr = enemy.get()]() {
        totalKilled++;
        if (onEnemyKilled) {
            onEnemyKilled(enemyPtr);
        }
    };
    
    if (onEnemySpawned) {
        onEnemySpawned(enemy.get());
    }
    
    enemies.push_back(std::make_unique<SpawnedEnemy>(std::move(enemy)));
    totalSpawned++;
    
    std::cout << "[EnemySpawner] Force spawned " << (type == EnemyType::ZOMBIE ? "Zombie" : "Skeleton")
              << " at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void EnemySpawner::AddLightSource(const Vec3& position, float radius, float intensity) {
    lightSources.emplace_back(position, radius, intensity);
}

void EnemySpawner::RemoveLightSource(const Vec3& position, float tolerance) {
    lightSources.erase(
        std::remove_if(lightSources.begin(), lightSources.end(),
            [&position, tolerance](const LightSource& light) {
                return (light.position - position).length() < tolerance;
            }),
        lightSources.end()
    );
}

void EnemySpawner::ClearLightSources() {
    lightSources.clear();
}

bool EnemySpawner::IsInSafeZone(const Vec3& position) const {
    for (const auto& light : lightSources) {
        float dist = (position - light.position).length();
        if (dist <= light.radius * light.intensity) {
            return true;
        }
    }
    return false;
}

bool EnemySpawner::IsNightTime() const {
    // Night time: before 0.25 (dawn) or after 0.75 (dusk)
    return currentTime > nightStartTime || currentTime < nightEndTime;
}

void EnemySpawner::SetSpawnRadius(float minRadius, float maxRadius) {
    minSpawnRadius = minRadius;
    maxSpawnRadius = maxRadius;
}

std::vector<EnemyAI*> EnemySpawner::GetAllEnemies() const {
    std::vector<EnemyAI*> result;
    for (const auto& spawned : enemies) {
        if (spawned->enemy) {
            result.push_back(spawned->enemy.get());
        }
    }
    return result;
}

std::vector<EnemyAI*> EnemySpawner::GetEnemiesInRadius(const Vec3& center, float radius) const {
    std::vector<EnemyAI*> result;
    for (const auto& spawned : enemies) {
        if (spawned->enemy && !spawned->enemy->IsDead()) {
            if ((spawned->enemy->GetPosition() - center).length() <= radius) {
                result.push_back(spawned->enemy.get());
            }
        }
    }
    return result;
}

int EnemySpawner::GetActiveEnemyCount() const {
    int count = 0;
    for (const auto& spawned : enemies) {
        if (spawned->enemy && !spawned->enemy->IsDead()) {
            count++;
        }
    }
    return count;
}

void EnemySpawner::RemoveDeadEnemies() {
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const std::unique_ptr<SpawnedEnemy>& spawned) {
                return spawned->markedForRemoval || 
                       (spawned->enemy && spawned->enemy->IsDead());
            }),
        enemies.end()
    );
}

void EnemySpawner::ClearAllEnemies() {
    enemies.clear();
    totalKilled += totalSpawned; // Count cleared enemies as killed
}

Vec3 EnemySpawner::FindSpawnPosition(const Vec3& playerPos) const {
    return GetRandomPointInAnnulus(playerPos, minSpawnRadius, maxSpawnRadius);
}

bool EnemySpawner::IsValidSpawnPosition(const Vec3& position, const Vec3& playerPos) const {
    // Check distance bounds
    float distToPlayer = (position - playerPos).length();
    if (distToPlayer < minSpawnRadius || distToPlayer > maxSpawnRadius * 1.5f) {
        return false;
    }
    
    // Check safe zones
    if (IsInSafeZone(position)) {
        return false;
    }
    
    return true;
}

Vec3 EnemySpawner::GetRandomPointInAnnulus(const Vec3& center, float minR, float maxR) const {
    float angle = angleDist(rng);
    float r = minR + radiusDist(rng) * (maxR - minR);
    
    float x = center.x + r * std::cos(angle);
    float z = center.z + r * std::sin(angle);
    float y = center.y; // Same height as player for now
    
    return Vec3(x, y, z);
}

} // namespace vge
