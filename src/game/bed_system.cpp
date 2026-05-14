#include "game/bed_system.h"
#include "voxel/block_types.h"
#include "voxel/block_registry.h"
#include "core/logger.h"
#include <cmath>
#include <algorithm>

namespace vge {

// ============================================
// Constructor / Destructor
// ============================================
BedSystem::BedSystem(World* w, DayNightCycle* cycle, LightSystem* lights)
    : world(w)
    , dayNightCycle(cycle)
    , lightSystem(lights)
    , activeBed(nullptr)
    , skipSpeed(50.0f)    // 50x speed during skip
    , skipProgress(0.0f)
    , isSkippingNight(false)
    , respawnPoint(0, 20, 0)
    , respawnSet(false) {}

BedSystem::~BedSystem() {
    ClearAllBeds();
}

// ============================================
// Update
// ============================================
void BedSystem::Update(float deltaTime, const Vec3& playerPos) {
    if (!dayNightCycle) return;

    // Handle night skip
    if (isSkippingNight && activeBed) {
        // Advance time rapidly
        float currentTime = dayNightCycle->GetTimeOfDay();
        float dawnTime = 6.0f; // 6:00 AM

        // Calculate how much time to advance
        float timeToDawn = dawnTime - currentTime;
        if (timeToDawn < 0.0f) {
            timeToDawn += 24.0f; // Wrap around midnight
        }

        // Advance time at skip speed
        float advance = deltaTime * skipSpeed;
        if (advance > timeToDawn) {
            advance = timeToDawn;
        }

        dayNightCycle->SetTime(currentTime + advance);
        skipProgress = 1.0f - (timeToDawn - advance) / timeToDawn;

        // Update sky light during skip
        if (lightSystem) {
            lightSystem->UpdateSkyLightForTime(dayNightCycle->GetDayNightBlend());
        }

        // Check if we've reached dawn
        currentTime = dayNightCycle->GetTimeOfDay();
        bool reachedDawn = false;

        // Dawn is between 5:00 and 7:00
        if (currentTime >= 5.0f && currentTime <= 7.0f) {
            reachedDawn = true;
        }

        if (reachedDawn) {
            FinishNightSkip();
        }
    }

    // Update sleep timer
    if (activeBed && !isSkippingNight) {
        activeBed->sleepTimer += deltaTime;

        // After 3 seconds in bed, start skipping
        if (activeBed->sleepTimer >= 3.0f) {
            StartNightSkip();
        }
    }

    // Check if player moved too far from bed while sleeping
    if (activeBed && !isSkippingNight) {
        float dist = (playerPos - activeBed->position).length();
        if (dist > 3.0f) {
            Logger::Info("[BedSystem] Player moved too far from bed - waking up");
            CancelSleep();
        }
    }
}

// ============================================
// Bed Management
// ============================================
bool BedSystem::PlaceBed(int x, int y, int z, int direction) {
    if (!world) return false;

    // Validate direction
    if (direction < 0 || direction > 3) direction = 0;

    // Check if foot position is valid (air or replaceable)
    BlockTypeID footBlock = world->GetBlock(x, y, z);
    if (footBlock != BLOCK_AIR) {
        const BlockDef& def = BlockRegistry::GetInstance().GetBlock(footBlock);
        if (def.IsSolid()) {
            Logger::Error("[BedSystem] Cannot place bed - foot position blocked");
            return false;
        }
    }

    // Calculate head position
    int hx = x, hz = z;
    switch (direction) {
        case 0: hx = x + 1; break;  // +X
        case 1: hz = z + 1; break;  // +Z
        case 2: hx = x - 1; break;  // -X
        case 3: hz = z - 1; break;  // -Z
    }

    // Check if head position is valid
    BlockTypeID headBlock = world->GetBlock(hx, y, hz);
    if (headBlock != BLOCK_AIR) {
        const BlockDef& def = BlockRegistry::GetInstance().GetBlock(headBlock);
        if (def.IsSolid()) {
            Logger::Error("[BedSystem] Cannot place bed - head position blocked");
            return false;
        }
    }

    // Place bed blocks
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID bedId = registry.GetBlockId("bed");
    if (bedId == BLOCK_AIR) {
        // Bed block not registered, use a placeholder
        Logger::Error("[BedSystem] Bed block not registered in block registry");
        return false;
    }

    world->SetBlock(x, y, z, bedId);
    world->SetBlock(hx, y, hz, bedId);

    // Create bed instance
    beds.emplace_back(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), direction);

