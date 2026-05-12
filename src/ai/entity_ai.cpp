#include "entity_ai.h"
#include "core/logger.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <filesystem>

namespace vge {

// ============================================
// Entity Archetype Registry
// ============================================
EntityArchetypeRegistry* EntityArchetypeRegistry::instance = nullptr;

EntityArchetypeRegistry* EntityArchetypeRegistry::GetInstance() {
    if (!instance) {
        instance = new EntityArchetypeRegistry();
    }
    return instance;
}

void EntityArchetypeRegistry::RegisterArchetype(const EntityArchetype& archetype) {
    auto copy = std::make_unique<EntityArchetype>(archetype);
    archetypes[archetype.id] = std::move(copy);
    Logger::Info("[EntityArchetype] Registered: " + archetype.id);
}

void EntityArchetypeRegistry::RegisterArchetype(std::unique_ptr<EntityArchetype> archetype) {
    std::string id = archetype->id;
    archetypes[id] = std::move(archetype);
    Logger::Info("[EntityArchetype] Registered: " + id);
}

const EntityArchetype* EntityArchetypeRegistry::GetArchetype(const std::string& id) const {
    auto it = archetypes.find(id);
    if (it != archetypes.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool EntityArchetypeRegistry::HasArchetype(const std::string& id) const {
    return archetypes.find(id) != archetypes.end();
}

std::vector<std::string> EntityArchetypeRegistry::GetAllArchetypeIds() const {
    std::vector<std::string> ids;
    for (const auto& pair : archetypes) {
        ids.push_back(pair.first);
    }
    return ids;
}

std::unique_ptr<AIEntity> EntityArchetypeRegistry::CreateEntity(const std::string& archetypeId, uint32_t entityId, const Vec3& position) const {
    const EntityArchetype* arch = GetArchetype(archetypeId);
    if (!arch) {
        Logger::Error("[EntityArchetype] Unknown archetype: " + archetypeId);
        return nullptr;
    }
    
    auto entity = std::make_unique<AIEntity>(entityId, archetypeId, position);
    entity->health = arch->health;
    entity->maxHealth = arch->health;
    entity->speed = arch->speed;
    entity->damage = arch->damage;
    
    return entity;
}

bool EntityArchetypeRegistry::LoadFromFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::Error("[EntityArchetype] Failed to open file: " + path);
            return false;
        }
        
        nlohmann::json j;
        file >> j;
        
        EntityArchetype archetype;
        archetype.id = j.value("id", "");
        archetype.displayName = j.value("displayName", archetype.id);
        archetype.health = j.value("health", 100.0f);
        archetype.speed = j.value("speed", 5.0f);
        archetype.damage = j.value("damage", 10.0f);
        archetype.attackRange = j.value("attackRange", 2.0f);
        archetype.attackCooldown = j.value("attackCooldown", 1.0f);
        archetype.detectionRange = j.value("detectionRange", 20.0f);
        
        // Behavior
        std::string behaviorStr = j.value("behavior", "passive");
        if (behaviorStr == "passive") archetype.behavior = AIBehaviorType::Passive;
        else if (behaviorStr == "neutral") archetype.behavior = AIBehaviorType::Neutral;
        else if (behaviorStr == "aggressive") archetype.behavior = AIBehaviorType::Aggressive;
        else if (behaviorStr == "territorial") archetype.behavior = AIBehaviorType::Territorial;
        else if (behaviorStr == "fleeing") archetype.behavior = AIBehaviorType::Fleeing;
        else if (behaviorStr == "boss") archetype.behavior = AIBehaviorType::Boss;
        
        // Spawn conditions
        archetype.minLightLevel = j.value("minLightLevel", 0);
        archetype.maxLightLevel = j.value("maxLightLevel", 15);
        archetype.minTimeOfDay = j.value("minTimeOfDay", 0.0f);
        archetype.maxTimeOfDay = j.value("maxTimeOfDay", 1.0f);
        archetype.spawnsOnGround = j.value("spawnsOnGround", true);
        archetype.spawnsInWater = j.value("spawnsInWater", false);
        archetype.spawnsInAir = j.value("spawnsInAir", false);
        
        // Environmental effects
        archetype.damagedBySunlight = j.value("damagedBySunlight", false);
        archetype.damagedByWater = j.value("damagedByWater", false);
        archetype.canClimbWalls = j.value("canClimbWalls", false);
        archetype.canFly = j.value("canFly", false);
        archetype.canSwim = j.value("canSwim", false);
        
        // Visual/Audio
        archetype.modelId = j.value("modelId", "");
        archetype.textureId = j.value("textureId", "");
        archetype.scale = j.value("scale", 1.0f);
        archetype.ambientSoundId = j.value("ambientSoundId", "");
        archetype.hurtSoundId = j.value("hurtSoundId", "");
        archetype.deathSoundId = j.value("deathSoundId", "");
        archetype.attackSoundId = j.value("attackSoundId", "");
        archetype.lootTableId = j.value("lootTableId", "");
        
        // Custom properties
        if (j.contains("customFloats")) {
            for (auto& [key, value] : j["customFloats"].items()) {
                archetype.customFloats[key] = value.get<float>();
            }
        }
        if (j.contains("customStrings")) {
            for (auto& [key, value] : j["customStrings"].items()) {
                archetype.customStrings[key] = value.get<std::string>();
            }
        }
        if (j.contains("customBools")) {
            for (auto& [key, value] : j["customBools"].items()) {
                archetype.customBools[key] = value.get<bool>();
            }
        }
        
        RegisterArchetype(archetype);
        Logger::Info("[EntityArchetype] Loaded archetype from JSON: " + archetype.id);
        return true;
    } catch (const std::exception& e) {
        Logger::Error("[EntityArchetype] Failed to load JSON: " + std::string(e.what()));
        return false;
    }
}

