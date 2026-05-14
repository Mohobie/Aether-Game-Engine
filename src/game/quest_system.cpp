#include "quest_system.h"
#include "core/logger.h"

namespace vge {

QuestSystem::QuestSystem() {
}

QuestSystem::~QuestSystem() {
}

void QuestSystem::RegisterQuest(const Quest& quest) {
    quests.push_back(quest);
    Logger::Info("[QuestSystem] Registered quest: " + quest.title);
}

void QuestSystem::StartQuest(const std::string& questId) {
    Quest* quest = GetQuest(questId);
    if (!quest) {
        Logger::Error("[QuestSystem] Quest not found: " + questId);
        return;
    }
    
    if (quest->active) {
        Logger::Info("[QuestSystem] Quest already active: " + questId);
        return;
    }
    
    quest->active = true;
    Logger::Info("[QuestSystem] Started quest: " + quest->title);
    
    if (onQuestStarted) {
        onQuestStarted(*quest);
    }
}

void QuestSystem::CompleteQuest(const std::string& questId) {
    Quest* quest = GetQuest(questId);
    if (!quest) {
        Logger::Error("[QuestSystem] Quest not found: " + questId);
        return;
    }
    
    if (!quest->active) {
        Logger::Error("[QuestSystem] Quest not active: " + questId);
        return;
    }
    
    if (!quest->IsComplete()) {
        Logger::Error("[QuestSystem] Quest not complete: " + questId);
        return;
    }
    
    quest->active = false;
    quest->completed = true;
    Logger::Info("[QuestSystem] Completed quest: " + quest->title);
    
    if (onQuestCompleted) {
        onQuestCompleted(*quest);
    }
}

void QuestSystem::AbandonQuest(const std::string& questId) {
    Quest* quest = GetQuest(questId);
    if (!quest) return;
    
    quest->active = false;
    // Reset progress
    for (auto& obj : quest->objectives) {
        obj.current = 0;
        obj.completed = false;
    }
    
    Logger::Info("[QuestSystem] Abandoned quest: " + quest->title);
}

void QuestSystem::UpdateProgress(QuestType type, const std::string& targetId, int amount) {
    for (auto& quest : quests) {
        if (!quest.active || quest.completed) continue;
        
        bool updated = false;
        for (auto& obj : quest.objectives) {
            if (obj.type == type && obj.targetId == targetId && !obj.completed) {
                obj.current += amount;
                if (obj.current >= obj.required) {
                    obj.current = obj.required;
                    obj.completed = true;
                    Logger::Info("[QuestSystem] Objective completed: " + obj.description);
                }
                updated = true;
                
                if (onObjectiveUpdated) {
                    onObjectiveUpdated(obj);
                }
            }
        }
        
        // Check if quest is complete
        if (updated && quest.IsComplete()) {
            CompleteQuest(quest.id);
        }
    }
}

void QuestSystem::UpdateObjective(const std::string& questId, int objectiveIndex, int progress) {
    Quest* quest = GetQuest(questId);
    if (!quest || !quest->active) return;
    if (objectiveIndex < 0 || objectiveIndex >= (int)quest->objectives.size()) return;
    
    QuestObjective& obj = quest->objectives[objectiveIndex];
    obj.current = progress;
    if (obj.current >= obj.required) {
        obj.current = obj.required;
        obj.completed = true;
    }
    
    if (onObjectiveUpdated) {
        onObjectiveUpdated(obj);
    }
    
    if (quest->IsComplete()) {
        CompleteQuest(questId);
    }
}

Quest* QuestSystem::GetQuest(const std::string& questId) {
    for (auto& quest : quests) {
        if (quest.id == questId) {
            return &quest;
        }
    }
    return nullptr;
}

std::vector<Quest*> QuestSystem::GetActiveQuests() {
    std::vector<Quest*> result;
    for (auto& quest : quests) {
        if (quest.active && !quest.completed) {
            result.push_back(&quest);
        }
    }
    return result;
}

std::vector<Quest*> QuestSystem::GetCompletedQuests() {
    std::vector<Quest*> result;
    for (auto& quest : quests) {
        if (quest.completed) {
            result.push_back(&quest);
        }
    }
    return result;
}

} // namespace vge
