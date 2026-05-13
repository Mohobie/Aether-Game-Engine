#pragma once
#include "voxel/world.h"
#include "core/player_controller.h"
#include <cstring>
#include <vector>

namespace vge {

// ============================================
// Save Game Info
// ============================================
struct SaveGameInfo {
    std::string name;
    std::string filePath;
    std::string createdDate;
    std::string lastPlayedDate;
    int playTimeMinutes;
    Vec3 playerPosition;
    int worldSeed;
    int64_t fileSize;
};

// ============================================
// Save Game Manager
// ============================================
class SaveGameManager {
public:
    SaveGameManager();
    ~SaveGameManager();

    // Initialize with save directory
    void Initialize(const std::string& saveDir);

    // Save current game state
    bool SaveGame(const std::string& saveName, 
                  const World& world,
                  const PlayerController& player,
                  float dayTime,
                  int dayCount);

    // Load game state
    bool LoadGame(const std::string& saveName,
                  World& world,
                  PlayerController& player,
                  float& outDayTime,
                  int& outDayCount);

    // Get list of saved games
    std::vector<SaveGameInfo> GetSaveList() const;

    // Delete a save
    bool DeleteSave(const std::string& saveName);

    // Check if save exists
    bool SaveExists(const std::string& saveName) const;

    // Auto-save
    void EnableAutoSave(bool enable) { autoSaveEnabled = enable; }
    void SetAutoSaveInterval(float minutes) { autoSaveInterval = minutes; }
    void UpdateAutoSave(float deltaTime, const World& world, const PlayerController& player, float dayTime, int dayCount);

    // Quick save/load slots
    bool QuickSave(const World& world, const PlayerController& player, float dayTime, int dayCount);
    bool QuickLoad(World& world, PlayerController& player, float& outDayTime, int& outDayCount);

private:
    std::string saveDirectory;
    bool autoSaveEnabled;
    float autoSaveInterval;
    float autoSaveTimer;

    std::string GetSavePath(const std::string& saveName) const;
    std::string GetQuickSavePath() const;
    void EnsureSaveDirectoryExists() const;
    SaveGameInfo GetSaveInfo(const std::string& filePath) const;
};

} // namespace vge
