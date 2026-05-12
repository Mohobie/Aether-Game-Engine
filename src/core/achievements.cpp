#include "achievements.h"
#include "config.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace vge {

Achievement::Achievement(const std::string& id, const std::string& name, 
                         const std::string& description, AchievementType type, int target)
    : id(id), name(name), description(description), type(type), target(target), 
      current(0), unlocked(false) {}

bool Achievement::UpdateProgress(int value) {
    if (unlocked) return false;
    
    current = value;
    if (current >= target) {
        current = target;
        unlocked = true;
        return true; // Just unlocked
    }
    return false;
}

bool Achievement::Increment(int amount) {
    if (unlocked) return false;
    
    current += amount;
    if (current >= target) {
        current = target;
        unlocked = true;
        return true; // Just unlocked
    }
    return false;
}

AchievementManager::AchievementManager() : initialized(false) {}

AchievementManager::~AchievementManager() {
    // No auto-save to avoid path issues
}

void AchievementManager::Initialize() {
    RegisterDefaultAchievements();
    initialized = true;
    std::cout << "[Achievements] Initialized with " << achievements.size() << " achievements" << std::endl;
}

void AchievementManager::RegisterDefaultAchievements() {
    // Mining achievements
    RegisterAchievement("miner", "Getting Started", "Mine your first block", 
                       AchievementType::MineBlock, 1);
    RegisterAchievement("miner10", "Novice Miner", "Mine 10 blocks", 
                       AchievementType::MineBlock, 10);
    RegisterAchievement("miner100", "Experienced Miner", "Mine 100 blocks", 
                       AchievementType::MineBlock, 100);
    RegisterAchievement("miner1000", "Master Miner", "Mine 1000 blocks", 
                       AchievementType::MineBlock, 1000);
    
    // Building achievements
    RegisterAchievement("builder", "Builder", "Place your first block", 
                       AchievementType::PlaceBlock, 1);
    RegisterAchievement("builder10", "Constructor", "Place 10 blocks", 
                       AchievementType::PlaceBlock, 10);
    RegisterAchievement("builder100", "Architect", "Place 100 blocks", 
                       AchievementType::PlaceBlock, 100);
    
    // Exploration achievements
    RegisterAchievement("explorer", "Explorer", "Walk 100 blocks", 
                       AchievementType::Walk, 100);
    RegisterAchievement("explorer1000", "Adventurer", "Walk 1000 blocks", 
                       AchievementType::Walk, 1000);
    
    // Time achievements
    RegisterAchievement("day1", "First Day", "Survive your first day", 
                       AchievementType::SurviveDays, 1);
    RegisterAchievement("day7", "One Week", "Survive 7 days", 
                       AchievementType::SurviveDays, 7);
    RegisterAchievement("day30", "One Month", "Survive 30 days", 
                       AchievementType::SurviveDays, 30);
    
    // Special achievements
    RegisterAchievement("depth", "Deep Dive", "Reach depth 50", 
                       AchievementType::ReachDepth, 50);
    RegisterAchievement("height", "Sky High", "Reach height 200", 
                       AchievementType::ReachHeight, 200);
    RegisterAchievement("diamonds", "Diamonds!", "Find your first diamond", 
                       AchievementType::FindDiamond, 1);
    RegisterAchievement("craft", "Crafter", "Craft your first item", 
                       AchievementType::CraftItem, 1);
}

void AchievementManager::RegisterAchievement(const std::string& id, const std::string& name,
                                              const std::string& description,
                                              AchievementType type, int target) {
    achievements.emplace_back(id, name, description, type, target);
}

bool AchievementManager::UpdateProgress(AchievementType type, int value) {
    bool anyUnlocked = false;
    
    for (auto& achievement : achievements) {
        if (achievement.GetType() == type) {
            if (achievement.UpdateProgress(value)) {
                anyUnlocked = true;
                std::cout << "[Achievement Unlocked] " << achievement.GetName() 
                         << ": " << achievement.GetDescription() << std::endl;
                
                if (onAchievementUnlocked) {
                    onAchievementUnlocked(achievement);
                }
            }
        }
    }
    
    return anyUnlocked;
}

