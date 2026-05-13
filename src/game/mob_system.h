#pragma once
#include "math/vec3.h"
#include <string>
#include <vector>
#include <functional>

namespace vge {

// ============================================
// Mob Types
// ============================================
enum class MobType {
    Passive,    // Animals, won't attack
    Neutral,    // Attack if provoked
    Hostile     // Attack on sight
};

// ============================================
// Mob Definition
// ============================================
struct MobDef {
    std::string id;
    std::string name;
    MobType type;
    float maxHealth;
    float speed;
    float damage;
    Vec3 color;
    float size;
    bool spawnsAtNight;
    bool spawnsAtDay;
    std::vector<std::string> drops;
};

// ============================================
// Mob Instance
// ============================================
struct Mob {
    std::string defId;
    Vec3 position;
    Vec3 velocity;
    float health;
    float yaw;
    float pitch;
    bool alive;
    bool aggro;
    Vec3 targetPosition;
    float wanderTimer;
    float attackCooldown;
    
    Mob() : health(10.0f), yaw(0.0f), pitch(0.0f), alive(true), 
            aggro(false), wanderTimer(0.0f), attackCooldown(0.0f) {}
};

// ============================================
// Mob System
// ============================================
class MobSystem {
public:
    MobSystem();
    ~MobSystem();

    // Register mob types
    void RegisterMob(const MobDef& def);
    
    // Spawn mobs
    Mob* SpawnMob(const std::string& mobId, const Vec3& position);
    void DespawnMob(int index);
    
    // Update all mobs
    void Update(float deltaTime, const Vec3& playerPos, float dayNightBlend);
    
    // Get mobs
    std::vector<Mob>& GetMobs() { return mobs; }
    const std::vector<Mob>& GetMobs() const { return mobs; }
    
    // Damage mob
    void DamageMob(int index, float damage);
    void KillMob(int index);
    
    // Spawn rules
    void UpdateSpawning(const Vec3& playerPos, float dayNightBlend);
    void SetSpawnRadius(float minRadius, float maxRadius);
    void SetMaxMobs(int maxCount);

private:
    std::vector<MobDef> definitions;
    std::vector<Mob> mobs;
    float minSpawnRadius;
    float maxSpawnRadius;
    int maxMobCount;
    float spawnTimer;
    
    void UpdatePassiveMob(Mob& mob, float deltaTime);
    void UpdateHostileMob(Mob& mob, float deltaTime, const Vec3& playerPos);
    void UpdateWandering(Mob& mob, float deltaTime);
    void UpdateMovement(Mob& mob, float deltaTime);
    bool CanSpawnHere(const Vec3& pos);
};

} // namespace vge
