#include "entity_spawner.h"
#include "core/logger.h"
#include "voxel/block_types.h"
#include <cmath>
#include <algorithm>

namespace vge {

GenericEntitySpawner::GenericEntitySpawner(World* w, LightSystem* lights, DayNightCycle* cycle)
    : world(w), lightSystem(lights), dayNightCycle(cycle), rng(std::random_device{}()) {}

GenericEntitySpawner::~GenericEntitySpawner() {
    DespawnAllEntities();
}

void GenericEntitySpawner::Update(float deltaTime, const Vec3& playerPosition) {
    spawnTimer += deltaTime;
    
    // Periodic spawn attempt
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.0f;
        TrySpawnEntities(playerPosition);
    }
    
    // Update all entities
    for (auto& instance : entities) {
        if (instance->entity && instance->entity->alive) {
            instance->controller->Update(deltaTime, playerPosition, nullptr);
        }
    }
    
    // Despawn distant entities
    DespawnDistantEntities(playerPosition);
    
    // Remove inactive/dead entities
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [this](const std::unique_ptr<SpawnedEntityInstance>& instance) {
                if (!instance->entity || !instance->entity->active) {
                    if (onEntityDespawned) {
                        onEntityDespawned(instance->id, instance->archetypeId);
                    }
                    return true;
                }
                return false;
            }),
        entities.end()
    );
}

SpawnedEntityInstance* GenericEntitySpawner::SpawnEntity(const std::string& archetypeId, const Vec3& position) {
    auto* registry = EntityArchetypeRegistry::GetInstance();
    const EntityArchetype* archetype = registry->GetArchetype(archetypeId);
    
    if (!archetype) {
        Logger::Error("[EntitySpawner] Unknown archetype: " + archetypeId);
        return nullptr;
    }
    
    uint32_t id = nextEntityId++;
    auto entity = registry->CreateEntity(archetypeId, id, position);
    if (!entity) {
        return nullptr;
    }
    
    auto controller = std::make_unique<EntityAIController>(entity.get(), archetype);
    
    auto instance = std::make_unique<SpawnedEntityInstance>(
        id, archetypeId, std::move(entity), std::move(controller)
    );
    
    SpawnedEntityInstance* ptr = instance.get();
    entities.push_back(std::move(instance));
    
    Logger::Info("[EntitySpawner] Spawned " + archetypeId + " at (" + 
                 std::to_string(static_cast<int>(position.x)) + ", " +
                 std::to_string(static_cast<int>(position.y)) + ", " +
                 std::to_string(static_cast<int>(position.z)) + ")");
    
    if (onEntitySpawned) {
        onEntitySpawned(archetypeId, position);
    }
    
    return ptr;
}

void GenericEntitySpawner::DespawnEntity(uint32_t entityId) {
    auto it = std::find_if(entities.begin(), entities.end(),
        [entityId](const std::unique_ptr<SpawnedEntityInstance>& instance) {
            return instance->id == entityId;
        });
    
    if (it != entities.end()) {
        if (onEntityDespawned) {
            onEntityDespawned((*it)->id, (*it)->archetypeId);
        }
        entities.erase(it);
    }
}

void GenericEntitySpawner::DespawnAllEntities() {
    for (auto& instance : entities) {
        if (onEntityDespawned) {
            onEntityDespawned(instance->id, instance->archetypeId);
        }
    }
    entities.clear();
}

void GenericEntitySpawner::DespawnEntitiesByArchetype(const std::string& archetypeId) {
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [this, &archetypeId](const std::unique_ptr<SpawnedEntityInstance>& instance) {
                if (instance->archetypeId == archetypeId) {
                    if (onEntityDespawned) {
                        onEntityDespawned(instance->id, instance->archetypeId);
                    }
                    return true;
                }
                return false;
            }),
        entities.end()
    );
}

size_t GenericEntitySpawner::GetEntityCountByArchetype(const std::string& archetypeId) const {
    return std::count_if(entities.begin(), entities.end(),
        [&archetypeId](const std::unique_ptr<SpawnedEntityInstance>& instance) {
            return instance->archetypeId == archetypeId;
        });
}

SpawnedEntityInstance* GenericEntitySpawner::GetEntity(uint32_t id) const {
    auto it = std::find_if(entities.begin(), entities.end(),
        [id](const std::unique_ptr<SpawnedEntityInstance>& instance) {
            return instance->id == id;
        });
    
    return (it != entities.end()) ? it->get() : nullptr;
}

std::vector<SpawnedEntityInstance*> GenericEntitySpawner::GetEntitiesInRadius(const Vec3& center, float radius) const {
    std::vector<SpawnedEntityInstance*> result;
    float radiusSq = radius * radius;
    
    for (const auto& instance : entities) {
        if (instance->entity) {
            float distSq = (instance->entity->position - center).length();
    distSq = distSq * distSq;
            if (distSq <= radiusSq) {
                result.push_back(instance.get());
            }
        }
    }
    
    return result;
}

std::vector<SpawnedEntityInstance*> GenericEntitySpawner::GetEntitiesByArchetype(const std::string& archetypeId) const {
    std::vector<SpawnedEntityInstance*> result;
    
    for (const auto& instance : entities) {
        if (instance->archetypeId == archetypeId) {
            result.push_back(instance.get());
        }
    }
    
    return result;
}