bool AchievementManager::Increment(AchievementType type, int amount) {
    bool anyUnlocked = false;
    
    for (auto& achievement : achievements) {
        if (achievement.GetType() == type) {
            if (achievement.Increment(amount)) {
                anyUnlocked = true;
                std::cout << "[Achievement Unlocked] " << achievement.GetName() 
                         << ": " << achievement.GetDescription() << std::endl;
                
                if (onAchievementUnlocked) {
                    onAchievementUnlocked(achievement);
                }
            }
        }
    }
    
    return anyUnlocked;
}

const Achievement* AchievementManager::GetAchievement(const std::string& id) const {
    for (const auto& achievement : achievements) {
        if (achievement.GetId() == id) {
            return &achievement;
        }
    }
    return nullptr;
}

std::vector<Achievement> AchievementManager::GetUnlockedAchievements() const {
    std::vector<Achievement> result;
    for (const auto& achievement : achievements) {
        if (achievement.IsUnlocked()) {
            result.push_back(achievement);
        }
    }
    return result;
}

std::vector<Achievement> AchievementManager::GetLockedAchievements() const {
    std::vector<Achievement> result;
    for (const auto& achievement : achievements) {
        if (!achievement.IsUnlocked()) {
            result.push_back(achievement);
        }
    }
    return result;
}

void AchievementManager::PrintStatus() const {
    int unlocked = 0;
    for (const auto& achievement : achievements) {
        if (achievement.IsUnlocked()) unlocked++;
    }
    
    std::cout << "=== Achievements ===" << std::endl;
    std::cout << "Unlocked: " << unlocked << "/" << achievements.size() << std::endl;
    
    for (const auto& achievement : achievements) {
        std::cout << (achievement.IsUnlocked() ? "[X] " : "[ ] ");
        std::cout << achievement.GetName() << " - " << achievement.GetDescription();
        if (!achievement.IsUnlocked()) {
            std::cout << " (" << achievement.GetCurrent() << "/" << achievement.GetTarget() << ")";
        }
        std::cout << std::endl;
    }
}

bool AchievementManager::Save(const std::string& path) const {
    json j;
    j["version"] = 1;
    
    json achievementsJson = json::array();
    for (const auto& a : achievements) {
        json achievementJson;
        achievementJson["id"] = a.GetId();
        achievementJson["name"] = a.GetName();
        achievementJson["description"] = a.GetDescription();
        achievementJson["type"] = static_cast<int>(a.GetType());
        achievementJson["target"] = a.GetTarget();
        achievementJson["current"] = a.GetCurrent();
        achievementJson["unlocked"] = a.IsUnlocked();
        achievementsJson.push_back(achievementJson);
    }
    j["achievements"] = achievementsJson;
    
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Achievements] Failed to write: " << path << std::endl;
        return false;
    }
    
    file << j.dump(4);
    std::cout << "[Achievements] Saved to " << path << std::endl;
    return true;
}

bool AchievementManager::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Achievements] Failed to open: " << path << std::endl;
        return false;
    }
    
    try {
        json j;
        file >> j;
        
        if (!j.contains("achievements")) {
            std::cerr << "[Achievements] Invalid format: missing 'achievements' array" << std::endl;
            return false;
        }
        
        // Clear existing achievements
        achievements.clear();
        
        for (const auto& achievementJson : j["achievements"]) {
            std::string id = achievementJson["id"].get<std::string>();
            std::string name = achievementJson["name"].get<std::string>();
            std::string description = achievementJson["description"].get<std::string>();
            AchievementType type = static_cast<AchievementType>(achievementJson["type"].get<int>());
            int target = achievementJson["target"].get<int>();
            
            Achievement achievement(id, name, description, type, target);
            
            // Restore progress
            if (achievementJson.contains("current")) {
                achievement.UpdateProgress(achievementJson["current"].get<int>());
            }
            if (achievementJson.contains("unlocked") && achievementJson["unlocked"].get<bool>()) {
                achievement.Increment(0); // Force unlock if needed
            }
            
            achievements.push_back(std::move(achievement));
        }
        
        std::cout << "[Achievements] Loaded " << achievements.size() << " achievements from " << path << std::endl;
        initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[Achievements] JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

} // namespace vge