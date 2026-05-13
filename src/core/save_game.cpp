#include "save_game.h"
#include "voxel/world_serializer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace vge {

SaveGameManager::SaveGameManager()
    : autoSaveEnabled(true), autoSaveInterval(5.0f), autoSaveTimer(0.0f) {
}

SaveGameManager::~SaveGameManager() {
}

void SaveGameManager::Initialize(const std::string& saveDir) {
    saveDirectory = saveDir;
    EnsureSaveDirectoryExists();
}

bool SaveGameManager::SaveGame(const std::string& saveName,
                               const World& world,
                               const PlayerController& player,
                               float dayTime,
                               int dayCount) {
    std::string savePath = GetSavePath(saveName);
    
    WorldSerializer serializer;
    Vec3 playerPos = player.GetPosition();
    Vec3 playerRot(player.GetYaw(), player.GetPitch(), 0.0f);
    
    bool success = serializer.SerializeWorld(world, savePath, playerPos, playerRot,
                                             static_cast<int>(dayTime), dayCount);
    
    if (success) {
        std::cout << "[SaveGame] Saved game: " << saveName << std::endl;
    } else {
        std::cerr << "[SaveGame] Failed to save: " << saveName << std::endl;
    }
    
    return success;
}

bool SaveGameManager::LoadGame(const std::string& saveName,
                               World& world,
                               PlayerController& player,
                               float& outDayTime,
                               int& outDayCount) {
    std::string savePath = GetSavePath(saveName);
    
    if (!std::filesystem::exists(savePath)) {
        std::cerr << "[SaveGame] Save not found: " << saveName << std::endl;
        return false;
    }
    
    WorldSerializer serializer;
    Vec3 playerPos, playerRot;
    int timeOfDay, dayCount;
    
    bool success = serializer.DeserializeWorld(world, savePath, playerPos, playerRot,
                                               timeOfDay, dayCount);
    
    if (success) {
        player.SetPosition(playerPos);
        // player.SetRotation(playerRot.x, playerRot.y); // Need to add this method
        outDayTime = static_cast<float>(timeOfDay);
        outDayCount = dayCount;
        std::cout << "[SaveGame] Loaded game: " << saveName << std::endl;
    } else {
        std::cerr << "[SaveGame] Failed to load: " << saveName << std::endl;
    }
    
    return success;
}

std::vector<SaveGameInfo> SaveGameManager::GetSaveList() const {
    std::vector<SaveGameInfo> saves;
    
    if (!std::filesystem::exists(saveDirectory)) {
        return saves;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(saveDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".bin") {
            SaveGameInfo info = GetSaveInfo(entry.path().string());
            info.name = entry.path().stem().string();
            saves.push_back(info);
        }
    }
    
    // Sort by last played date (newest first)
    std::sort(saves.begin(), saves.end(), [](const SaveGameInfo& a, const SaveGameInfo& b) {
        return a.lastPlayedDate > b.lastPlayedDate;
    });
    
    return saves;
}

bool SaveGameManager::DeleteSave(const std::string& saveName) {
    std::string savePath = GetSavePath(saveName);
    
    if (!std::filesystem::exists(savePath)) {
        return false;
    }
    
    try {
        std::filesystem::remove(savePath);
        std::cout << "[SaveGame] Deleted: " << saveName << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SaveGame] Failed to delete: " << e.what() << std::endl;
        return false;
    }
}

bool SaveGameManager::SaveExists(const std::string& saveName) const {
    return std::filesystem::exists(GetSavePath(saveName));
}

void SaveGameManager::UpdateAutoSave(float deltaTime, const World& world, const PlayerController& player, float dayTime, int dayCount) {
    if (!autoSaveEnabled) return;
    
    autoSaveTimer += deltaTime;
    
    if (autoSaveTimer >= autoSaveInterval * 60.0f) { // Convert minutes to seconds
        autoSaveTimer = 0.0f;
        QuickSave(world, player, dayTime, dayCount);
    }
}

bool SaveGameManager::QuickSave(const World& world, const PlayerController& player, float dayTime, int dayCount) {
    return SaveGame("quicksave", world, player, dayTime, dayCount);
}

bool SaveGameManager::QuickLoad(World& world, PlayerController& player, float& outDayTime, int& outDayCount) {
    return LoadGame("quicksave", world, player, outDayTime, outDayCount);
}

std::string SaveGameManager::GetSavePath(const std::string& saveName) const {
    return saveDirectory + "/" + saveName + ".bin";
}

std::string SaveGameManager::GetQuickSavePath() const {
    return saveDirectory + "/quicksave.bin";
}

void SaveGameManager::EnsureSaveDirectoryExists() const {
    if (!std::filesystem::exists(saveDirectory)) {
        std::filesystem::create_directories(saveDirectory);
    }
}

SaveGameInfo SaveGameManager::GetSaveInfo(const std::string& filePath) const {
    SaveGameInfo info;
    info.filePath = filePath;
    
    // Get file size
    info.fileSize = std::filesystem::file_size(filePath);
    
    // Get last modified time
    auto ftime = std::filesystem::last_write_time(filePath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    auto time = std::chrono::system_clock::to_time_t(sctp);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    info.lastPlayedDate = ss.str();
    info.createdDate = info.lastPlayedDate; // Same for now
    
    // Try to read world info from file
    std::ifstream file(filePath, std::ios::binary);
    if (file.is_open()) {
        WorldFileHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        
        if (header.magic == WORLD_MAGIC) {
            info.playerPosition = Vec3(header.playerPosX, header.playerPosY, header.playerPosZ);
            info.worldSeed = static_cast<int>(header.worldSeed);
        }
        
        file.close();
    }
    
    return info;
}

} // namespace vge
