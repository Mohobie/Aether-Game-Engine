#include "entity_manager.h"
namespace aether {
EntityManager::EntityManager() {}
Entity* EntityManager::createEntity() {
    EntityID id = nextID++;
    entities[id] = std::make_unique<Entity>(id);
    return entities[id].get();
}
void EntityManager::destroyEntity(EntityID id) {
    entities.erase(id);
}
Entity* EntityManager::getEntity(EntityID id) {
    auto it = entities.find(id);
    return it != entities.end() ? it->second.get() : nullptr;
}
size_t EntityManager::getEntityCount() const { return entities.size(); }
void EntityManager::clear() { entities.clear(); nextID = 1; }
} // namespace aether
