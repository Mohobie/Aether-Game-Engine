#include "game/quest_manager.hpp"

namespace game {

void Quest::updateObjective(size_t idx, uint32_t delta) {
    if (idx >= objectives.size()) return;
    objectives[idx].current += delta;
}

bool Quest::isComplete() const {
    for (const auto& o : objectives) if (!o.complete()) return false;
    return true;
}

void Quest::complete() {
    if (isComplete()) status = QuestStatus::Completed;
}

void QuestManager::addQuest(Quest quest) {
    quests.push_back(std::move(quest));
}

Quest* QuestManager::get(const std::string& id) {
    for (auto& q : quests) if (q.id == id) return &q;
    return nullptr;
}

std::vector<Quest*> QuestManager::activeQuests() {
    std::vector<Quest*> out;
    for (auto& q : quests) if (q.status == QuestStatus::Active) out.push_back(&q);
    return out;
}

std::vector<Quest*> QuestManager::completedQuests() {
    std::vector<Quest*> out;
    for (auto& q : quests) if (q.status == QuestStatus::Completed) out.push_back(&q);
    return out;
}

} // namespace game
