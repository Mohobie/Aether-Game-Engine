#include "achievements.h"
#include "config.h"
#include <fstream>
#include <iostream>

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
    if (initialized) Save();
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
    j["achievements"] = json::array();
    
    for (const auto& achievement : achievements) {
        json ach;
        ach["id"] = achievement.GetId();
        ach["current"] = achievement.GetCurrent();
        ach["unlocked"] = achievement.IsUnlocked();
        j["achievements"].push_back(ach);
    }
    
    std::ofstream file(path);
    if (!file.is_open()) return false;
    
    file << j.dump(2);
    return true;
}

bool AchievementManager::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    
    try {
        json j;
        file >> j;
        
        if (j.contains("achievements")) {
            for (const auto& ach : j["achievements"]) {
                std::string id = ach["id"].get<std::string>();
                int current = ach["current"].get<int>();
                bool unlocked = ach["unlocked"].get<bool>();
                
                for (auto& achievement : achievements) {
                    if (achievement.GetId() == id) {
                        achievement.UpdateProgress(current);
                        break;
                    }
                }
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace vge