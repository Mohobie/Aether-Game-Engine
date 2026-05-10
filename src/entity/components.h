#pragma once
#include "entity/entity.h"
#include "math/vec3.h"

namespace vge {

// Transform component - position, rotation, scale
class TransformComponent : public Component {
public:
    Vec3 position;
    Vec3 rotation; // Euler angles in degrees
    Vec3 scale;
    
    TransformComponent() 
        : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1) {}
    
    TransformComponent(const Vec3& pos) 
        : position(pos), rotation(0, 0, 0), scale(1, 1, 1) {}
    
    void Translate(const Vec3& delta) { position = position + delta; }
    void Rotate(const Vec3& delta) { rotation = rotation + delta; }
    void SetScale(const Vec3& s) { scale = s; }
    
    Vec3 GetForward() const;
    Vec3 GetRight() const;
    Vec3 GetUp() const;
};

// Health component
class HealthComponent : public Component {
public:
    float maxHealth;
    float currentHealth;
    
    HealthComponent(float max = 100.0f) 
        : maxHealth(max), currentHealth(max) {}
    
    void TakeDamage(float amount);
    void Heal(float amount);
    bool IsAlive() const { return currentHealth > 0; }
    float GetHealthPercent() const { return currentHealth / maxHealth; }
    
    // Callback for death
    std::function<void()> onDeath;
    std::function<void(float)> onDamage;
};

// Movement component
class MovementComponent : public Component {
public:
    Vec3 velocity;
    Vec3 acceleration;
    float maxSpeed;
    float friction;
    
    MovementComponent(float maxSpeed = 10.0f, float friction = 0.9f)
        : velocity(0, 0, 0), acceleration(0, 0, 0), 
          maxSpeed(maxSpeed), friction(friction) {}
    
    void AddForce(const Vec3& force) { acceleration = acceleration + force; }
    void Update(float deltaTime) override;
    void Stop() { velocity = Vec3(0, 0, 0); }
};

// Script component for custom behavior
class ScriptComponent : public Component {
public:
    std::function<void(float)> updateFunc;
    std::function<void()> initFunc;
    
    ScriptComponent() = default;
    
    void Init() override {
        if (initFunc) initFunc();
    }
    
    void Update(float deltaTime) override {
        if (updateFunc) updateFunc(deltaTime);
    }
};

} // namespace vge
