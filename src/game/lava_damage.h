#pragma once
#include "math/vec3.h"
#include "voxel/world.h"
#include <functional>

namespace vge {

// ============================================
// Lava Damage System
// ============================================
class LavaDamageSystem {
public:
    LavaDamageSystem(World* world);
    ~LavaDamageSystem();

    // Update (call every frame)
    void Update(const Vec3& playerPosition, float deltaTime);
    
    // Configuration
    void SetDamagePerSecond(float damage) { damagePerSecond = damage; }
    void SetDamageInterval(float interval) { damageInterval = interval; }
    
    // Getters
    bool IsInLava() const { return inLava; }
    float GetTimeInLava() const { return timeInLava; }
    
    // Callback
    std::function<void(float)> onLavaDamage;

private:
    World* world;
    bool inLava;
    float timeInLava;
    float damageTimer;
    float damagePerSecond;
    float damageInterval;
    
    bool CheckIfInLava(const Vec3& position);
};

} // namespace vge
