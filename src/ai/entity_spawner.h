#pragma once
#include "math/vec3.h"
#include "voxel/world.h"
#include "voxel/chunk.h"
#include "rendering/light_system.h"
#include "game/day_night_cycle.h"
#include "entity_ai.h"
#include <vector>
#include <memory>
#include <random>
#include <functional>

namespace vge {

// ============================================
// Spawn Condition
// ============================================
struct SpawnCondition {
    // Time of day (0.0-1.0, where 0.25=noon, 0.75=midnight)
    float minTimeOfDay = 0.0f;
    float maxTimeOfDay = 1.0f;
    
    // Light level (0-15)
    int minLightLevel = 0;
    int maxLightLevel = 15;
    
    // Height
    int minY = 0;
    int maxY = 255;
    
    // Biome (empty = any)
    std::vector<std::string> allowedBiomes;
    
    // Weather
    bool requiresRain = false;
    bool requiresStorm = false;
    
    // Group spawning
    int minGroupSize = 1;
    int maxGroupSize = 1;
    float groupRadius = 5.0f;
};

// ============================================
// Spawned Entity Instance
// ============================================
struct SpawnedEntityInstance {
    uint32_t id;
    std::string archetypeId;
    std::unique_ptr<AIEntity> entity;
    std::unique_ptr<EntityAIController> controller;
    
    SpawnedEntityInstance(uint32_t eid, const std::string& aid, 
                          std::unique_ptr<AIEntity> ent, 
                          std::unique_ptr<EntityAIController> ctrl)
        : id(eid), archetypeId(aid), entity(std::move(ent)), controller(std::move(ctrl)) {}
};

// ============================================
// Generic Entity Spawner
// ============================================
// Spawns any entity type based on archetype definitions.
// No hardcoded mob types - fully data-driven.
// ============================================
class GenericEntitySpawner {
private:
    World* world;
    LightSystem* lightSystem;
    DayNightCycle* dayNightCycle;
    
    std::vector<std::unique_ptr<SpawnedEntityInstance>> entities;
    uint32_t nextEntityId = 1;
    float spawnTimer = 0.0f;
    mutable std::mt19937 rng;
    
    // Spawn configuration
    static constexpr int DEFAULT_MAX_ENTITIES = 100;
    static constexpr float DEFAULT_SPAWN_RADIUS = 32.0f;
    static constexpr float DEFAULT_DESPAWN_RADIUS = 64.0f;
    static constexpr float DEFAULT_SPAWN_INTERVAL = 2.0f;
    
    int maxEntities = DEFAULT_MAX_ENTITIES;
    float spawnRadius = DEFAULT_SPAWN_RADIUS;
    float despawnRadius = DEFAULT_DESPAWN_RADIUS;
    float spawnInterval = DEFAULT_SPAWN_INTERVAL;
    
    // Internal helpers
    void TrySpawnEntities(const Vec3& playerPosition);
    bool CanSpawnArchetypeAt(const Vec3& pos, const EntityArchetype& archetype) const;
    bool IsValidSpawnPosition(const Vec3& pos, const EntityArchetype& archetype) const;
    Vec3 FindSpawnPosition(const Vec3& center, float radius) const;
    void DespawnDistantEntities(const Vec3& playerPos);
    void DespawnEntitiesByCondition(std::function<bool(const SpawnedEntityInstance&)> condition);
    
    // Get spawnable archetypes for current conditions
    std::vector<const EntityArchetype*> GetSpawnableArchetypes(const Vec3& pos) const;
    
public:
    GenericEntitySpawner(World* w, LightSystem* lights, DayNightCycle* cycle);
    ~GenericEntitySpawner();
    
    // Update - call every frame
    void Update(float deltaTime, const Vec3& playerPosition);
    
    // Spawning
    SpawnedEntityInstance* SpawnEntity(const std::string& archetypeId, const Vec3& position);
    void DespawnEntity(uint32_t entityId);
    void DespawnAllEntities();
    void DespawnEntitiesByArchetype(const std::string& archetypeId);
    
    // Configuration
    void SetMaxEntities(int max) { maxEntities = max; }
    void SetSpawnRadius(float radius) { spawnRadius = radius; }
    void SetDespawnRadius(float radius) { despawnRadius = radius; }
    void SetSpawnInterval(float interval) { spawnInterval = interval; }
    
    // Queries
    size_t GetEntityCount() const { return entities.size(); }
    size_t GetEntityCountByArchetype(const std::string& archetypeId) const;
    SpawnedEntityInstance* GetEntity(uint32_t id) const;
    std::vector<SpawnedEntityInstance*> GetEntitiesInRadius(const Vec3& center, float radius) const;
    std::vector<SpawnedEntityInstance*> GetEntitiesByArchetype(const std::string& archetypeId) const;
    std::vector<SpawnedEntityInstance*> GetAllEntities() const;
    
    // Events
    std::function<void(const std::string& archetypeId, const Vec3& pos)> onEntitySpawned;
    std::function<void(uint32_t entityId, const std::string& archetypeId)> onEntityDespawned;
};

} // namespace vge
