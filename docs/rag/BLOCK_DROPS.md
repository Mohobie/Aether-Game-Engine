# Block Drops

## Quick Reference

```cpp
vge::BlockDropSystem drops;

// Get drops for a block
auto items = drops.GetDrops("stone", "pickaxe", 1);
for (const auto& [itemId, count] : items) {
    inventory.AddItem(itemId, count);
}

// Check if can harvest with current tool
bool canHarvest = drops.CanHarvest("iron_ore", "pickaxe", 2);
```

## Features

### Drop Properties
- **Item ID:** What item is dropped
- **Count:** Min/max amount
- **Chance:** Probability (0.0-1.0)
- **Tool Required:** Needs specific tool type
- **Tool Tier:** Minimum tool material tier

### Default Drops

| Block | Drops | Tool Required | Tier |
|-------|-------|--------------|------|
| Stone | Cobblestone | Pickaxe | Wood (0) |
| Coal Ore | Coal | Pickaxe | Wood (0) |
| Iron Ore | Raw Iron | Pickaxe | Stone (1) |
| Gold Ore | Raw Gold | Pickaxe | Iron (2) |
| Diamond Ore | Diamond | Pickaxe | Iron (2) |
| Emerald Ore | Emerald | Pickaxe | Iron (2) |
| Wood | Wood | Axe | Wood (0) |
| Leaves | Sapling (20%) | None | - |
| Grass | Dirt | None | - |
| Dirt | Dirt | None | - |
| Sand | Sand | Shovel | Wood (0) |
| Gravel | Gravel/Flint | Shovel | Wood (0) |
| Glass | Nothing | - | - |
| Flower | Flower | None | - |
| Cactus | Cactus | None | - |

## Tool Requirements

```cpp
// Stone requires pickaxe (any tier)
auto stoneDrops = drops.GetDrops("stone", "pickaxe", 0); // Works
auto stoneDrops2 = drops.GetDrops("stone", "pickaxe", 2); // Works
auto stoneDrops3 = drops.GetDrops("stone", "axe", 0); // Empty - wrong tool
auto stoneDrops4 = drops.GetDrops("stone", "", 0); // Empty - no tool

// Iron ore requires stone pickaxe+
auto ironDrops = drops.GetDrops("iron_ore", "pickaxe", 0); // Empty - tier too low
auto ironDrops2 = drops.GetDrops("iron_ore", "pickaxe", 1); // Works
```

## Custom Drops

```cpp
// Register custom drops
vge::BlockDrop customDrop;
customDrop.itemId = "custom_item";
customDrop.minCount = 1;
customDrop.maxCount = 3;
customDrop.chance = 0.5f;
customDrop.requiresTool = true;
customDrop.requiredToolType = "pickaxe";
customDrop.requiredToolTier = 2; // Iron+

drops.RegisterDrops("custom_block", {customDrop});
```

## Files
- `src/game/block_drops.h`
- `src/game/block_drops.cpp`
