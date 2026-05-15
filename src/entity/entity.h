#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include "math/vec3.h"

namespace vge {

// Forward declarations
class Entity;
class Component;
class EntityManager;

// Unique entity ID
using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = 0;

// ============================================
// Component Base Class
// ============================================
class Component {
public:
    Entity* owner;
    
    Component() : owner(nullptr) {}
    virtual ~Component() = default;
    
    // Called when component is added to entity
    virtual void OnAttach() {}
    
    // Called when component is removed
    virtual void OnDetach() {}
    
    // Update called every frame
    virtual void Update(float deltaTime) { (void)deltaTime; }
    
    // Called when entity is initialized
    virtual void Init() {}
};

// ============================================
// Entity - Container for components
// ============================================
class Entity {
private:
    EntityID id;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
    bool active;
    std::string name;
    
public:
    Entity(EntityID id, const std::string& name = std::string("Entity"));
    ~Entity();
    
    EntityID GetID() const { return id; }
    const std::string& GetName() const { return name; }
    void SetName(const std::string& n) { name = n; }
    
    bool IsActive() const { return active; }
    void SetActive(bool a) { active = a; }
    
    // Component management
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = comp.get();
        comp->owner = this;
        components[std::type_index(typeid(T))] = std::move(comp);
        ptr->OnAttach();
        ptr->Init();
        return ptr;
    }
    
    template<typename T>
    T* GetComponent() {
        auto it = components.find(std::type_index(typeid(T)));
        if (it != components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }
    
    template<typename T>
    bool HasComponent() {
        return components.find(std::type_index(typeid(T))) != components.end();
    }
    
    template<typename T>
    void RemoveComponent() {
        auto it = components.find(std::type_index(typeid(T)));
        if (it != components.end()) {
            it->second->OnDetach();
            components.erase(it);
        }
    }
    
    void Update(float deltaTime);
    
    // Get all components
    std::vector<Component*> GetAllComponents() const;
};

// ============================================
// Entity Manager - Factory and container
// ============================================
class EntityManager {
private:
    std::unordered_map<EntityID, std::unique_ptr<Entity>> entities;
    EntityID nextID;
    
public:
    EntityManager();
    ~EntityManager();
    
    // Create a new entity
    Entity* CreateEntity(const std::string& name = std::string("Entity"));
    
    // Destroy an entity
    void DestroyEntity(EntityID id);
    void DestroyEntity(Entity* entity);
    
    // Get entity by ID
    Entity* GetEntity(EntityID id);
    
    // Get all entities
    std::vector<Entity*> GetAllEntities() const;
    
    // Get entities with specific component
    template<typename T>
    std::vector<Entity*> GetEntitiesWithComponent() {
        std::vector<Entity*> result;
        for (auto& [id, entity] : entities) {
            if (entity->template HasComponent<T>()) {
                result.push_back(entity.get());
            }
        }
        return result;
    }
    
    // Update all entities
    void Update(float deltaTime);
    
    // Clear all entities
    void Clear();
    
    // Get count
    size_t GetEntityCount() const { return entities.size(); }
};

} // namespace vge