    Logger::Info("[BedSystem] Bed placed at (" + std::to_string(x) + ", " +
                 std::to_string(y) + ", " + std::to_string(z) + ") facing " +
                 std::to_string(direction));
    return true;
}

bool BedSystem::RemoveBed(int x, int y, int z) {
    if (!world) return false;

    // Find bed at this position
    auto it = std::find_if(beds.begin(), beds.end(),
        [x, y, z](const Bed& bed) {
            int bx = static_cast<int>(std::floor(bed.position.x));
            int by = static_cast<int>(std::floor(bed.position.y));
            int bz = static_cast<int>(std::floor(bed.position.z));
            int hx = static_cast<int>(std::floor(bed.headPosition.x));
            int hz = static_cast<int>(std::floor(bed.headPosition.z));
            return (bx == x && by == y && bz == z) || (hx == x && by == y && hz == z);
        });

    if (it == beds.end()) return false;

    // Remove blocks
    int fx = static_cast<int>(std::floor(it->position.x));
    int fy = static_cast<int>(std::floor(it->position.y));
    int fz = static_cast<int>(std::floor(it->position.z));
    int hx = static_cast<int>(std::floor(it->headPosition.x));
    int hz = static_cast<int>(std::floor(it->headPosition.z));

    world->SetBlock(fx, fy, fz, BLOCK_AIR);
    world->SetBlock(hx, fy, hz, BLOCK_AIR);

    // If player was sleeping in this bed, wake them up
    if (activeBed == &(*it)) {
        CancelSleep();
    }

    beds.erase(it);
    Logger::Info("[BedSystem] Bed removed");
    return true;
}

void BedSystem::ClearAllBeds() {
    if (world) {
        for (const auto& bed : beds) {
            int fx = static_cast<int>(std::floor(bed.position.x));
            int fy = static_cast<int>(std::floor(bed.position.y));
            int fz = static_cast<int>(std::floor(bed.position.z));
            int hx = static_cast<int>(std::floor(bed.headPosition.x));
            int hz = static_cast<int>(std::floor(bed.headPosition.z));

            world->SetBlock(fx, fy, fz, BLOCK_AIR);
            world->SetBlock(hx, fy, hz, BLOCK_AIR);
        }
    }
    beds.clear();
    activeBed = nullptr;
    isSkippingNight = false;
}

// ============================================
// Sleep Mechanic
// ============================================
SleepResult BedSystem::TrySleep(const Vec3& playerPos) {
    // Find nearest bed
    Bed* nearest = nullptr;
    float nearestDist = 9999.0f;

    for (auto& bed : beds) {
        float dist = (playerPos - bed.position).length();
        if (dist < nearestDist && dist <= 2.5f) {
            nearestDist = dist;
            nearest = &bed;
        }
    }

    if (!nearest) {
        return SleepResult::TooFar;
    }

    return TrySleepAt(nearest->position, playerPos);
}