bool EntityArchetypeRegistry::LoadFromDirectory(const std::string& dirPath) {
    bool anyLoaded = false;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                if (LoadFromFile(entry.path().string())) {
                    anyLoaded = true;
                }
            }
        }
    } catch (const std::exception& e) {
        Logger::Error("[EntityArchetype] Failed to read directory: " + std::string(e.what()));
    }
    
    return anyLoaded;
}

void EntityArchetypeRegistry::Clear() {
    archetypes.clear();
}

// ============================================
// Entity AI Controller
// ============================================
EntityAIController::EntityAIController(AIEntity* ent, const EntityArchetype* arch)
    : entity(ent), archetype(arch) {}

void EntityAIController::Update(float deltaTime, const Vec3& targetPos, const NavigationMesh* navMesh) {
    if (!entity || !entity->alive || !entity->active) return;
    
    // Update timers
    entity->attackCooldownTimer -= deltaTime;
    entity->stateTimer += deltaTime;
    entity->pathUpdateTimer -= deltaTime;
    
    // Environmental damage
    if (archetype->damagedBySunlight && !entity->inCave && !entity->inWater) {
        // Check if entity is exposed to sky (simple check: block above is air)
        // In a full implementation, this would check the light system for skylight
        TakeDamage(2.0f * deltaTime); // Burn damage
    }
    if (archetype->damagedByWater && entity->inWater) {
        TakeDamage(1.0f * deltaTime); // Drowning damage
    }
    
    // State machine
    switch (entity->state) {
        case AIState::IDLE:
            UpdateIdle(deltaTime, targetPos);
            break;
        case AIState::WANDER:
            UpdateWander(deltaTime);
            break;
        case AIState::CHASE:
            UpdateChase(deltaTime, targetPos, navMesh);
            break;
        case AIState::ATTACK:
            UpdateAttack(deltaTime, targetPos);
            break;
        case AIState::FLEE:
            UpdateFlee(deltaTime, targetPos);
            break;
        case AIState::DEAD:
            UpdateDead(deltaTime);
            break;
        default:
            break;
    }
}

void EntityAIController::SetState(AIState newState) {
    if (!entity || entity->state == newState) return;
    
    AIState oldState = entity->state;
    entity->state = newState;
    entity->stateTimer = 0.0f;
    
    if (onStateChange) {
        onStateChange(oldState, newState);
    }
}

void EntityAIController::TakeDamage(float amount) {
    if (!entity || !entity->alive) return;
    
    entity->health -= amount;
    if (onDamageTaken) {
        onDamageTaken(amount);
    }
    
    if (entity->health <= 0.0f) {
        Die();
    } else if (archetype->behavior == AIBehaviorType::Fleeing || 
               archetype->behavior == AIBehaviorType::Passive) {
        SetState(AIState::FLEE);
    }
}

void EntityAIController::Heal(float amount) {
    if (!entity || !entity->alive) return;
    entity->health = std::min(entity->health + amount, entity->maxHealth);
}

void EntityAIController::Die() {
    if (!entity) return;
    entity->health = 0.0f;
    entity->alive = false;
    SetState(AIState::DEAD);
    
    if (onDeath) {
        onDeath();
    }
}

