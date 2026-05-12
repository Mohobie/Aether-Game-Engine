#pragma once
#include "math/vec3.h"
#include "ai/ai_system.h"
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>

namespace vge {

// ============================================
// AI Behavior Types (Generic)
// ============================================
enum class AIBehaviorType {
    Passive,        // Runs from player
    Neutral,        // Attacks only when provoked
    Aggressive,     // Attacks on sight
    Territorial,    // Defends an area
    Fleeing,        // Runs away when damaged
    Boss            // Special attack patterns
};

// ============================================
// AI State Machine
// ============================================
enum class AIState {
    IDLE,
    WANDER,
    CHASE,
    ATTACK,
    FLEE,
    DEAD,
    CUSTOM_1,       // For game-specific states
    CUSTOM_2,
    CUSTOM_3
};

// ============================================
// Entity Archetype Definition
// ============================================
// Games define their own entity types using this data-driven approach
struct EntityArchetype {
    std::string id;                 // Unique identifier (e.g., "goblin", "dragon")
    std::string displayName;        // Human-readable name
    
    // Stats
    float health = 100.0f;
    float speed = 5.0f;
    float damage = 10.0f;
    float attackRange = 2.0f;
    float attackCooldown = 1.0f;
    float detectionRange = 20.0f;
    
    // Behavior
    AIBehaviorType behavior = AIBehaviorType::Passive;
    
    // Spawn conditions
    int minLightLevel = 0;          // 0-15, max light to spawn
    int maxLightLevel = 15;         // 0-15, min light to spawn
    float minTimeOfDay = 0.0f;      // 0.0-1.0, when entities can spawn
    float maxTimeOfDay = 1.0f;      // 0.0-1.0, when entities can spawn
    bool spawnsOnGround = true;
    bool spawnsInWater = false;
    bool spawnsInAir = false;
    
    // Environmental effects (all optional - game defines what hurts what)
    bool damagedBySunlight = false;   // Takes damage in sunlight
    bool damagedByWater = false;      // Takes damage in water (drowning)
    bool damagedByDarkness = false;   // Takes damage in darkness
    bool healedBySunlight = false;    // Heals in sunlight
    bool healedByWater = false;       // Heals in water
    bool healedByDarkness = false;    // Heals in darkness
    bool canClimbWalls = false;
    bool canFly = false;
    bool canSwim = false;
    
    // Loot table ID
    std::string lootTableId;
    
    // Visual
    std::string modelId;
    std::string textureId;
    float scale = 1.0f;
    
    // Audio
    std::string ambientSoundId;
    std::string hurtSoundId;
    std::string deathSoundId;
    std::string attackSoundId;
    
    // Custom properties (game-specific)
    std::unordered_map<std::string, float> customFloats;
    std::unordered_map<std::string, std::string> customStrings;
    std::unordered_map<std::string, bool> customBools;
};

// ============================================
// Runtime Entity Instance
// ============================================
struct AIEntity {
    uint32_t id = 0;
    std::string archetypeId;
    
    // State
    AIState state = AIState::IDLE;
    bool alive = true;
    bool active = true;
    
    // Stats (can differ from archetype via buffs/debuffs)
    float health = 100.0f;
    float maxHealth = 100.0f;
    float speed = 5.0f;
    float damage = 10.0f;
    
    // Position
    Vec3 position;
    Vec3 velocity;
    Vec3 targetPosition;
    Vec3 homePosition;          // For territorial behavior
    
    // Timing
    float stateTimer = 0.0f;
    float attackCooldownTimer = 0.0f;
    float idleTimer = 0.0f;
    
    // Pathfinding
    AIPath currentPath;
    float pathUpdateTimer = 0.0f;
    
    // Environmental
    bool inCave = false;
    bool inWater = false;
    bool onGround = true;
    
    // Custom state (game-specific)
    std::unordered_map<std::string, float> stateFloats;
    std::unordered_map<std::string, int> stateInts;
    
    AIEntity(uint32_t entityId, const std::string& archId, const Vec3& pos)
        : id(entityId), archetypeId(archId), position(pos), homePosition(pos) {}
};

// ============================================
// Entity AI Controller
// ============================================
class EntityAIController {
private:
    AIEntity* entity;
    const EntityArchetype* archetype;
    
    // State handlers
    void UpdateIdle(float deltaTime, const Vec3& targetPos);
    void UpdateWander(float deltaTime);
    void UpdateChase(float deltaTime, const Vec3& targetPos, const NavigationMesh* navMesh);
    void UpdateAttack(float deltaTime, const Vec3& targetPos);
    void UpdateFlee(float deltaTime, const Vec3& threatPos);
    void UpdateDead(float deltaTime);
    
    // Transitions
    bool ShouldChase(const Vec3& targetPos) const;
    bool ShouldAttack(const Vec3& targetPos) const;
    bool ShouldFlee() const;
    bool ShouldLoseInterest(const Vec3& targetPos) const;
    
    // Movement
    void MoveToward(const Vec3& target, float deltaTime);
    void Wander(float deltaTime);
    void FleeFrom(const Vec3& threat, float deltaTime);
    
    // Pathfinding
    void UpdatePath(const Vec3& target, const NavigationMesh* navMesh);
    void FollowPath(float deltaTime);
    
public:
    EntityAIController(AIEntity* ent, const EntityArchetype* arch);
    
    // Core update - targetPos is usually player position
    void Update(float deltaTime, const Vec3& targetPos, const NavigationMesh* navMesh = nullptr);
    
    // State control
    void SetState(AIState newState);
    AIState GetState() const { return entity->state; }
    
    // Combat
    void TakeDamage(float amount);
    void Heal(float amount);
    void Die();
    bool CanAttack() const { return entity->attackCooldownTimer <= 0.0f; }
    void ResetAttackCooldown() { entity->attackCooldownTimer = archetype->attackCooldown; }
    
    // Queries
    bool IsAlive() const { return entity->alive && entity->health > 0.0f; }
    bool IsActive() const { return entity->active; }
    float GetHealthPercent() const { return entity->health / entity->maxHealth; }
    
    // Callbacks
    std::function<void()> onDeath;
    std::function<void()> onAttack;
    std::function<void(float)> onDamageTaken;
    std::function<void(AIState, AIState)> onStateChange; // old, new
};

// ============================================
// Entity Archetype Registry
// ============================================
class EntityArchetypeRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<EntityArchetype>> archetypes;
    static EntityArchetypeRegistry* instance;
    
public:
    static EntityArchetypeRegistry* GetInstance();
    
    // Registration
    void RegisterArchetype(const EntityArchetype& archetype);
    void RegisterArchetype(std::unique_ptr<EntityArchetype> archetype);
    
    // Queries
    const EntityArchetype* GetArchetype(const std::string& id) const;
    bool HasArchetype(const std::string& id) const;
    std::vector<std::string> GetAllArchetypeIds() const;
    
    // Factory
    std::unique_ptr<AIEntity> CreateEntity(const std::string& archetypeId, uint32_t entityId, const Vec3& position) const;
    
    // Loading
    bool LoadFromFile(const std::string& path);
    bool LoadFromDirectory(const std::string& dirPath);
    void Clear();
};

} // namespace vge