SleepResult BedSystem::TrySleepAt(const Vec3& bedPos, const Vec3& playerPos) {
    if (isSkippingNight) {
        return SleepResult::AlreadySkipping;
    }

    // Check if it's night time
    if (!IsNightTime(dayNightCycle)) {
        return SleepResult::NotNight;
    }

    // Find the bed
    Bed* bed = FindBedAt(bedPos);
    if (!bed) {
        return SleepResult::TooFar;
    }

    // Check distance
    float dist = (playerPos - bed->position).length();
    if (dist > 2.5f) {
        return SleepResult::TooFar;
    }

    // Check if bed is obstructed
    if (IsBedObstructed(*bed)) {
        return SleepResult::Obstructed;
    }

    // Check for monsters nearby
    if (AreMonstersNearby(bed->position)) {
        return SleepResult::MonstersNearby;
    }

    // Start sleeping
    bed->state = BedState::Occupied;
    bed->sleepTimer = 0.0f;
    activeBed = bed;

    // Set respawn point to this bed
    SetRespawnPoint(bed->position);

    Logger::Info("[BedSystem] Player went to sleep at (" +
                 std::to_string(static_cast<int>(bed->position.x)) + ", " +
                 std::to_string(static_cast<int>(bed->position.y)) + ", " +
                 std::to_string(static_cast<int>(bed->position.z)) + ")");

    if (onSleepStarted) {
        onSleepStarted();
    }

    return SleepResult::Success;
}

void BedSystem::WakeUp() {
    if (!activeBed) return;

    Logger::Info("[BedSystem] Player woke up");

    CancelSleep();
}

void BedSystem::StartNightSkip() {
    if (!activeBed || isSkippingNight) return;

    isSkippingNight = true;
    skipProgress = 0.0f;
    activeBed->state = BedState::Skipping;

    // Pause normal time progression
    if (dayNightCycle) {
        dayNightCycle->Pause();
    }

    Logger::Info("[BedSystem] Starting night skip...");
}

void BedSystem::FinishNightSkip() {
    if (!isSkippingNight) return;

    // Set time to exactly dawn
    if (dayNightCycle) {
        dayNightCycle->SetTime(6.0f);
        dayNightCycle->Resume();
    }

    // Update sky light
    if (lightSystem) {
        lightSystem->UpdateSkyLightForTime(1.0f); // Full day
    }

    isSkippingNight = false;
    skipProgress = 1.0f;

    if (activeBed) {
        activeBed->state = BedState::Empty;
        activeBed->sleepTimer = 0.0f;
        activeBed = nullptr;
    }

    Logger::Info("[BedSystem] Night skipped - good morning!");

    if (onSleepEnded) {
        onSleepEnded();
    }
}

void BedSystem::CancelSleep() {
    if (activeBed) {
        activeBed->state = BedState::Empty;
        activeBed->sleepTimer = 0.0f;
        activeBed = nullptr;
    }

    isSkippingNight = false;
    skipProgress = 0.0f;

    // Resume normal time
    if (dayNightCycle) {
        dayNightCycle->Resume();
    }
}

// ============================================
// Spawn Point
// ============================================
void BedSystem::SetRespawnPoint(const Vec3& pos) {
    respawnPoint = pos;
    respawnSet = true;

    Logger::Info("[BedSystem] Respawn point set to (" +
                 std::to_string(static_cast<int>(pos.x)) + ", " +
                 std::to_string(static_cast<int>(pos.y)) + ", " +
                 std::to_string(static_cast<int>(pos.z)) + ")");

    if (onRespawnPointSet) {
        onRespawnPointSet();
    }
}

// ============================================
// Queries
// ============================================
bool BedSystem::IsBedAt(int x, int y, int z) const {
    for (const auto& bed : beds) {
        int bx = static_cast<int>(std::floor(bed.position.x));
        int by = static_cast<int>(std::floor(bed.position.y));
        int bz = static_cast<int>(std::floor(bed.position.z));
        int hx = static_cast<int>(std::floor(bed.headPosition.x));
        int hz = static_cast<int>(std::floor(bed.headPosition.z));

        if ((bx == x && by == y && bz == z) || (hx == x && by == y && hz == z)) {
            return true;
        }
    }
    return false;
}

