#pragma once
#include "math/vec3.h"
#include "voxel/world.h"
#include "rendering/light_system.h"
#include "game/day_night_cycle.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <random>

namespace vge {

// ============================================
// Spawn Condition Flags
// ============================================
enum class SpawnConditionFlags : uint32_t {
    None              = 0,
    RequiresNight     = 1 << 0,   // Only spawn at night
    RequiresDay       = 1 << 1,   // Only spawn during day
    RequiresGround    = 1 << 2,   // Needs solid block below
    RequiresDarkness  = 1 << 3,   // Light level below threshold
    RequiresLight     = 1 << 4,   // Light level above threshold
    DespawnAtDawn     = 1 << 5,   // Despawn when day begins
    DespawnAtDusk     = 1 << 6,   // Despawn when night begins
    ProtectedInCaves  = 1 << 7,   // Don't despawn if underground
    DespawnWhenDistant= 1 << 8,   // Despawn beyond radius
};

inline SpawnConditionFlags operator|(SpawnConditionFlags a, SpawnConditionFlags b) {
    return static_cast<SpawnConditionFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline bool HasFlag(SpawnConditionFlags flags, SpawnConditionFlags flag) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
}

// ============================================
// Spawn Archetype (user-defined entity type)
// ============================================
struct SpawnArchetype {
    std::string id;                    // Unique identifier (e.g., "goblin", "fire_elemental")
    std::string displayName;

    // Spawn conditions
    SpawnConditionFlags conditions = SpawnConditionFlags::None;
    int maxLightLevel = 15;            // For RequiresDarkness (spawn if light <= this)
    int minLightLevel = 0;             // For RequiresLight (spawn if light >= this)

    // Limits
    int maxPerChunk = 5;
    float spawnRadius = 32.0f;         // Spawn around player
    float despawnRadius = 64.0f;       // Despawn beyond this
    float spawnInterval = 2.0f;        // Seconds between spawn attempts

    // Spawn position
    bool needsSolidGround = false;
    float minSpawnHeight = 0.0f;
    float maxSpawnHeight = 255.0f;

    // User data (scripting/config can attach anything)
    std::unordered_map<std::string, float> floatProperties;
    std::unordered_map<std::string, std::string> stringProperties;

    SpawnArchetype() = default;
    SpawnArchetype(const std::string& archetypeId, const std::string& name)
        : id(archetypeId), displayName(name) {}
};

// ============================================
// Spawned Entity Instance
// ============================================
struct SpawnedEntity {
    std::string archetypeId;
    Vec3 position;
    Vec3 velocity;
    bool alive;
    bool inCave;
    int chunkX, chunkY, chunkZ;

    // User-defined runtime properties
    std::unordered_map<std::string, float> runtimeFloats;
    std::unordered_map<std::string, int> runtimeInts;

    SpawnedEntity(const std::string& type, const Vec3& pos)
        : archetypeId(type), position(pos), velocity(0, 0, 0),
          alive(true), inCave(false), chunkX(0), chunkY(0), chunkZ(0) {}
};

// ============================================
// Spawn Callbacks (for game-specific logic)
// ============================================
using OnEntitySpawnedFn = std::function<void(SpawnedEntity* entity, const SpawnArchetype& archetype)>;
using OnEntityDespawnedFn = std::function<void(SpawnedEntity* entity, const SpawnArchetype& archetype)>;
using ShouldDespawnFn = std::function<bool(SpawnedEntity* entity, const SpawnArchetype& archetype, float dayNightBlend)>;

// ============================================
// Entity Spawn System
// ============================================
// Generic spawn/despawn system. Users define archetypes via config/script.
// No hardcoded mob types — purely data-driven.
// ============================================
class EntitySpawnSystem {
private:
    World* world;
    LightSystem* lightSystem;
    DayNightCycle* dayNightCycle;

    std::vector<std::unique_ptr<SpawnedEntity>> entities;
    std::unordered_map<std::string, SpawnArchetype> archetypes;
    std::mt19937 rng;

    // Per-archetype spawn timers
    std::unordered_map<std::string, float> spawnTimers;

    // Callbacks
    OnEntitySpawnedFn onSpawned;
    OnEntityDespawnedFn onDespawned;
    ShouldDespawnFn customDespawnCheck;

    // Internal helpers
    bool CanSpawnAt(int x, int y, int z, const SpawnArchetype& archetype);
    bool IsValidSpawnPosition(int x, int y, int z);
    bool IsInCave(int x, int y, int z);
    int CountEntitiesInChunk(const std::string& archetypeId, int cx, int cy, int cz) const;
    Vec3 FindSpawnPosition(const Vec3& center, float radius, float minH, float maxH);
    const SpawnArchetype* GetArchetype(const std::string& id) const;

public:
    EntitySpawnSystem(World* w, LightSystem* lights, DayNightCycle* cycle);
    ~EntitySpawnSystem();

    // Archetype management (users register their entity types)
    void RegisterArchetype(const SpawnArchetype& archetype);
    void UnregisterArchetype(const std::string& id);
    bool HasArchetype(const std::string& id) const;
    const SpawnArchetype* GetArchetypePtr(const std::string& id) const;
    std::vector<std::string> GetArchetypeIds() const;

    // Update - call every frame
    void Update(float deltaTime, const Vec3& playerPosition);

    // Spawning
    SpawnedEntity* SpawnEntity(const std::string& archetypeId, const Vec3& position);
    void DespawnEntity(SpawnedEntity* entity);
    void DespawnAllEntities();
    void DespawnByArchetype(const std::string& archetypeId);

    // Day/Night transitions (optional hooks)
    void OnCycleTransition(float oldBlend, float newBlend);

    // Callbacks
    void SetOnSpawned(OnEntitySpawnedFn fn) { onSpawned = fn; }
    void SetOnDespawned(OnEntityDespawnedFn fn) { onDespawned = fn; }
    void SetCustomDespawnCheck(ShouldDespawnFn fn) { customDespawnCheck = fn; }

    // Queries
    size_t GetEntityCount() const { return entities.size(); }
    size_t GetEntityCountByArchetype(const std::string& archetypeId) const;
    std::vector<SpawnedEntity*> GetEntitiesInChunk(int chunkX, int chunkY, int chunkZ) const;
    std::vector<SpawnedEntity*> GetEntitiesInRadius(const Vec3& center, float radius) const;
    std::vector<SpawnedEntity*> GetAllEntities() const;
    std::vector<SpawnedEntity*> GetEntitiesByArchetype(const std::string& archetypeId) const;
};

} // namespace vge
