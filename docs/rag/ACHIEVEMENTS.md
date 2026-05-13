# Achievements System

## Quick Reference

```cpp
vge::AchievementManager achievements;
achievements.Initialize();

// Register custom achievement
achievements.RegisterAchievement("miner", "Miner", "Mine 100 blocks", vge::AchievementType::MineBlock, 100);

// Update progress
achievements.Increment(vge::AchievementType::MineBlock);

// Check if unlocked
auto* ach = achievements.GetAchievement("miner");
if (ach && ach->IsUnlocked()) {
    std::cout << "Achievement unlocked: " << ach->GetName() << std::endl;
}
```

## Features

### Achievement Types
| Type | Description |
|------|-------------|
| MineBlock | Mining blocks |
| PlaceBlock | Placing blocks |
| Walk | Walking distance |
| SurviveDays | Surviving days |
| ReachDepth | Reaching depth |
| ReachHeight | Reaching height |
| FindDiamond | Finding diamonds |
| CraftItem | Crafting items |
| KillEnemy | Killing enemies |
| Custom | Custom criteria |

### Default Achievements
- **Getting Wood:** Mine your first log
- **Benchmarking:** Craft a crafting table
- **Time to Mine!:** Use planks and sticks to make a pickaxe
- **Hot Topic:** Construct a furnace out of eight cobblestone blocks
- **Acquire Hardware:** Smelt an iron ingot
- **DIAMONDS!:** Acquire diamonds
- **Enchanter:** Enchant an item
- **The End?:** Enter an end portal

## Implementation

```cpp
vge::AchievementManager achievements;
achievements.Initialize();

// Set callback
achievements.onAchievementUnlocked = [](const vge::Achievement& ach) {
    std::cout << "🏆 Achievement Unlocked: " << ach.GetName() << " - " << ach.GetDescription() << std::endl;
};

// Update progress
achievements.Increment(vge::AchievementType::MineBlock);
achievements.UpdateProgress(vge::AchievementType::Walk, 100);

// Save/load
achievements.Save("achievements.dat");
achievements.Load("achievements.dat");
```

## Files
- `src/core/achievements.h`
- `src/core/achievements.cpp`
