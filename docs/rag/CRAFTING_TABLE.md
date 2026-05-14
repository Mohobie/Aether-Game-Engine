# Crafting Table

## Quick Reference

```cpp
// Place crafting table block
world.SetBlock(x, y, z, registry.GetBlockId("crafting_table"));

// Open crafting UI when right-clicking crafting table
void OnInteractWithBlock(BlockTypeID block, const Vec3& pos) {
    if (block == registry.GetBlockId("crafting_table")) {
        craftingUI.Open();
    }
}
```

## Features

### Block Properties
- **Type:** Interactive block
- **Size:** 1x1x1 block
- **Texture:** Wooden table with grid pattern
- **Transparent:** No
- **Solid:** Yes

### Crafting UI
- **3x3 Grid:** Larger crafting area
- **Recipe Book:** Shows available recipes
- **Output Slot:** Result of crafting
- **Inventory Integration:** Pulls from player inventory

### Recipes (3x3)
| Recipe | Ingredients | Output |
|--------|-------------|--------|
| Pickaxe | 3 planks top, 2 sticks middle | Wooden Pickaxe |
| Axe | 3 planks, 2 sticks | Wooden Axe |
| Sword | 2 planks, 1 stick | Wooden Sword |
| Chest | 8 planks | Chest |
| Furnace | 8 cobblestone | Furnace |

## Implementation

```cpp
// Register crafting table block
BlockDef craftingTable;
craftingTable.id = "crafting_table";
craftingTable.name = "Crafting Table";
craftingTable.interactive = true;
craftingTable.onInteract = [](Player& player) {
    player.OpenCraftingUI(3, 3); // 3x3 grid
};
registry.RegisterBlock(craftingTable);

// In game, place and use
world.SetBlock(x, y, z, registry.GetBlockId("crafting_table"));
```

## Files
- `src/core/crafting.h`
- `src/core/crafting.cpp`
