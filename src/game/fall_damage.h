#pragma once
#include "math/vec3.h"
#include <functional>

namespace vge {

// ============================================
// Fall Damage System
// ============================================
class FallDamageSystem {
public:
    FallDamageSystem();
    ~FallDamageSystem();

    // Update tracking (call every frame)
    void Update(const Vec3& currentPosition, bool isOnGround);
    
    // Configure
    void SetMinFallDistance(float distance) { minFallDistance = distance; }
    void SetDamagePerBlock(float damage) { damagePerBlock = damage; }
    void SetMaxDamage(float maxDmg) { maxDamage = maxDmg; }
    
    // Getters
    float GetFallDistance() const { return fallDistance; }
    bool IsFalling() const { return falling; }
    
    // Callback
    std::function<void(float)> onFallDamage;

private:
    Vec3 lastPosition;
    float highestPoint;
    float fallDistance;
    bool falling;
    bool wasOnGround;
    
    float minFallDistance;
    float damagePerBlock;
    float maxDamage;
};

} // namespace vge
