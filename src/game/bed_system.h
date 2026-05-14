#pragma once
#include "math/vec3.h"
#include "voxel/world.h"
#include "rendering/sky/day_night_cycle.h"
#include "rendering/light_system.h"
#include "core/player_controller.h"
#include <vector>
#include <string>
#include <functional>

namespace vge {

// ============================================
// Bed State
// ============================================
enum class BedState {
    Empty,      // No one sleeping
    Occupied,   // Player is in bed
    Skipping    // Fast-forwarding to dawn
};

// ============================================
// Bed Instance
// ============================================
struct Bed {
    Vec3 position;      // Foot of the bed
    Vec3 headPosition;  // Head of the bed
    int direction;      // 0=+X, 1=+Z, 2=-X, 3=-Z
    BedState state;
    float sleepTimer;   // Time spent in bed
    bool isValid;       // Has valid placement

    Bed() : position(0, 0, 0), headPosition(0, 0, 0), direction(0),
            state(BedState::Empty), sleepTimer(0.0f), isValid(false) {}
    Bed(const Vec3& pos, int dir)
        : position(pos), direction(dir), state(BedState::Empty),
          sleepTimer(0.0f), isValid(true) {
        // Calculate head position based on direction
        switch (dir) {
            case 0: headPosition = pos + Vec3(1, 0, 0); break;  // +X
            case 1: headPosition = pos + Vec3(0, 0, 1); break;  // +Z
            case 2: headPosition = pos + Vec3(-1, 0, 0); break; // -X
            case 3: headPosition = pos + Vec3(0, 0, -1); break; // -Z
        }
    }
};

// ============================================
// Sleep Result
// ============================================
enum class SleepResult {
    Success,            // Sleep started
    NotNight,           // Can only sleep at night
    TooFar,             // Player too far from bed
    Obstructed,         // Bed is blocked
    MonstersNearby,     // Hostile mobs within 8 blocks
    AlreadySkipping     // Night is already being skipped
};

// ============================================
// Bed System
// ============================================
// Manages bed blocks, sleep mechanic, and night skipping.
// Players can sleep in beds to skip the night and set spawn.
// ============================================
class BedSystem {
private:
    World* world;
    DayNightCycle* dayNightCycle;
    LightSystem* lightSystem;

    std::vector<Bed> beds;
    Bed* activeBed;           // Bed player is currently in
    float skipSpeed;          // How fast time advances during skip
    float skipProgress;       // 0.0 to 1.0 during skip
    bool isSkippingNight;     // Currently fast-forwarding

    // Spawn point tracking
    Vec3 respawnPoint;
    bool respawnSet;

    // Callbacks
    std::function<void()> onSleepStarted;
    std::function<void()> onSleepEnded;
    std::function<void()> onRespawnPointSet;

    // Internal helpers
    bool CanSleepAt(const Bed& bed, const Vec3& playerPos) const;
    bool AreMonstersNearby(const Vec3& position) const;
    bool IsBedObstructed(const Bed& bed) const;
    Bed* FindBedAt(const Vec3& position);
    void StartNightSkip();
    void FinishNightSkip();
    void CancelSleep();

public:
    BedSystem(World* w, DayNightCycle* cycle, LightSystem* lights);
    ~BedSystem();

    // Update - call every frame
    void Update(float deltaTime, const Vec3& playerPos);

    // Bed management
    bool PlaceBed(int x, int y, int z, int direction);
    bool RemoveBed(int x, int y, int z);
    void ClearAllBeds();

    // Sleep mechanic
    SleepResult TrySleep(const Vec3& playerPos);
    SleepResult TrySleepAt(const Vec3& bedPos, const Vec3& playerPos);
    void WakeUp();
    bool IsSleeping() const { return activeBed != nullptr; }
    bool IsSkippingNight() const { return isSkippingNight; }

    // Respawn point
    void SetRespawnPoint(const Vec3& pos);
    Vec3 GetRespawnPoint() const { return respawnPoint; }
    bool HasRespawnPoint() const { return respawnSet; }
    void ClearRespawnPoint() { respawnSet = false; }

    // Bed queries
    Bed* GetActiveBed() const { return activeBed; }
    size_t GetBedCount() const { return beds.size(); }
    const std::vector<Bed>& GetAllBeds() const { return beds; }
    bool IsBedAt(int x, int y, int z) const;

    // Skip configuration
    void SetSkipSpeed(float speed) { skipSpeed = speed; }
    float GetSkipSpeed() const { return skipSpeed; }
    float GetSkipProgress() const { return skipProgress; }

    // Callbacks
    void SetOnSleepStarted(std::function<void()> fn) { onSleepStarted = fn; }
    void SetOnSleepEnded(std::function<void()> fn) { onSleepEnded = fn; }
    void SetOnRespawnPointSet(std::function<void()> fn) { onRespawnPointSet = fn; }

    // Static helpers
    static bool IsNightTime(DayNightCycle* cycle);
    static std::string SleepResultToString(SleepResult result);
};

} // namespace vge
