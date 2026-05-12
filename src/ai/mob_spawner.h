#pragma once
#include "math/vec3.h"
#include "voxel/world.h"
#include "voxel/chunk.h"
#include "rendering/light_system.h"
#include "game/day_night_cycle.h"
#include <vector>
#include <memory>
#include <random>

namespace vge {

// ============================================
// Mob Type
// ============================================
enum class MobType {
    Zombie,
    Skeleton,
    Spider
};

// ============================================
// Mob Definition
// ============================================
struct MobDef {
    MobType type;
    std::string name;
    float health;
    float speed;
    float damage;
    int minLightLevel;      // Max light to spawn (e.g., 0-6)
    bool spawnsOnGround;    // Needs solid ground
    bool canClimbWalls;     // Spider trait
    bool burnsInDaylight;   // Zombie/Skeleton trait

    MobDef(MobType t, const std::string& n, float hp, float spd, float dmg,
           int light, bool ground, bool climb, bool burn)
        : type(t), name(n), health(hp), speed(spd), damage(dmg),
          minLightLevel(light), spawnsOnGround(ground), canClimbWalls(climb), burnsInDaylight(burn) {}
};

// ============================================
// Spawned Mob
// ============================================
struct Mob {
    MobType type;
    Vec3 position;
    Vec3 velocity;
    float health;
    bool alive;
    bool inCave;            // Protected from daylight
    int chunkX, chunkY, chunkZ;

    Mob(MobType t, const Vec3& pos)
        : type(t), position(pos), velocity(0, 0, 0),
          health(20.0f), alive(true), inCave(false),
          chunkX(0), chunkY(0), chunkZ(0) {}
};

// ============================================
// Mob Spawner
// ============================================
// Spawns enemies at night, despawns at dawn.
// Respects spawn caps per chunk and light levels.
// ============================================
class MobSpawner {
private:
    World* world;
    LightSystem* lightSystem;
    DayNightCycle* dayNightCycle;

    std::vector<std::unique_ptr<Mob>> mobs;
    float spawnTimer;
    std::mt19937 rng;

    // Spawn configuration
    static constexpr int MAX_MOBS_PER_CHUNK = 5;
    static constexpr float SPAWN_RADIUS = 32.0f;      // Around player
    static constexpr float DESPAWN_RADIUS = 64.0f;    // Beyond this, despawn
    static constexpr float SPAWN_INTERVAL = 2.0f;     // Seconds between spawn attempts
    // Mob definitions
    static MobDef GetMobDef(MobType type);

    // Internal helpers
    void TrySpawnMobs(const Vec3& playerPosition);
    bool CanSpawnAt(int x, int y, int z, MobType type);
    bool IsValidSpawnPosition(int x, int y, int z);
    bool IsInCave(int x, int y, int z);
    int CountMobsInChunk(int chunkX, int chunkY, int chunkZ) const;
    Vec3 FindSpawnPosition(const Vec3& center, float radius);
    void DespawnDistantMobs(const Vec3& playerPos);
    void DespawnDaylightMobs();
    void BurnDaylightMobs();

public:
    MobSpawner(World* w, LightSystem* lights, DayNightCycle* cycle);
    ~MobSpawner();

    // Update - call every frame
    void Update(float deltaTime, const Vec3& playerPosition);

    // Spawning
    Mob* SpawnMob(MobType type, const Vec3& position);
    void DespawnMob(Mob* mob);
    void DespawnAllMobs();

    // Day/Night transitions
    void OnNightBegin();
    void OnDawnBegin();

    // Queries
    size_t GetMobCount() const { return mobs.size(); }
    std::vector<Mob*> GetMobsInChunk(int chunkX, int chunkY, int chunkZ) const;
    std::vector<Mob*> GetMobsInRadius(const Vec3& center, float radius) const;
    std::vector<Mob*> GetAllMobs() const;

    // Mob properties
    static MobDef GetMobDefForType(MobType type) { return GetMobDef(type); }
};

} // namespace vge
