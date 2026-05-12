#include "save_system.h"
#include "voxel/world_serializer.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <cstring>

namespace vge {

SaveSystem::SaveSystem()
    : autoSaveTimer(0.0f)
    , autoSaveInterval(300.0f) // 5 minutes
    , autoSaveEnabled(true)
    , manualSavePending(false)
{}

SaveSystem::~SaveSystem() {}

void SaveSystem::Initialize(const std::string& saveDir) {
    saveDirectory = saveDir;
    // Create directory if it doesn't exist
    std::filesystem::create_directories(saveDirectory);
}

std::string SaveSystem::GetSlotPath(int slot) const {
    return saveDirectory + "/slot" + std::to_string(slot);
}

std::string SaveSystem::GetWorldPath(int slot) const {
    return GetSlotPath(slot) + "/world.bin";
}

std::string SaveSystem::GetPlayerPath(int slot) const {
    return GetSlotPath(slot) + "/player.bin";
}

std::string SaveSystem::GetEnemyPath(int slot) const {
    return GetSlotPath(slot) + "/enemies.bin";
}

std::string SaveSystem::GetMetaPath(int slot) const {
    return GetSlotPath(slot) + "/meta.bin";
}

bool SaveSystem::SaveGame(int slot, const World& world,
                          const Vec3& playerPos, const Vec3& playerRot, const Vec3& playerVel,
                          const PlayerStats& stats, const Inventory& inventory,
                          const EnemySpawner* enemySpawner, const TimeSystem* timeSystem) {
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        if (onError) onError("Invalid save slot: " + std::to_string(slot));
        return false;
    }

    std::string slotPath = GetSlotPath(slot);
    std::filesystem::create_directories(slotPath);

    // Save world
    int timeOfDay = timeSystem ? timeSystem->GetTimeOfDay() : 6000;
    int dayCount = timeSystem ? timeSystem->GetDayCount() : 0;

    if (!worldSerializer.SerializeWorld(world, GetWorldPath(slot), playerPos, playerRot, timeOfDay, dayCount)) {
        if (onError) onError("Failed to save world");
        return false;
    }

    // Save player data
    PlayerData playerData;
    playerData.FromPlayer(playerPos, playerRot, playerVel, stats, inventory);
    if (!playerData.SaveToFile(GetPlayerPath(slot))) {
        if (onError) onError("Failed to save player data");
        return false;
    }

    // Save enemies
    std::vector<EnemySaveData> enemyData;
    if (enemySpawner) {
        for (auto* enemy : enemySpawner->GetAllEnemies()) {
            if (enemy && !enemy->IsDead()) {
                EnemySaveData data;
                data.type = static_cast<int32_t>(enemy->GetType());
                Vec3 pos = enemy->GetPosition();
                data.posX = pos.x;
                data.posY = pos.y;
                data.posZ = pos.z;
                data.health = enemy->GetHealth();
                data.maxHealth = enemy->GetMaxHealth();
                data.isDead = enemy->IsDead();
                enemyData.push_back(data);
            }
        }
    }
    if (!SaveEnemies(enemyData, GetEnemyPath(slot))) {
        if (onError) onError("Failed to save enemies");
        return false;
    }

    // Save metadata
    std::ofstream metaFile(GetMetaPath(slot), std::ios::binary);
    if (metaFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        std::string timestamp = ss.str();

        uint32_t nameLen = static_cast<uint32_t>(timestamp.length());
        metaFile.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
        metaFile.write(timestamp.c_str(), nameLen);
        metaFile.write(reinterpret_cast<const char*>(&dayCount), sizeof(int));
        metaFile.write(reinterpret_cast<const char*>(&playerData.playTime), sizeof(float));
        metaFile.close();
    }

    std::cout << "[SaveSystem] Game saved to slot " << slot << std::endl;
    if (onSaveCompleted) onSaveCompleted(slot);
    return true;
}

