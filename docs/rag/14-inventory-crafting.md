# Inventory and Crafting System

The engine includes a comprehensive inventory and crafting system with drag-and-drop, item categories, and recipe matching.

## Item System

### Item Categories

Items are organized into categories for filtering and organization:

```cpp
enum class ItemCategory {
    Block,      // Building blocks (stone, dirt, planks)
    Tool,       // Tools (pickaxe, axe, shovel)
    Weapon,     // Weapons (sword, bow)
    Armor,      // Armor (helmet, chestplate, boots)
    Consumable, // Food, potions, medicine
    Material,   // Crafting materials (ore, ingot, stick)
    Misc        // Everything else
};
```

### Item Definition

```cpp
struct ItemDef {
    BlockTypeID blockType;      // Block type ID (BLOCK_AIR for non-block items)
    std::string itemID;         // Unique item string ID
    std::string displayName;    // Human-readable name
    std::string description;    // Tooltip description
    ItemCategory category;      // Item category
    int maxStackSize;           // Maximum stack size
    bool isBlock;               // Whether this item places a block
    std::string iconPath;       // Icon texture path
};
```

### Item Registry

```cpp
// Register items
vge::ItemRegistry::GetInstance().RegisterItem({
    .itemID = "iron_ore",
    .displayName = "Iron Ore",
    .category = vge::ItemCategory::Material,
    .maxStackSize = 64
});

// Query items
auto materials = vge::ItemRegistry::GetInstance().GetItemsByCategory(vge::ItemCategory::Material);
```

## Inventory System

### Creating Inventories

```cpp
vge::InventoryManager invManager;

// Create player inventory (9x4 = 36 slots)
vge::Inventory* playerInv = invManager.CreateInventory("player_inventory", 9, 4);

// Create hotbar (9x1)
vge::Inventory* hotbar = invManager.CreateInventory("player_hotbar", 9, 1);

// Create crafting grid (3x3)
vge::Inventory* craftingGrid = invManager.CreateInventory("crafting_grid", 3, 3);
```

### Item Operations

```cpp
// Add items
playerInv->AddItem("stone", 64);
playerInv->AddItem("wood", 32);

// Add to specific slot
playerInv->AddItemToSlot(0, "diamond", 5);

// Remove items
playerInv->RemoveItem("stone", 10);
playerInv->RemoveItem(5, 1);  // Remove 1 from slot 5

// Check items
bool hasStone = playerInv->HasItem("stone", 10);
int stoneCount = playerInv->GetItemCount("stone");

// Stack operations
playerInv->MergeStacks(0, 1);  // Merge slot 0 into slot 1
playerInv->SplitStack(0, 32);  // Split 32 items from slot 0

// Move items
playerInv->SwapSlots(0, 1);
playerInv->MoveItem(0, 5);
```

### Filtering

```cpp
// Show only materials
playerInv->SetFilter(vge::ItemCategory::Material);

// Clear filter
playerInv->ClearFilter();

// Get filtered slots
auto filteredSlots = playerInv->GetFilteredSlots();
```

## Drag and Drop

```cpp
// Start drag
invManager.StartDrag("player_inventory", 5);

// Update drag position (for visual feedback)
invManager.UpdateDragPosition(mousePos);

// End drag (drop)
invManager.EndDrag("player_hotbar", 2);

// Cancel drag
invManager.CancelDrag();

// Check if dragging
if (invManager.IsDragging()) {
    // Render dragged item at mouse position
}
```

## Crafting System

### Defining Recipes

```cpp
// Shapeless recipe (ingredients can be in any order)
vge::CraftingRecipe plankRecipe;
plankRecipe.recipeID = "planks_from_wood";
plankRecipe.displayName = "Planks";
plankRecipe.outputItemID = "planks";
plankRecipe.outputCount = 4;
plankRecipe.shapeless = true;
plankRecipe.ingredients.push_back({"wood", 1});

// Shaped recipe (3x3 grid pattern)
vge::CraftingRecipe pickaxeRecipe;
pickaxeRecipe.recipeID = "stone_pickaxe";
pickaxeRecipe.displayName = "Stone Pickaxe";
pickaxeRecipe.outputItemID = "stone_pickaxe";
pickaxeRecipe.outputCount = 1;
pickaxeRecipe.shapeless = false;
pickaxeRecipe.pattern = {
    {"stone", "stone", "stone"},
    {"", "stick", ""},
    {"", "stick", ""}
};

// Register recipes
vge::CraftingSystem craftingSystem;
craftingSystem.AddRecipe(plankRecipe);
craftingSystem.AddRecipe(pickaxeRecipe);
```

### Recipe Ingredients with Substitutes

```cpp
// Ingredient that accepts multiple item types
vge::RecipeIngredient ingredient;
ingredient.itemID = "wood";
ingredient.AddSubstitute("oak_wood");
ingredient.AddSubstitute("birch_wood");
ingredient.AddSubstitute("spruce_wood");
ingredient.count = 1;
```

### Crafting Operations

