#include "game/mob_spawn_rules.h"
#include "core/logger.h"

namespace vge {

void MobSpawnRules::Initialize() {
    Logger::Info("[MobSpawnRules] Initialized");
}

bool MobSpawnRules::CanSpawnMob(const std::string& mobType, int x, int y, int z, World& world) {
    (void)mobType; (void)x; (void)y; (void)z; (void)world;
    return true;
}

} // namespace vge
