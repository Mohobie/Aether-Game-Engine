#pragma once
#include "ai/enemy_ai.h"
#include "math/vec3.h"
#include <vector>
#include <memory>
#include <random>

namespace vge {

// ============================================
// Light Source Info (for safe zones)
// ============================================
struct LightSource {
    Vec3 position;
    float radius;
    float intensity;
    
    LightSource(const Vec3& pos, float r, float i = 1.0f)
        : position(pos), radius(r), intensity(i) {}
};

// ============================================
// Spawned Enemy Entry
// ============================================
struct SpawnedEnemy {
    std::unique_ptr<EnemyAI> enemy;
    float lifetime;
    bool markedForRemoval;
    
    SpawnedEnemy(std::unique_ptr<EnemyAI> e)
        : enemy(std::move(e)), lifetime(0.0f), markedForRemoval(false) {}
};

// ============================================
// Enemy Spawner
// ============================================
class EnemySpawner {
private:
    std::vector<std::unique_ptr<SpawnedEnemy>> enemies;
    std::vector<LightSource> lightSources;
    
    // Spawn configuration
    int maxEnemies;
    float minSpawnRadius;
    float maxSpawnRadius;
    float spawnCooldown;
    float spawnCooldownTimer;
    float nightStartTime;
    float nightEndTime;
    
    // Random generation
    mutable std::mt19937 rng;
    mutable std::uniform_real_distribution<float> angleDist;
    mutable std::uniform_real_distribution<float> radiusDist;
    mutable std::uniform_int_distribution<int> typeDist;
    
    // Current time (0-1, where 0.5 is noon)
    float currentTime;
    
    // Statistics
    int totalSpawned;
    int totalKilled;
    
public:
    EnemySpawner();
    
    // Update
    void Update(float deltaTime, const Vec3& playerPos, const NavigationMesh* navMesh = nullptr);
    
    // Spawning
    bool TrySpawnEnemy(const Vec3& playerPos, EnemyType type, const NavigationMesh* navMesh = nullptr);
    bool TrySpawnRandomEnemy(const Vec3& playerPos, const NavigationMesh* navMesh = nullptr);
    void ForceSpawnEnemy(const Vec3& position, EnemyType type);
    
    // Light sources (safe zones)
    void AddLightSource(const Vec3& position, float radius, float intensity = 1.0f);
    void RemoveLightSource(const Vec3& position, float tolerance = 1.0f);
    void ClearLightSources();
    bool IsInSafeZone(const Vec3& position) const;
    
    // Time
    void SetTime(float time) { currentTime = time; }
    float GetTime() const { return currentTime; }
    bool IsNightTime() const;
    
    // Configuration
    void SetMaxEnemies(int max) { maxEnemies = max; }
    int GetMaxEnemies() const { return maxEnemies; }
    void SetSpawnRadius(float minRadius, float maxRadius);
    float GetMinSpawnRadius() const { return minSpawnRadius; }
    float GetMaxSpawnRadius() const { return maxSpawnRadius; }
    void SetSpawnCooldown(float cooldown) { spawnCooldown = cooldown; }
    float GetSpawnCooldown() const { return spawnCooldown; }
    
    // Enemy management
    std::vector<EnemyAI*> GetAllEnemies() const;
    std::vector<EnemyAI*> GetEnemiesInRadius(const Vec3& center, float radius) const;
    int GetEnemyCount() const { return enemies.size(); }
    int GetActiveEnemyCount() const;
    void RemoveDeadEnemies();
    void ClearAllEnemies();
    
    // Statistics
    int GetTotalSpawned() const { return totalSpawned; }
    int GetTotalKilled() const { return totalKilled; }
    
    // Callbacks
    std::function<void(EnemyAI*)> onEnemySpawned;
    std::function<void(EnemyAI*)> onEnemyKilled;
    
private:
    // Spawn helpers
    Vec3 FindSpawnPosition(const Vec3& playerPos) const;
    bool IsValidSpawnPosition(const Vec3& position, const Vec3& playerPos) const;
    Vec3 GetRandomPointInAnnulus(const Vec3& center, float minR, float maxR) const;
};

} // namespace vge
