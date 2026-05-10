#pragma once
#include "math/vec3.h"
#include "voxel/block_types.h"
#include <cstdint>
#include <vector>

namespace vge {

// ============================================
// Entity Spawner - Spawn entities in the world
// ============================================
struct SpawnedEntity {
    uint32_t id;
    std::string type;
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    bool active;
    
    SpawnedEntity() : id(0), active(false), scale(1,1,1) {}
};

class EntitySpawner {
private:
    uint32_t nextId;
    std::vector<SpawnedEntity> entities;
    
public:
    EntitySpawner();
    ~EntitySpawner();
    
    // Spawn entity at position
    uint32_t SpawnEntity(const std::string& type, const Vec3& position);
    uint32_t SpawnEntity(const std::string& type, const Vec3& position, const Vec3& rotation);
    uint32_t SpawnEntity(const std::string& type, const Vec3& position, const Vec3& rotation, const Vec3& scale);
    
    // Remove entity
    bool RemoveEntity(uint32_t id);
    void ClearAllEntities();
    
    // Get entity
    SpawnedEntity* GetEntity(uint32_t id);
    const std::vector<SpawnedEntity>& GetAllEntities() const { return entities; }
    
    // Entity manipulation
    bool SetEntityPosition(uint32_t id, const Vec3& position);
    bool SetEntityRotation(uint32_t id, const Vec3& rotation);
    bool SetEntityScale(uint32_t id, const Vec3& scale);
    
    // Bulk operations
    std::vector<uint32_t> SpawnEntitiesInArea(const std::string& type, const Vec3& center, float radius, int count);
    std::vector<uint32_t> SpawnEntitiesInGrid(const std::string& type, const Vec3& start, int width, int depth, float spacing);
    
    // Serialization
    std::string SerializeEntities() const;
    bool DeserializeEntities(const std::string& data);
};

} // namespace vge
