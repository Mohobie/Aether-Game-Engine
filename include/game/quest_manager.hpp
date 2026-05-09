#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace game {

enum class QuestStatus { Inactive, Active, Completed, Failed };

struct QuestObjective {
    std::string description;
    uint32_t target = 1;
    uint32_t current = 0;
    bool complete() const { return current >= target; }
};

struct QuestReward {
    std::string itemID;
    uint32_t count = 1;
};

class Quest {
public:
    std::string id;
    std::string title;
    std::string description;
    std::vector<QuestObjective> objectives;
    std::vector<QuestReward> rewards;
    QuestStatus status = QuestStatus::Inactive;

    void start() { status = QuestStatus::Active; }
    void updateObjective(size_t idx, uint32_t delta);
    bool isComplete() const;
    void complete();
};

class QuestManager {
public:
    void addQuest(Quest quest);
    Quest* get(const std::string& id);
    std::vector<Quest*> activeQuests();
    std::vector<Quest*> completedQuests();
private:
    std::vector<Quest> quests;
};

} // namespace game
