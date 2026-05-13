# Crafting System

## Quick Reference

```cpp
vge::CraftingSystem craftingSystem;

// Create a recipe
vge::CraftingRecipe recipe;
recipe.recipeID = "planks";
recipe.displayName = "Wooden Planks";
recipe.outputItemID = "wood";
recipe.outputCount = 4;
recipe.shapeless = true;
recipe.ingredients.push_back(vge::RecipeIngredient("log", 1));

craftingSystem.AddRecipe(recipe);

// Try to craft
vge::CraftingGrid grid;
grid.SetItem(0, 0, "log");

auto result = craftingSystem.TryCraft(grid);
if (result.success) {
    // Crafted result.outputCount of result.outputItemID
}
```

## Features

### Recipe Types
- **Shaped:** Items must be in specific pattern (3x3 grid)
- **Shapeless:** Items just need to be present, any arrangement
- **Station Required:** Some recipes need crafting table/furnace

### Crafting Grid
- 3x3 grid for crafting
- 2x2 grid for inventory crafting
- Supports empty slots

### Recipe Properties
- **Ingredients:** Required items with quantities
- **Substitutes:** Alternative items that work (e.g., oak/birch wood)
- **Output:** Result item and count
- **Craft Time:** Instant or timed crafting
- **XP Reward:** Experience given on craft
- **Level Requirement:** Player level needed

## Default Recipes

| Recipe | Ingredients | Output |
|--------|-------------|--------|
| Planks | 1 Log | 4 Planks |
| Sticks | 2 Planks | 4 Sticks |
| Crafting Table | 4 Planks | 1 Crafting Table |
| Wooden Pickaxe | 3 Planks + 2 Sticks | 1 Pickaxe |
| Stone Pickaxe | 3 Cobblestone + 2 Sticks | 1 Pickaxe |
| Iron Pickaxe | 3 Iron Ingot + 2 Sticks | 1 Pickaxe |

## Smelting

```cpp
vge::SmeltingSystem smelting(&craftingSystem);

// Add fuel types
smelting.RegisterFuel("coal", 80.0f);  // 80 seconds burn time
smelting.RegisterFuel("wood", 15.0f);

// Add smelting recipe
vge::SmeltingRecipe ironSmelt;
ironSmelt.inputItemID = "iron_ore";
ironSmelt.outputItemID = "iron_ingot";
ironSmelt.smeltTime = 10.0f;
craftingSystem.AddSmeltingRecipe(ironSmelt);

// Use furnace
smelting.AddInput(0, "iron_ore", 1);
smelting.AddFuel(0, "coal", 1);
smelting.StartSmelting(0);

// Update in game loop
smelting.Update(deltaTime);
```

## Recipe Discovery

```cpp
vge::RecipeProgression progression;

// Recipes can be hidden until discovered
recipe.hidden = true;

// Discover by obtaining items
progression.RegisterItemTrigger("iron_ore", "iron_pickaxe_recipe");
progression.OnItemObtained("iron_ore");  // Unlocks recipe

// Check if discovered
bool canCraft = progression.IsDiscovered(recipe.recipeID);
```

## Files
- `src/core/crafting.h`
- `src/core/crafting.cpp`
