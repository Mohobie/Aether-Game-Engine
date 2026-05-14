# Quest System

## Quick Reference

```cpp
vge::QuestSystem quests;

// Register a quest
vge::Quest quest;
quest.id = "first_quest";
quest.title = "Getting Started";
quest.description = "Gather basic resources";

vge::QuestObjective obj;
obj.description = "Collect 10 wood";
obj.type = vge::QuestType::Collect;
obj.targetId = "wood";
obj.required = 10;
quest.objectives.push_back(obj);

quests.RegisterQuest(quest);
quests.StartQuest("first_quest");

// Update progress
quests.UpdateProgress(vge::QuestType::Collect, "wood", 1);
```

## Features

### Quest Types
| Type | Description | Example |
|------|-------------|---------|
| Collect | Gather items | Collect 10 wood |
| Kill | Defeat mobs | Kill 5 zombies |
| Explore | Visit locations | Find the cave |
| Craft | Craft items | Craft a pickaxe |
| Survive | Survive time | Survive 1 night |
| Escort | Protect NPC | Escort villager |
| Custom | Custom objective | Any custom goal |

### Quest States
- **Inactive:** Not started
- **Active:** In progress
- **Completed:** All objectives done

### Objectives
- Multiple objectives per quest
- Progress tracking
- Auto-completion when requirements met

## Implementation

```cpp
// Create quest system
vge::QuestSystem questSystem;

// Set callbacks
questSystem.onQuestStarted = [](const vge::Quest& q) {
    std::cout << "Quest started: " << q.title << std::endl;
};

questSystem.onQuestCompleted = [](const vge::Quest& q) {
    std::cout << "Quest completed: " << q.title << std::endl;
    // Give rewards
};

// Register and start quest
questSystem.RegisterQuest(quest);
questSystem.StartQuest("first_quest");

// Update progress in game loop
void OnBlockMined(const std::string& blockId) {
    questSystem.UpdateProgress(vge::QuestType::Collect, blockId, 1);
}

void OnMobKilled(const std::string& mobId) {
    questSystem.UpdateProgress(vge::QuestType::Kill, mobId, 1);
}
```

## Files
- `src/game/quest_system.h`
- `src/game/quest_system.cpp`