bool SaveSystem::LoadGame(int slot, World& world,
                          Vec3& outPlayerPos, Vec3& outPlayerRot, Vec3& outPlayerVel,
                          PlayerStats& outStats, Inventory& outInventory,
                          std::vector<EnemySaveData>& outEnemies,
                          int& outTimeOfDay, int& outDayCount) {
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        if (onError) onError("Invalid save slot: " + std::to_string(slot));
        return false;
    }

    if (!HasSave(slot)) {
        if (onError) onError("No save found in slot " + std::to_string(slot));
        return false;
    }

    // Load world
    Vec3 playerRot;
    if (!worldSerializer.DeserializeWorld(world, GetWorldPath(slot), outPlayerPos, playerRot, outTimeOfDay, outDayCount)) {
        if (onError) onError("Failed to load world");
        return false;
    }
    outPlayerRot = playerRot;

    // Load player data
    PlayerData playerData;
    if (!playerData.LoadFromFile(GetPlayerPath(slot))) {
        if (onError) onError("Failed to load player data");
        return false;
    }

    outPlayerPos = playerData.position;
    outPlayerRot = playerData.rotation;
    outPlayerVel = playerData.velocity;

    outStats = PlayerStats(playerData.spawnPoint);
    outStats.SetHealth(playerData.health);
    outStats.SetHunger(playerData.hunger);
    outStats.SetStamina(playerData.stamina);

    outInventory = Inventory(playerData.inventorySize);
    for (size_t i = 0; i < playerData.inventorySlots.size(); ++i) {
        if (playerData.inventorySlots[i].type != BLOCK_AIR && playerData.inventorySlots[i].count > 0) {
            outInventory.AddItem(playerData.inventorySlots[i].type, playerData.inventorySlots[i].count);
        }
    }
    outInventory.SelectSlot(playerData.selectedSlot);

    // Load enemies
    if (!LoadEnemies(outEnemies, GetEnemyPath(slot))) {
        outEnemies.clear();
    }

    std::cout << "[SaveSystem] Game loaded from slot " << slot << std::endl;
    if (onLoadCompleted) onLoadCompleted(slot);
    return true;
}

bool SaveSystem::QuickSave(const World& world,
                           const Vec3& playerPos, const Vec3& playerRot, const Vec3& playerVel,
                           const PlayerStats& stats, const Inventory& inventory,
                           const EnemySpawner* enemySpawner, const TimeSystem* timeSystem) {
    return SaveGame(0, world, playerPos, playerRot, playerVel, stats, inventory, enemySpawner, timeSystem);
}

bool SaveSystem::QuickLoad(World& world,
                           Vec3& outPlayerPos, Vec3& outPlayerRot, Vec3& outPlayerVel,
                           PlayerStats& outStats, Inventory& outInventory,
                           std::vector<EnemySaveData>& outEnemies,
                           int& outTimeOfDay, int& outDayCount) {
    return LoadGame(0, world, outPlayerPos, outPlayerRot, outPlayerVel, outStats, outInventory, outEnemies, outTimeOfDay, outDayCount);
}

void SaveSystem::Update(float deltaTime) {
    if (!autoSaveEnabled) return;

    autoSaveTimer += deltaTime;
    if (autoSaveTimer >= autoSaveInterval) {
        autoSaveTimer = 0.0f;
        manualSavePending = true; // Trigger auto-save on next opportunity
    }
}

void SaveSystem::TriggerManualSave() {
    manualSavePending = true;
}

SaveSlotInfo SaveSystem::GetSlotInfo(int slot) const {
    SaveSlotInfo info;
    info.name = "Slot " + std::to_string(slot);
    info.path = GetSlotPath(slot);
    info.exists = HasSave(slot);

    if (info.exists) {
        std::ifstream metaFile(GetMetaPath(slot), std::ios::binary);
        if (metaFile.is_open()) {
            uint32_t nameLen;
            metaFile.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
            if (nameLen > 0 && nameLen < 256) {
                std::string timestamp(nameLen, '\0');
                metaFile.read(&timestamp[0], nameLen);
                info.lastPlayed = timestamp;
            }
            metaFile.read(reinterpret_cast<char*>(&info.dayCount), sizeof(int));
            metaFile.read(reinterpret_cast<char*>(&info.playTime), sizeof(float));
            metaFile.close();
        }
    }

    return info;
}

std::vector<SaveSlotInfo> SaveSystem::GetAllSlotInfo() const {
    std::vector<SaveSlotInfo> result;
    for (int i = 0; i < MAX_SAVE_SLOTS; ++i) {
        result.push_back(GetSlotInfo(i));
    }
    return result;
}

bool SaveSystem::DeleteSave(int slot) {
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) return false;

    std::string slotPath = GetSlotPath(slot);
    if (!std::filesystem::exists(slotPath)) return false;

    try {
        std::filesystem::remove_all(slotPath);
        std::cout << "[SaveSystem] Deleted save slot " << slot << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SaveSystem] Failed to delete slot " << slot << ": " << e.what() << std::endl;
        return false;
    }
}

bool SaveSystem::HasSave(int slot) const {
    return std::filesystem::exists(GetWorldPath(slot));
}

bool SaveSystem::SaveEnemies(const std::vector<EnemySaveData>& enemies, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    uint32_t count = static_cast<uint32_t>(enemies.size());
    file.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

    for (const auto& enemy : enemies) {
        file.write(reinterpret_cast<const char*>(&enemy), sizeof(EnemySaveData));
    }

    file.close();
    return true;
}

bool SaveSystem::LoadEnemies(std::vector<EnemySaveData>& enemies, const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    uint32_t count;
    file.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));

    enemies.resize(count);
    for (uint32_t i = 0; i < count; ++i) {
        file.read(reinterpret_cast<char*>(&enemies[i]), sizeof(EnemySaveData));
    }

    file.close();
    return true;
}

} // namespace vge
