#pragma once
#include <string>
#include <vector>
#include <functional>

namespace vge {

// ============================================
// Quest Types
// ============================================
enum class QuestType {
    Collect,      // Gather items
    Kill,         // Defeat mobs
    Explore,      // Visit locations
    Craft,        // Craft items
    Survive,      // Survive for time
    Escort,       // Protect NPC
    Custom        // Custom objective
};

// ============================================
// Quest Objective
// ============================================
struct QuestObjective {
    std::string description;
    QuestType type;
    std::string targetId;    // Item ID, mob ID, location ID
    int required;
    int current;
    bool completed;
    
    QuestObjective() : required(1), current(0), completed(false) {}
};

// ============================================
// Quest Definition
// ============================================
struct Quest {
    std::string id;
    std::string title;
    std::string description;
    std::vector<QuestObjective> objectives;
    std::vector<std::string> rewards;  // Item IDs
    bool active;
    bool completed;
    
    Quest() : active(false), completed(false) {}
    
    bool IsComplete() const {
        for (const auto& obj : objectives) {
            if (!obj.completed) return false;
        }
        return true;
    }
};

// ============================================
// Quest System
// ============================================
class QuestSystem {
public:
    QuestSystem();
    ~QuestSystem();

    // Quest management
    void RegisterQuest(const Quest& quest);
    void StartQuest(const std::string& questId);
    void CompleteQuest(const std::string& questId);
    void AbandonQuest(const std::string& questId);
    
    // Progress tracking
    void UpdateProgress(QuestType type, const std::string& targetId, int amount = 1);
    void UpdateObjective(const std::string& questId, int objectiveIndex, int progress);
    
    // Getters
    Quest* GetQuest(const std::string& questId);
    std::vector<Quest*> GetActiveQuests();
    std::vector<Quest*> GetCompletedQuests();
    
    // Callbacks
    std::function<void(const Quest&)> onQuestStarted;
    std::function<void(const Quest&)> onQuestCompleted;
    std::function<void(const QuestObjective&)> onObjectiveUpdated;

private:
    std::vector<Quest> quests;
};

} // namespace vge
