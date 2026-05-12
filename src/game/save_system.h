#pragma once

#include "voxel/world.h"
#include "voxel/world_serializer.h"
#include "game/player_data.h"
#include "ai/enemy_ai.h"
#include "ai/enemy_spawner.h"
#include "core/time_system.h"
#include "core/inventory.h"
#include "game/player_stats.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>

namespace vge {

// ============================================
// Enemy Save Data
// ============================================
struct EnemySaveData {
    int32_t type;
    float posX, posY, posZ;
    float health;
    float maxHealth;
    bool isDead;
};

// ============================================
// Save Slot Info
// ============================================
struct SaveSlotInfo {
    std::string name;
    std::string path;
    std::string lastPlayed;
    int dayCount;
    float playTime;
    bool exists;
};

// ============================================
// SaveSystem - High-level save/load manager
// ============================================
class SaveSystem {
public:
    SaveSystem();
    ~SaveSystem();

    // Initialize with save directory
    void Initialize(const std::string& saveDir);

    // Save slots
    static constexpr int MAX_SAVE_SLOTS = 3;
    bool SaveGame(int slot, const World& world,
                  const Vec3& playerPos, const Vec3& playerRot, const Vec3& playerVel,
                  const PlayerStats& stats, const Inventory& inventory,
                  const EnemySpawner* enemySpawner, const TimeSystem* timeSystem);
    bool LoadGame(int slot, World& world,
                  Vec3& outPlayerPos, Vec3& outPlayerRot, Vec3& outPlayerVel,
                  PlayerStats& outStats, Inventory& outInventory,
                  std::vector<EnemySaveData>& outEnemies,
                  int& outTimeOfDay, int& outDayCount);

    // Quick save / load (uses slot 0 as auto-save)
    bool QuickSave(const World& world,
                   const Vec3& playerPos, const Vec3& playerRot, const Vec3& playerVel,
                   const PlayerStats& stats, const Inventory& inventory,
                   const EnemySpawner* enemySpawner, const TimeSystem* timeSystem);
    bool QuickLoad(World& world,
                   Vec3& outPlayerPos, Vec3& outPlayerRot, Vec3& outPlayerVel,
                   PlayerStats& outStats, Inventory& outInventory,
                   std::vector<EnemySaveData>& outEnemies,
                   int& outTimeOfDay, int& outDayCount);

    // Auto-save (every 5 minutes)
    void Update(float deltaTime);
    void SetAutoSaveInterval(float seconds) { autoSaveInterval = seconds; }
    float GetAutoSaveInterval() const { return autoSaveInterval; }
    bool IsAutoSaveEnabled() const { return autoSaveEnabled; }
    void SetAutoSaveEnabled(bool enabled) { autoSaveEnabled = enabled; }

    // Manual save trigger (e.g., F5 key)
    void TriggerManualSave();
    bool IsManualSavePending() const { return manualSavePending; }

    // Save slot info
    SaveSlotInfo GetSlotInfo(int slot) const;
    std::vector<SaveSlotInfo> GetAllSlotInfo() const;
    bool DeleteSave(int slot);
    bool HasSave(int slot) const;

    // Get save path for slot
    std::string GetSlotPath(int slot) const;

    // Callbacks
    std::function<void(int)> onSaveCompleted;
    std::function<void(int)> onLoadCompleted;
    std::function<void(const std::string&)> onError;

private:
    std::string saveDirectory;
    float autoSaveTimer;
    float autoSaveInterval;
    bool autoSaveEnabled;
    bool manualSavePending;

    WorldSerializer worldSerializer;

    // Internal save helpers
    bool SaveEnemies(const std::vector<EnemySaveData>& enemies, const std::string& path);
    bool LoadEnemies(std::vector<EnemySaveData>& enemies, const std::string& path);

    std::string GetWorldPath(int slot) const;
    std::string GetPlayerPath(int slot) const;
    std::string GetEnemyPath(int slot) const;
    std::string GetMetaPath(int slot) const;
};

} // namespace vge
