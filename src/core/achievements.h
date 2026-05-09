#pragma once
#include <string>
#include <vector>
#include <functional>
#include "json.hpp"

namespace vge {

enum class AchievementType {
    MineBlock,
    PlaceBlock,
    Walk,
    SurviveDays,
    ReachDepth,
    ReachHeight,
    FindDiamond,
    CraftItem,
    KillEnemy,
    Custom
};

class Achievement {
private:
    std::string id;
    std::string name;
    std::string description;
    AchievementType type;
    int target;
    int current;
    bool unlocked;
    
public:
    Achievement(const std::string& id, const std::string& name,
                const std::string& description, AchievementType type, int target);
    
    bool UpdateProgress(int value);
    bool Increment(int amount = 1);
    
    std::string GetId() const { return id; }
    std::string GetName() const { return name; }
    std::string GetDescription() const { return description; }
    AchievementType GetType() const { return type; }
    int GetTarget() const { return target; }
    int GetCurrent() const { return current; }
    bool IsUnlocked() const { return unlocked; }
};

class AchievementManager {
private:
    std::vector<Achievement> achievements;
    bool initialized;
    
    void RegisterDefaultAchievements();
    
public:
    std::function<void(const Achievement&)> onAchievementUnlocked;
    
    AchievementManager();
    ~AchievementManager();
    
    void Initialize();
    void RegisterAchievement(const std::string& id, const std::string& name,
                            const std::string& description,
                            AchievementType type, int target);
    
    bool UpdateProgress(AchievementType type, int value);
    bool Increment(AchievementType type, int amount = 1);
    
    const Achievement* GetAchievement(const std::string& id) const;
    std::vector<Achievement> GetUnlockedAchievements() const;
    std::vector<Achievement> GetLockedAchievements() const;
    const std::vector<Achievement>& GetAllAchievements() const { return achievements; }
    
    void PrintStatus() const;
    
    bool Save(const std::string& path) const;
    bool Load(const std::string& path);
};

} // namespace vge
