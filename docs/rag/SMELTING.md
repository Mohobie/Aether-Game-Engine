# Smelting System

## Quick Reference

```cpp
vge::FurnaceManager furnaces(&craftingSystem);

// Place furnace
furnaces.PlaceFurnace(x, y, z);

// Add items
furnaces.AddInput(x, y, z, "iron_ore", 1);
furnaces.AddFuel(x, y, z, "coal", 1);

// Update
furnaces.Update(deltaTime);

// Collect output
furnaces.RemoveOutput(x, y, z, 1);
float xp = furnaces.CollectExperience(x, y, z);
```

## Features

### Furnace Block
- Placeable block with state
- Lit/unlit visual states
- Persistent inventory

### Smelting Recipes
| Input | Output | Time | XP |
|-------|--------|------|-----|
| Iron Ore | Iron Ingot | 10s | 2 |
| Gold Ore | Gold Ingot | 10s | 3 |
| Coal Ore | Coal | 5s | 1 |
| Diamond Ore | Diamond | 15s | 5 |

### Fuel Types
| Fuel | Burn Time |
|------|-----------|
| Coal | 80s |
| Charcoal | 80s |
| Wood | 15s |
| Planks | 15s |
| Stick | 5s |
| Lava Bucket | 1000s |
| Coal Block | 800s |

## Implementation

```cpp
// Initialize
vge::FurnaceManager furnaces(&craftingSystem);
FurnaceManager::RegisterDefaultSmeltingRecipes(&craftingSystem);

// Place furnace in world
furnaces.PlaceFurnace(10, 20, 30);

// Add iron ore and coal
furnaces.AddInput(10, 20, 30, "iron_ore", 1);
furnaces.AddFuel(10, 20, 30, "coal", 1);

// In game loop
void Update(float deltaTime) {
    furnaces.Update(deltaTime);
}

// Check progress
float progress = furnaces.GetSmeltProgress(10, 20, 30);
bool isLit = furnaces.IsLit(10, 20, 30);

// Collect results
if (progress >= 1.0f) {
    furnaces.RemoveOutput(10, 20, 30, 1);
    float xp = furnaces.CollectExperience(10, 20, 30);
    player.AddExperience(xp);
}
```

## Files
- `src/game/furnace_system.h`
- `src/game/furnace_system.cpp`