```cpp
// Try to craft from grid
vge::CraftingGrid grid;
grid.SetItem(0, 0, "stone");
grid.SetItem(1, 0, "stone");
grid.SetItem(2, 0, "stone");
grid.SetItem(1, 1, "stick");
grid.SetItem(1, 2, "stick");

vge::CraftingResult2 result = craftingSystem.TryCraft(grid, playerInv);
if (result.success) {
    // Crafting succeeded
    std::cout << "Crafted: " << result.outputItemID << std::endl;
}

// Check if recipe can be crafted
if (craftingSystem.CanCraftRecipe(*playerInv, "stone_pickaxe")) {
    // Player has ingredients
}

// Get missing ingredients
auto missing = craftingSystem.GetMissingIngredients(pickaxeRecipe, *playerInv);
for (const auto& [itemID, count] : missing) {
    std::cout << "Need " << count << " more " << itemID << std::endl;
}
```

### Crafting Queue

```cpp
vge::CraftingQueue queue;

// Queue multiple crafts
queue.Enqueue("planks", 4);  // Craft 4 planks
queue.Enqueue("sticks", 8);  // Craft 8 sticks

// Update queue (call every frame)
queue.Update(deltaTime);

// Check progress
if (queue.IsProcessing()) {
    float progress = queue.GetCurrentProgress();
    std::cout << "Crafting progress: " << (progress * 100) << "%" << std::endl;
}

// Callbacks
queue.SetOnCraftComplete([](const vge::CraftingQueueEntry& entry) {
    std::cout << "Crafted " << entry.quantity << "x " << entry.recipeID << std::endl;
});
```

## Smelting System

```cpp
vge::SmeltingSystem smelting(&craftingSystem);

// Register fuel types
smelting.RegisterFuel("coal", 80.0f);    // 80 seconds burn time
smelting.RegisterFuel("lava_bucket", 1000.0f);

// Add input and fuel
smelting.AddInput(0, "iron_ore", 1);
smelting.AddFuel(0, "coal", 1);

// Start smelting
smelting.StartSmelting(0);

// Update (call every frame)
smelting.Update(deltaTime);

// Check progress
if (smelting.IsSmelting(0)) {
    float progress = smelting.GetSmeltProgress(0);
}

// Get output
if (smelting.GetSlot(0).HasOutput()) {
    auto output = smelting.GetSlot(0).outputItemID;
    int count = smelting.GetSlot(0).outputCount;
}
```

## Recipe Book

```cpp
vge::RecipeBook recipeBook;

// Add recipes
recipeBook.addRecipe(plankRecipe);
recipeBook.addRecipe(pickaxeRecipe);

// Filter by category
recipeBook.setCategoryFilter(vge::ItemCategory::Tool);

// Search
recipeBook.setSearchQuery("pick");

// Get available recipes (can craft with current inventory)
auto available = recipeBook.getFilteredRecipes();

// Check if specific recipe can be crafted
bool canCraft = recipeBook.canCraft(pickaxeRecipe, *playerInv);

// Get missing ingredients
auto missing = recipeBook.getMissingIngredients(pickaxeRecipe, *playerInv);
```

## Crafting Bridge (Inventory <-> Crafting)

```cpp
vge::CraftingInventoryBridge bridge(&invManager, &craftingSystem);
bridge.initialize();

// Set inventories
bridge.setCraftingInventory("crafting_grid");
bridge.setResultInventory("crafting_result");

// Check if current grid matches recipe
vge::CraftingResult result;
if (bridge.checkRecipeMatch(result)) {
    // Recipe matched
}

// Perform craft
if (bridge.craftItem(result)) {
    // Success
}

// Auto-fill recipe
bridge.autoFillRecipe(pickaxeRecipe);

// Clear grid
bridge.clearCraftingGrid();
```

## UI Integration

```cpp
// Create inventory UI
vge::UIInventoryController uiController(&invManager);
uiController.initialize();

// Open/close inventory
uiController.toggleInventory();

// Handle input
uiController.handleMouseClick(mousePos, false);  // Left click
uiController.handleMouseClick(mousePos, true);   // Right click
uiController.handleMouseMove(mousePos);
uiController.handleKeyPress('E');

// Render
uiController.render();

// Create crafting panel
vge::UICraftingPanel craftingPanel(&invManager);
craftingPanel.initialize();
craftingPanel.tryCraft();
```

## Serialization

```cpp
// Save inventory
std::string data = playerInv->Serialize();

// Load inventory
playerInv->Deserialize(data);

// Save all inventories
std::string allData = invManager.SerializeAll();

// Load all inventories
invManager.DeserializeAll(allData);
```

## Block-to-Item Mapping

The engine automatically maps block types to item categories based on block ID patterns:

```cpp
// These patterns are automatically categorized:
// - "ore", "ingot", "gem", "coal", "iron", "gold", "diamond" -> Material
// - "food", "meat", "fruit", "vegetable", "bread", "apple" -> Consumable
// - "sword", "axe", "pickaxe", "shovel", "hoe", "bow" -> Weapon
// - "armor", "helmet", "chestplate", "leggings", "boots", "shield" -> Armor
// - "plank", "stone", "brick", "glass", "wood", "dirt", "sand" -> Block
// - "stick", "string", "feather", "leather", "wool" -> Material
```

## Best Practices

1. **Use item IDs consistently** - Always use string IDs, not block type IDs
2. **Set appropriate stack sizes** - Tools/weapons: 1, Blocks/materials: 64
3. **Use categories** - Helps with inventory organization and filtering
4. **Validate recipes** - Always check CanCraft before crafting
5. **Handle drag-and-drop** - Provide visual feedback during drag operations