// ============================================
// State Handlers
// ============================================
void EntityAIController::UpdateIdle(float deltaTime, const Vec3& targetPos) {
    entity->idleTimer += deltaTime;
    
    // Behavior-specific transitions
    switch (archetype->behavior) {
        case AIBehaviorType::Aggressive:
        case AIBehaviorType::Territorial:
            if (ShouldChase(targetPos)) {
                SetState(AIState::CHASE);
                return;
            }
            break;
        case AIBehaviorType::Passive:
            if (ShouldChase(targetPos)) {
                SetState(AIState::FLEE);
                return;
            }
            break;
        default:
            break;
    }
    
    // Transition to wander after idle time
    if (entity->idleTimer > 3.0f) {
        SetState(AIState::WANDER);
        entity->idleTimer = 0.0f;
    }
}

void EntityAIController::UpdateWander(float deltaTime) {
    Wander(deltaTime);
    
    // Return to idle after wandering
    if (entity->stateTimer > 5.0f) {
        SetState(AIState::IDLE);
    }
}

void EntityAIController::UpdateChase(float deltaTime, const Vec3& targetPos, const NavigationMesh* navMesh) {
    if (ShouldAttack(targetPos)) {
        SetState(AIState::ATTACK);
        return;
    }
    
    if (ShouldLoseInterest(targetPos)) {
        SetState(AIState::IDLE);
        return;
    }
    
    // Update path to target
    if (entity->pathUpdateTimer <= 0.0f) {
        UpdatePath(targetPos, navMesh);
        entity->pathUpdateTimer = 0.5f;
    }
    
    FollowPath(deltaTime);
}

void EntityAIController::UpdateAttack(float deltaTime, const Vec3& targetPos) {
    if (!ShouldAttack(targetPos)) {
        SetState(AIState::CHASE);
        return;
    }
    
    if (CanAttack()) {
        // Perform attack
        if (onAttack) {
            onAttack();
        }
        ResetAttackCooldown();
    }
}

void EntityAIController::UpdateFlee(float deltaTime, const Vec3& threatPos) {
    FleeFrom(threatPos, deltaTime);
    
    // Stop fleeing after some time
    if (entity->stateTimer > 5.0f) {
        SetState(AIState::IDLE);
    }
}

void EntityAIController::UpdateDead(float deltaTime) {
    // Could play death animation, drop loot, etc.
    // For now, just mark inactive after a delay
    if (entity->stateTimer > 5.0f) {
        entity->active = false;
    }
}

// ============================================
// Transitions
// ============================================
bool EntityAIController::ShouldChase(const Vec3& targetPos) const {
    float dist = (targetPos - entity->position).length();
    return dist < archetype->detectionRange;
}

bool EntityAIController::ShouldAttack(const Vec3& targetPos) const {
    float dist = (targetPos - entity->position).length();
    return dist < archetype->attackRange;
}

bool EntityAIController::ShouldFlee() const {
    return entity->health < entity->maxHealth * 0.3f;
}

bool EntityAIController::ShouldLoseInterest(const Vec3& targetPos) const {
    float dist = (targetPos - entity->position).length();
    return dist > archetype->detectionRange * 1.5f;
}

// ============================================
// Movement
// ============================================
void EntityAIController::MoveToward(const Vec3& target, float deltaTime) {
    Vec3 direction = target - entity->position;
    float dist = direction.length();
    
    if (dist > 0.1f) {
        direction = direction / dist; // Normalize
        entity->velocity = direction * archetype->speed;
        entity->position = entity->position + entity->velocity * deltaTime;
    }
}

void EntityAIController::Wander(float deltaTime) {
    // Simple random wander
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
    Vec3 wanderDir(std::cos(angle), 0.0f, std::sin(angle));
    entity->position = entity->position + wanderDir * archetype->speed * 0.5f * deltaTime;
}

void EntityAIController::FleeFrom(const Vec3& threat, float deltaTime) {
    Vec3 fleeDir = entity->position - threat;
    float dist = fleeDir.length();
    
    if (dist > 0.1f) {
        fleeDir = fleeDir / dist;
        entity->velocity = fleeDir * archetype->speed * 1.5f; // Sprint when fleeing
        entity->position = entity->position + entity->velocity * deltaTime;
    }
}

// ============================================
// Pathfinding
// ============================================
void EntityAIController::UpdatePath(const Vec3& target, const NavigationMesh* navMesh) {
    if (!navMesh) return;
    auto waypoints = navMesh->FindPath(entity->position, target);
    entity->currentPath.waypoints = waypoints;
    entity->currentPath.currentWaypoint = 0;
    entity->currentPath.completed = false;
}

void EntityAIController::FollowPath(float deltaTime) {
    if (entity->currentPath.waypoints.empty()) return;
    
    Vec3 nextWaypoint = entity->currentPath.waypoints[0];
    float dist = (nextWaypoint - entity->position).length();
    
    if (dist < 0.5f) {
        entity->currentPath.waypoints.erase(entity->currentPath.waypoints.begin());
    } else {
        MoveToward(nextWaypoint, deltaTime);
    }
}

} // namespace vge
