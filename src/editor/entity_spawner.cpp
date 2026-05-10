#include "entity_spawner.h"
#include <algorithm>
#include <cmath>

namespace vge {

EntitySpawner::EntitySpawner() : nextId(1) {}
EntitySpawner::~EntitySpawner() {}

uint32_t EntitySpawner::SpawnEntity(const std::string& type, const Vec3& position) {
    return SpawnEntity(type, position, Vec3(0, 0, 0), Vec3(1, 1, 1));
}

uint32_t EntitySpawner::SpawnEntity(const std::string& type, const Vec3& position, const Vec3& rotation) {
    return SpawnEntity(type, position, rotation, Vec3(1, 1, 1));
}

uint32_t EntitySpawner::SpawnEntity(const std::string& type, const Vec3& position, const Vec3& rotation, const Vec3& scale) {
    SpawnedEntity entity;
    entity.id = nextId++;
    entity.type = type;
    entity.position = position;
    entity.rotation = rotation;
    entity.scale = scale;
    entity.active = true;
    entities.push_back(entity);
    return entity.id;
}

bool EntitySpawner::RemoveEntity(uint32_t id) {
    auto it = std::remove_if(entities.begin(), entities.end(),
        [id](const SpawnedEntity& e) { return e.id == id; });
    if (it != entities.end()) {
        entities.erase(it, entities.end());
        return true;
    }
    return false;
}

void EntitySpawner::ClearAllEntities() {
    entities.clear();
    nextId = 1;
}

SpawnedEntity* EntitySpawner::GetEntity(uint32_t id) {
    for (auto& entity : entities) {
        if (entity.id == id && entity.active) {
            return &entity;
        }
    }
    return nullptr;
}

bool EntitySpawner::SetEntityPosition(uint32_t id, const Vec3& position) {
    SpawnedEntity* entity = GetEntity(id);
    if (entity) {
        entity->position = position;
        return true;
    }
    return false;
}

bool EntitySpawner::SetEntityRotation(uint32_t id, const Vec3& rotation) {
    SpawnedEntity* entity = GetEntity(id);
    if (entity) {
        entity->rotation = rotation;
        return true;
    }
    return false;
}

bool EntitySpawner::SetEntityScale(uint32_t id, const Vec3& scale) {
    SpawnedEntity* entity = GetEntity(id);
    if (entity) {
        entity->scale = scale;
        return true;
    }
    return false;
}

std::vector<uint32_t> EntitySpawner::SpawnEntitiesInArea(const std::string& type, const Vec3& center, float radius, int count) {
    std::vector<uint32_t> ids;
    ids.reserve(count);
    for (int i = 0; i < count; ++i) {
        float angle = (float(i) / count) * 2.0f * 3.14159f;
        float dist = radius * (0.5f + 0.5f * (float(i % 7) / 7.0f)); // pseudo-random distribution
        Vec3 pos(
            center.x + std::cos(angle) * dist,
            center.y,
            center.z + std::sin(angle) * dist
        );
        ids.push_back(SpawnEntity(type, pos));
    }
    return ids;
}

std::vector<uint32_t> EntitySpawner::SpawnEntitiesInGrid(const std::string& type, const Vec3& start, int width, int depth, float spacing) {
    std::vector<uint32_t> ids;
    ids.reserve(width * depth);
    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < depth; ++z) {
            Vec3 pos(
                start.x + x * spacing,
                start.y,
                start.z + z * spacing
            );
            ids.push_back(SpawnEntity(type, pos));
        }
    }
    return ids;
}

std::string EntitySpawner::SerializeEntities() const {
    std::string result = "entities:" + std::to_string(entities.size()) + "\n";
    for (const auto& e : entities) {
        result += std::to_string(e.id) + "," + e.type + ","
                + std::to_string(e.position.x) + "," + std::to_string(e.position.y) + "," + std::to_string(e.position.z) + "\n";
    }
    return result;
}

bool EntitySpawner::DeserializeEntities(const std::string& data) {
    (void)data;
    // Simplified deserialization - would parse CSV format
    return false;
}

} // namespace vge
