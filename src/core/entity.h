#pragma once
#include <vector>
#include <memory>
#include <string>
#include "voxel/block.h"
#include "math/vec3.h"

namespace vge {

// Forward declarations
class Entity;
class Component;

// Base component class
class Component {
public:
    virtual ~Component() = default;
    virtual void Update(float deltaTime) {}
    virtual void OnAttach(Entity* owner) { this->owner = owner; }
    
    Entity* GetOwner() const { return owner; }
    
protected:
    Entity* owner = nullptr;
};

// Entity class - container for components
class Entity {
public:
    Entity(const std::string& name = "Entity") : name(name), active(true) {}
    ~Entity() = default;
    
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = comp.get();
        comp->OnAttach(this);
        components.push_back(std::move(comp));
        return ptr;
    }
    
    template<typename T>
    T* GetComponent() {
        for (auto& comp : components) {
            T* casted = dynamic_cast<T*>(comp.get());
            if (casted) return casted;
        }
        return nullptr;
    }
    
    void Update(float deltaTime) {
        if (!active) return;
        for (auto& comp : components) {
            comp->Update(deltaTime);
        }
    }
    
    void SetPosition(const Vec3& pos) { position = pos; }
    Vec3 GetPosition() const { return position; }
    void Render(); // Declaration only
    
    // Factory methods
    static Entity* CreatePlayer(const Vec3& position);
    static Entity* CreateBlockEntity(const Vec3& position, BlockType type);
    static Entity* CreateCameraEntity(const Vec3& position);
    
    void SetActive(bool active) { this->active = active; }
    bool IsActive() const { return active; }
    
    const std::string& GetName() const { return name; }
    
private:
    std::string name;
    Vec3 position;
    bool active;
    std::vector<std::unique_ptr<Component>> components;
};

// Transform component
class TransformComponent : public Component {
public:
    Vec3 position;
    Vec3 rotation; // Euler angles in degrees
    Vec3 scale;
    
    TransformComponent() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1) {}
    
    void Translate(const Vec3& offset) { position = position + offset; }
    void Rotate(const Vec3& angles) { rotation = rotation + angles; }
};

// Physics component
class PhysicsComponent : public Component {
public:
    Vec3 velocity;
    Vec3 acceleration;
    float mass;
    bool useGravity;
    bool isStatic;
    
    PhysicsComponent() : velocity(0, 0, 0), acceleration(0, 0, 0), 
                         mass(1.0f), useGravity(true), isStatic(false) {}
    
    void Update(float deltaTime) override {
        if (isStatic) return;
        
        if (useGravity) {
            acceleration = acceleration + Vec3(0, -9.81f, 0);
        }
        
        velocity = velocity + (acceleration * deltaTime);
        
        if (owner) {
            TransformComponent* transform = owner->GetComponent<TransformComponent>();
            if (transform) {
                transform->Translate(velocity * deltaTime);
            }
        }
        
        acceleration = Vec3(0, 0, 0); // Reset
    }
    
    void ApplyForce(const Vec3& force) {
        acceleration = acceleration + (force * (1.0f / mass));
    }
};

// Render component (stub - would reference mesh/material)
class RenderComponent : public Component {
public:
    Vec3 color;
    bool visible;
    
    RenderComponent() : color(1.0f, 1.0f, 1.0f), visible(true) {}
};

} // namespace vge