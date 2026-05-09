#pragma once
#include "entity.h"
#include <unordered_map>
#include <memory>
namespace aether {
class EntityManager {
public:
    EntityManager();
    Entity* createEntity();
    void destroyEntity(EntityID id);
    Entity* getEntity(EntityID id);
    size_t getEntityCount() const;
    void clear();
private:
    std::unordered_map<EntityID, std::unique_ptr<Entity>> entities;
    EntityID nextID = 1;
};
} // namespace aether
