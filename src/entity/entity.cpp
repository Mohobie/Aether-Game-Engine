#include "entity.h"
#include <iostream>

namespace vge {

// ============================================
// Entity Implementation
// ============================================

Entity::Entity(EntityID id, const std::string& name) 
    : id(id), active(true), name(name) {}

Entity::~Entity() {
    // Components will be automatically cleaned up by unique_ptr
}

void Entity::Update(float deltaTime) {
    if (!active) return;
    
    for (auto& [type, comp] : components) {
        comp->Update(deltaTime);
    }
}

std::vector<Component*> Entity::GetAllComponents() const {
    std::vector<Component*> result;
    for (const auto& [type, comp] : components) {
        result.push_back(comp.get());
    }
    return result;
}

// ============================================
// Entity Manager Implementation
// ============================================

EntityManager::EntityManager() : nextID(1) {}

EntityManager::~EntityManager() {
    Clear();
}

Entity* EntityManager::CreateEntity(const std::string& name) {
    EntityID id = nextID++;
    auto entity = std::make_unique<Entity>(id, name);
    Entity* ptr = entity.get();
    entities[id] = std::move(entity);
    std::cout << "[EntityManager] Created entity '" << name << "' (ID: " << id << ")" << std::endl;
    return ptr;
}

void EntityManager::DestroyEntity(EntityID id) {
    auto it = entities.find(id);
    if (it != entities.end()) {
        std::cout << "[EntityManager] Destroyed entity '" << it->second->GetName() << "' (ID: " << id << ")" << std::endl;
        entities.erase(it);
    }
}

void EntityManager::DestroyEntity(Entity* entity) {
    if (entity) {
        DestroyEntity(entity->GetID());
    }
}

Entity* EntityManager::GetEntity(EntityID id) {
    auto it = entities.find(id);
    if (it != entities.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Entity*> EntityManager::GetAllEntities() const {
    std::vector<Entity*> result;
    for (const auto& [id, entity] : entities) {
        result.push_back(entity.get());
    }
    return result;
}

void EntityManager::Update(float deltaTime) {
    for (auto& [id, entity] : entities) {
        entity->Update(deltaTime);
    }
}

void EntityManager::Clear() {
    entities.clear();
    nextID = 1;
}

} // namespace vge
