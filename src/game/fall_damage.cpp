#include "fall_damage.h"
#include "core/logger.h"
#include <cmath>

namespace vge {

FallDamageSystem::FallDamageSystem()
    : highestPoint(0.0f),
      fallDistance(0.0f),
      falling(false),
      wasOnGround(true),
      minFallDistance(3.0f),
      damagePerBlock(1.0f),
      maxDamage(20.0f) {
}

FallDamageSystem::~FallDamageSystem() {
}

void FallDamageSystem::Update(const Vec3& currentPosition, bool isOnGround) {
    // Track highest point when not on ground
    if (!isOnGround) {
        if (!falling) {
            // Started falling
            falling = true;
            highestPoint = currentPosition.y;
        } else {
            // Update highest point if we go higher (jumping)
            if (currentPosition.y > highestPoint) {
                highestPoint = currentPosition.y;
            }
        }
    } else {
        // Landed
        if (falling) {
            // Calculate fall distance
            fallDistance = highestPoint - currentPosition.y;
            
            // Apply damage if fell far enough
            if (fallDistance > minFallDistance) {
                float damage = (fallDistance - minFallDistance) * damagePerBlock;
                if (damage > maxDamage) {
                    damage = maxDamage;
                }
                
                Logger::Info("[FallDamage] Fell " + std::to_string(fallDistance) + 
                           " blocks, taking " + std::to_string(damage) + " damage");
                
                if (onFallDamage) {
                    onFallDamage(damage);
                }
            }
            
            falling = false;
            fallDistance = 0.0f;
        }
    }
    
    wasOnGround = isOnGround;
    lastPosition = currentPosition;
}

} // namespace vge