std::vector<SpawnedEntityInstance*> GenericEntitySpawner::GetAllEntities() const {
    std::vector<SpawnedEntityInstance*> result;
    
    for (const auto& instance : entities) {
        result.push_back(instance.get());
    }
    
    return result;
}

// ============================================
// Internal Helpers
// ============================================
void GenericEntitySpawner::TrySpawnEntities(const Vec3& playerPosition) {
    if (static_cast<int>(entities.size()) >= maxEntities) {
        return;
    }
    
    auto spawnableArchetypes = GetSpawnableArchetypes(playerPosition);
    if (spawnableArchetypes.empty()) {
        return;
    }
    
    // Pick random archetype
    std::uniform_int_distribution<size_t> dist(0, spawnableArchetypes.size() - 1);
    const EntityArchetype* archetype = spawnableArchetypes[dist(rng)];
    
    // Find spawn position
    Vec3 spawnPos = FindSpawnPosition(playerPosition, spawnRadius);
    
    if (CanSpawnArchetypeAt(spawnPos, *archetype)) {
        SpawnEntity(archetype->id, spawnPos);
    }
}

bool GenericEntitySpawner::CanSpawnArchetypeAt(const Vec3& pos, const EntityArchetype& archetype) const {
    // Check light level
    if (lightSystem) {
        int lightLevel = lightSystem->GetTotalLightLevel(
            static_cast<int>(pos.x), 
            static_cast<int>(pos.y), 
            static_cast<int>(pos.z)
        );
        if (lightLevel < archetype.minLightLevel || lightLevel > archetype.maxLightLevel) {
            return false;
        }
    }
    
    // Check position validity
    if (!IsValidSpawnPosition(pos, archetype)) {
        return false;
    }
    
    return true;
}

bool GenericEntitySpawner::IsValidSpawnPosition(const Vec3& pos, const EntityArchetype& archetype) const {
    if (!world) return false;
    
    int x = static_cast<int>(pos.x);
    int y = static_cast<int>(pos.y);
    int z = static_cast<int>(pos.z);
    
    // Check ground
    if (archetype.spawnsOnGround) {
        Block blockBelow = world->GetBlock(x, y - 1, z);
        if (blockBelow.typeId == BLOCK_AIR) {
            return false;
        }
    }
    
    // Check water
    Block blockAt = world->GetBlock(x, y, z);
    if (!archetype.spawnsInWater) {
        const BlockDef& def = BlockRegistry::GetInstance().GetBlock(blockAt.typeId);
        if (def.id == "water") {
            return false;
        }
    }
    
    // Check air (flying entities)
    if (!archetype.canFly && blockAt.typeId != BLOCK_AIR) {
        return false;
    }
    
    return true;
}

Vec3 GenericEntitySpawner::FindSpawnPosition(const Vec3& center, float radius) const {
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> radiusDist(5.0f, radius); // Min 5 units away
    
    float angle = angleDist(rng);
    float r = radiusDist(rng);
    
    Vec3 pos;
    pos.x = center.x + std::cos(angle) * r;
    pos.z = center.z + std::sin(angle) * r;
    pos.y = center.y; // Will be adjusted by terrain
    
    return pos;
}

void GenericEntitySpawner::DespawnDistantEntities(const Vec3& playerPos) {
    float despawnSq = despawnRadius * despawnRadius;
    
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [this, &playerPos, despawnSq](const std::unique_ptr<SpawnedEntityInstance>& instance) {
                if (instance->entity) {
                    float distSq = (instance->entity->position - playerPos).length();
    distSq = distSq * distSq;
                    if (distSq > despawnSq) {
                        if (onEntityDespawned) {
                            onEntityDespawned(instance->id, instance->archetypeId);
                        }
                        return true;
                    }
                }
                return false;
            }),
        entities.end()
    );
}

std::vector<const EntityArchetype*> GenericEntitySpawner::GetSpawnableArchetypes(const Vec3& pos) const {
    std::vector<const EntityArchetype*> result;
    
    auto* registry = EntityArchetypeRegistry::GetInstance();
    auto archetypeIds = registry->GetAllArchetypeIds();
    
    float timeOfDay = 0.5f; // Default to midday
    if (dayNightCycle) {
        timeOfDay = dayNightCycle->GetTimeOfDay();
    }
    
    for (const auto& id : archetypeIds) {
        const EntityArchetype* arch = registry->GetArchetype(id);
        if (!arch) continue;
        
        // Check time of day (handle wrap-around for night spawns)
        bool validTime = false;
        if (arch->minTimeOfDay <= arch->maxTimeOfDay) {
            // Normal range (e.g., 0.2 to 0.8)
            validTime = (timeOfDay >= arch->minTimeOfDay && timeOfDay <= arch->maxTimeOfDay);
        } else {
            // Wrap-around range (e.g., 0.7 to 0.3 means night time)
            validTime = (timeOfDay >= arch->minTimeOfDay || timeOfDay <= arch->maxTimeOfDay);
        }
        if (!validTime) {
            continue;
        }
        
        result.push_back(arch);
    }
    
    return result;
}

} // namespace vge
