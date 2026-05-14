#include "lava_damage.h"
#include "voxel/block_registry.h"
#include "core/logger.h"

namespace vge {

LavaDamageSystem::LavaDamageSystem(World* worldPtr)
    : world(worldPtr),
      inLava(false),
      timeInLava(0.0f),
      damageTimer(0.0f),
      damagePerSecond(4.0f),
      damageInterval(0.5f) {
}

LavaDamageSystem::~LavaDamageSystem() {
}

void LavaDamageSystem::Update(const Vec3& playerPosition, float deltaTime) {
    bool wasInLava = inLava;
    inLava = CheckIfInLava(playerPosition);
    
    if (inLava) {
        timeInLava += deltaTime;
        damageTimer += deltaTime;
        
        // Apply damage at intervals
        if (damageTimer >= damageInterval) {
            float damage = damagePerSecond * damageInterval;
            
            Logger::Info("[LavaDamage] Taking " + std::to_string(damage) + 
                       " lava damage (time in lava: " + std::to_string(timeInLava) + ")");
            
            if (onLavaDamage) {
                onLavaDamage(damage);
            }
            
            damageTimer = 0.0f;
        }
    } else {
        if (wasInLava) {
            // Just exited lava
            Logger::Info("[LavaDamage] Exited lava after " + std::to_string(timeInLava) + " seconds");
        }
        timeInLava = 0.0f;
        damageTimer = 0.0f;
    }
}

bool LavaDamageSystem::CheckIfInLava(const Vec3& position) {
    if (!world) return false;
    
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID lavaId = registry.GetBlockId("lava");
    if (lavaId == BLOCK_AIR) return false;
    
    // Check block at player position (feet)
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    int z = static_cast<int>(position.z);
    
    BlockTypeID block = world->GetBlock(x, y, z);
    if (block == lavaId) return true;
    
    // Also check one block below (standing in lava)
    block = world->GetBlock(x, y - 1, z);
    if (block == lavaId) return true;
    
    return false;
}

} // namespace vge