Bed* BedSystem::FindBedAt(const Vec3& position) {
    int x = static_cast<int>(std::floor(position.x));
    int y = static_cast<int>(std::floor(position.y));
    int z = static_cast<int>(std::floor(position.z));

    for (auto& bed : beds) {
        int bx = static_cast<int>(std::floor(bed.position.x));
        int by = static_cast<int>(std::floor(bed.position.y));
        int bz = static_cast<int>(std::floor(bed.position.z));
        int hx = static_cast<int>(std::floor(bed.headPosition.x));
        int hz = static_cast<int>(std::floor(bed.headPosition.z));

        if ((bx == x && by == y && bz == z) || (hx == x && by == y && hz == z)) {
            return &bed;
        }
    }
    return nullptr;
}

// ============================================
// Sleep Conditions
// ============================================
bool BedSystem::CanSleepAt(const Bed& bed, const Vec3& playerPos) const {
    // Check distance
    float dist = (playerPos - bed.position).length();
    if (dist > 2.5f) return false;

    // Check if night
    if (!IsNightTime(dayNightCycle)) return false;

    // Check obstruction
    if (IsBedObstructed(bed)) return false;

    // Check monsters
    if (AreMonstersNearby(bed.position)) return false;

    return true;
}

bool BedSystem::AreMonstersNearby(const Vec3& position) const {
    // In a full implementation, this would check for hostile mobs
    // For now, we check light level - if it's bright, no monsters
    if (!lightSystem) return false;

    int px = static_cast<int>(std::floor(position.x));
    int py = static_cast<int>(std::floor(position.y));
    int pz = static_cast<int>(std::floor(position.z));

    // Check a radius around the bed for darkness (where monsters could be)
    for (int dx = -8; dx <= 8; ++dx) {
        for (int dy = -4; dy <= 4; ++dy) {
            for (int dz = -8; dz <= 8; ++dz) {
                if (std::abs(dx) + std::abs(dy) + std::abs(dz) > 8) continue;

                int light = lightSystem->GetTotalLightLevel(px + dx, py + dy, pz + dz);
                if (light < 7) {
                    // Dark enough for monsters - assume they could be here
                    // In full implementation, check actual mob positions
                    return true;
                }
            }
        }
    }

    return false;
}

bool BedSystem::IsBedObstructed(const Bed& bed) const {
    if (!world) return true;

    // Check if there's space above both foot and head
    int fx = static_cast<int>(std::floor(bed.position.x));
    int fy = static_cast<int>(std::floor(bed.position.y));
    int fz = static_cast<int>(std::floor(bed.position.z));
    int hx = static_cast<int>(std::floor(bed.headPosition.x));
    int hz = static_cast<int>(std::floor(bed.headPosition.z));

    // Need 2 blocks of vertical space (bed + player)
    for (int y = fy + 1; y <= fy + 2; ++y) {
        BlockTypeID block = world->GetBlock(fx, y, fz);
        if (block != BLOCK_AIR) {
            const BlockDef& def = BlockRegistry::GetInstance().GetBlock(block);
            if (def.IsSolid()) return true;
        }

        block = world->GetBlock(hx, y, hz);
        if (block != BLOCK_AIR) {
            const BlockDef& def = BlockRegistry::GetInstance().GetBlock(block);
            if (def.IsSolid()) return true;
        }
    }

    return false;
}

// ============================================
// Static Helpers
// ============================================
bool BedSystem::IsNightTime(DayNightCycle* cycle) {
    if (!cycle) return false;
    return cycle->IsNight();
}

std::string BedSystem::SleepResultToString(SleepResult result) {
    switch (result) {
        case SleepResult::Success:         return "Success";
        case SleepResult::NotNight:        return "You can only sleep at night";
        case SleepResult::TooFar:          return "You may not rest now, the bed is too far away";
        case SleepResult::Obstructed:      return "This bed is obstructed";
        case SleepResult::MonstersNearby:  return "You may not rest now, there are monsters nearby";
        case SleepResult::AlreadySkipping: return "Night is already being skipped";
    }
    return "Unknown";
}

} // namespace vge
