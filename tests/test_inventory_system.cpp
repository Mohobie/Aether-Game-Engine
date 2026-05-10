#include "core/item_system.h"
#include "core/crafting_inventory_bridge.h"
#include "ui/ui_inventory.h"
#include <iostream>
#include <cassert>

using namespace vge;

void TestItemRegistry() {
    std::cout << "\n=== Item Registry Test ===" << std::endl;
    
    ItemRegistry& registry = ItemRegistry::GetInstance();
    registry.Clear();
    
    // Register test items
    ItemDef stone;
    stone.itemID = "stone";
    stone.displayName = "Stone";
    stone.description = "A solid rock";
    stone.category = ItemCategory::Block;
    stone.maxStackSize = 64;
    stone.isBlock = true;
    registry.RegisterItem(stone);
    
    ItemDef pickaxe;
    pickaxe.itemID = "pickaxe";
    pickaxe.displayName = "Iron Pickaxe";
    pickaxe.description = "Mines blocks faster";
    pickaxe.category = ItemCategory::Tool;
    pickaxe.maxStackSize = 1;
    pickaxe.isBlock = false;
    registry.RegisterItem(pickaxe);
    
    ItemDef apple;
    apple.itemID = "apple";
    apple.displayName = "Apple";
    apple.description = "Restores hunger";
    apple.category = ItemCategory::Consumable;
    apple.maxStackSize = 16;
    apple.isBlock = false;
    registry.RegisterItem(apple);
    
    // Verify registration
    assert(registry.HasItem("stone"));
    assert(registry.HasItem("pickaxe"));
    assert(registry.HasItem("apple"));
    assert(!registry.HasItem("diamond"));
    
    // Verify item properties
    const ItemDef& stoneDef = registry.GetItem("stone");
    assert(stoneDef.maxStackSize == 64);
    assert(stoneDef.category == ItemCategory::Block);
    
    const ItemDef& pickaxeDef = registry.GetItem("pickaxe");
    assert(pickaxeDef.maxStackSize == 1);
    assert(pickaxeDef.category == ItemCategory::Tool);
    
    // Test category filtering
    auto blocks = registry.GetItemsByCategory(ItemCategory::Block);
    assert(blocks.size() == 1);
    assert(blocks[0] == "stone");
    
    auto tools = registry.GetItemsByCategory(ItemCategory::Tool);
    assert(tools.size() == 1);
    assert(tools[0] == "pickaxe");
    
    std::cout << "Item registry test passed!" << std::endl;
}

void TestInventory() {
    std::cout << "\n=== Inventory Test ===" << std::endl;
    
    // Set up item registry
    ItemRegistry& registry = ItemRegistry::GetInstance();
    registry.Clear();
    
    ItemDef stone;
    stone.itemID = "stone";
    stone.displayName = "Stone";
    stone.category = ItemCategory::Block;
    stone.maxStackSize = 64;
    registry.RegisterItem(stone);
    
    ItemDef dirt;
    dirt.itemID = "dirt";
    dirt.displayName = "Dirt";
    dirt.category = ItemCategory::Block;
    dirt.maxStackSize = 64;
    registry.RegisterItem(dirt);
    
    // Create inventory
    Inventory inv("test", 9, 4); // 9x4 = 36 slots
    assert(inv.GetWidth() == 9);
    assert(inv.GetHeight() == 4);
    assert(inv.GetTotalSlots() == 36);
    
    // Test adding items
    assert(inv.AddItem("stone", 32));
    assert(inv.GetItemCount("stone") == 32);
    
    // Test stacking
    assert(inv.AddItem("stone", 32));
    assert(inv.GetItemCount("stone") == 64);
    
    // Test overflow (should create new stack)
    assert(inv.AddItem("stone", 10));
    assert(inv.GetItemCount("stone") == 74);
    
    // Test adding different item
    assert(inv.AddItem("dirt", 10));
    assert(inv.GetItemCount("dirt") == 10);
    
    // Test HasItem
    assert(inv.HasItem("stone", 74));
    assert(!inv.HasItem("stone", 75));
    
    // Test slot operations
    assert(inv.GetSlot(0).item.count == 64); // First stack full
    assert(inv.GetSlot(1).item.count == 10);  // Second stack
    
    // Test RemoveItem
    assert(inv.RemoveItem("stone", 10));
    assert(inv.GetItemCount("stone") == 64);
    
    // Test slot removal
    assert(inv.RemoveItem(0, 64));
    assert(inv.GetSlot(0).IsEmpty());
    
    // Test slot locking
    inv.LockSlot(5);
    assert(inv.IsSlotLocked(5));
    
    ItemDef gem;
    gem.itemID = "gem";
    gem.maxStackSize = 64;
    registry.RegisterItem(gem);
    
    // Should not be able to add to locked slot
    // (AddItem will skip locked slots)
    int countBefore = inv.GetItemCount("gem");
    inv.AddItem("gem", 1);
    // Item should go to first available unlocked slot
    
    inv.UnlockSlot(5);
    assert(!inv.IsSlotLocked(5));
    
    // Test swap
    inv.Clear();
    inv.AddItem("stone", 10);
    inv.AddItem("dirt", 20);
    assert(inv.SwapSlots(0, 1));
    assert(inv.GetSlot(0).item.itemID == "dirt");
    assert(inv.GetSlot(1).item.itemID == "stone");
    
    // Test merge
    inv.Clear();
    inv.AddItemToSlot(0, "stone", 30);
    inv.AddItemToSlot(1, "stone", 20);
    assert(inv.MergeStacks(0, 1));
    assert(inv.GetSlot(1).item.count == 50);
    assert(inv.GetSlot(0).IsEmpty() || inv.GetSlot(0).item.count == 0);
    
    // Test split
    inv.Clear();
    inv.AddItemToSlot(0, "stone", 50);
    assert(inv.SplitStack(0, 10));
    assert(inv.GetSlot(0).item.count == 40);
    bool foundSplit = false;
    for (int i = 1; i < inv.GetTotalSlots(); ++i) {
        if (!inv.GetSlot(i).IsEmpty() && inv.GetSlot(i).item.count == 10) {
            foundSplit = true;
            break;
        }
    }
    assert(foundSplit);
    
    // Test serialization
    std::string serialized = inv.Serialize();
    assert(!serialized.empty());
    assert(serialized.find("test") != std::string::npos);
    
    // Test filtering
    inv.ClearFilter();
    assert(!inv.IsFilterEnabled());
    
    inv.SetFilter(ItemCategory::Block);
    assert(inv.IsFilterEnabled());
    assert(inv.GetFilter() == ItemCategory::Block);
    
    std::cout << "Inventory test passed!" << std::endl;
}

void TestInventoryManager() {
    std::cout << "\n=== Inventory Manager Test ===" << std::endl;
    
    // Set up item registry
    ItemRegistry& registry = ItemRegistry::GetInstance();
    registry.Clear();
    
    ItemDef stone;
    stone.itemID = "stone";
    stone.maxStackSize = 64;
    registry.RegisterItem(stone);
    
    ItemDef wood;
    wood.itemID = "wood";
    wood.maxStackSize = 64;
    registry.RegisterItem(wood);
    
    InventoryManager manager;
    
    // Create inventories
    Inventory* playerInv = manager.CreateInventory("player", 9, 4);
    assert(playerInv != nullptr);
    assert(playerInv->GetTotalSlots() == 36);
    
    Inventory* chestInv = manager.CreateInventory("chest", 9, 3);
    assert(chestInv != nullptr);
    assert(chestInv->GetTotalSlots() == 27);
    
    // Test HasInventory
    assert(manager.HasInventory("player"));
    assert(manager.HasInventory("chest"));
    assert(!manager.HasInventory("nonexistent"));
    
    // Test GetInventory
    assert(manager.GetInventory("player") == playerInv);
    assert(manager.GetInventory("chest") == chestInv);
    
    // Add items to player inventory
    playerInv->AddItem("stone", 10);
    assert(playerInv->GetItemCount("stone") == 10);
    
    // Test transfer
    assert(manager.TransferItem("player", 0, "chest", 0));
    assert(playerInv->GetSlot(0).IsEmpty());
    assert(chestInv->GetSlot(0).item.itemID == "stone");
    assert(chestInv->GetSlot(0).item.count == 10);
    
    // Test active inventory
    manager.SetActiveInventory("player");
    assert(manager.GetActiveInventory() == playerInv);
    
    manager.ClearActiveInventory();
    assert(manager.GetActiveInventory() == nullptr);
    
    // Test drag and drop
    playerInv->Clear();
    playerInv->AddItem("wood", 5);
    
    manager.SetActiveInventory("player");
    assert(manager.StartDrag("player", 0));
    assert(manager.IsDragging());
    assert(manager.GetDraggedItem().itemID == "wood");
    assert(manager.GetDraggedItem().count == 5);
    assert(playerInv->GetSlot(0).IsEmpty()); // Source cleared
    
    // End drag to another slot
    assert(manager.EndDrag("player", 5));
    assert(!manager.IsDragging());
    assert(playerInv->GetSlot(5).item.itemID == "wood");
    assert(playerInv->GetSlot(5).item.count == 5);
    
    // Test cancel drag
    playerInv->Clear();
    playerInv->AddItem("stone", 10);
    manager.StartDrag("player", 0);
    manager.CancelDrag();
    assert(!manager.IsDragging());
    assert(playerInv->GetSlot(0).item.itemID == "stone");
    assert(playerInv->GetSlot(0).item.count == 10);
    
    // Test crafting integration
    playerInv->Clear();
    playerInv->AddItem("wood", 4);
    
    std::vector<std::pair<std::string, int>> ingredients = {{"wood", 2}};
    assert(manager.ConsumeItemsForCrafting("player", ingredients));
    assert(playerInv->GetItemCount("wood") == 2);
    
    assert(manager.AddCraftingResult("player", "stone", 1));
    assert(playerInv->GetItemCount("stone") == 1);
    
    // Test serialization
    std::string serialized = manager.SerializeAll();
    assert(!serialized.empty());
    
    // Test DestroyInventory
    manager.DestroyInventory("chest");
    assert(!manager.HasInventory("chest"));
    
    std::cout << "Inventory manager test passed!" << std::endl;
}

void TestCraftingIntegration() {
    std::cout << "\n=== Crafting Integration Test ===" << std::endl;
    
    // Set up item registry with block types
    ItemRegistry& registry = ItemRegistry::GetInstance();
    registry.Clear();
    
    ItemDef wood;
    wood.itemID = "wood";
    wood.blockType = 1; // Assuming block type 1
    wood.maxStackSize = 64;
    registry.RegisterItem(wood);
    
    ItemDef planks;
    planks.itemID = "planks";
    planks.blockType = 2; // Assuming block type 2
    planks.maxStackSize = 64;
    registry.RegisterItem(planks);
    
    // Create inventory manager and crafting system
    InventoryManager invManager;
    CraftingSystem craftSystem;
    
    CraftingInventoryBridge bridge(&invManager, &craftSystem);
    assert(bridge.initialize());
    
    // Create crafting inventories
    Inventory* craftingInv = invManager.CreateInventory("crafting", 3, 3);
    Inventory* resultInv = invManager.CreateInventory("result", 1, 1);
    Inventory* playerInv = invManager.CreateInventory("player", 9, 4);
    
    bridge.setCraftingInventory("crafting");
    bridge.setResultInventory("result");
    
    // Add ingredients to player inventory
    playerInv->AddItem("wood", 5);
    
    // Test hasIngredients
    // Note: This requires actual recipes in CraftingSystem
    // For now, just test the bridge functions exist and work
    
    // Test clearCraftingGrid
    craftingInv->AddItem("wood", 1);
    bridge.clearCraftingGrid();
    assert(craftingInv->GetSlot(0).IsEmpty());
    
    // Test autoFillRecipe (requires valid recipe)
    // bridge.autoFillRecipe(recipe);
    
    // Test available recipes
    auto recipes = bridge.getAvailableRecipes("player");
    // Should be empty since we don't have matching ingredients for default recipes
    
    std::cout << "Crafting integration test passed!" << std::endl;
}

void TestInventoryTooltip() {
    std::cout << "\n=== Inventory Tooltip Test ===" << std::endl;
    
    // Set up item registry
    ItemRegistry& registry = ItemRegistry::GetInstance();
    registry.Clear();
    
    ItemDef sword;
    sword.itemID = "iron_sword";
    sword.displayName = "Iron Sword";
    sword.description = "A sharp blade for combat";
    sword.category = ItemCategory::Weapon;
    sword.maxStackSize = 1;
    registry.RegisterItem(sword);
    
    // Test tooltip generation
    std::string tooltip = InventoryTooltip::GenerateTooltipText("iron_sword");
    assert(!tooltip.empty());
    assert(tooltip.find("Iron Sword") != std::string::npos);
    assert(tooltip.find("Weapon") != std::string::npos);
    assert(tooltip.find("sharp blade") != std::string::npos);
    
    // Test tooltip state
    InventoryTooltip tooltipUI;
    assert(!tooltipUI.IsVisible());
    
    tooltipUI.Show("iron_sword", Vec2(100, 100));
    // Should not be visible immediately (has delay)
    assert(!tooltipUI.IsVisible());
    
    // Simulate time passing
    tooltipUI.Update(1.0f);
    assert(tooltipUI.IsVisible());
    
    tooltipUI.Hide();
    assert(!tooltipUI.IsVisible());
    
    std::cout << "Inventory tooltip test passed!" << std::endl;
}

void TestUIInventory() {
    std::cout << "\n=== UI Inventory Test ===" << std::endl;
    
    // Create inventory manager
    InventoryManager manager;
    manager.CreateInventory("player_inventory", 9, 4);
    manager.CreateInventory("player_hotbar", 9, 1);
    manager.SetActiveInventory("player_inventory");
    
    // Create UI controller
    UIInventoryController controller(&manager);
    assert(controller.initialize());
    
    // Test open/close
    assert(!controller.isOpen());
    controller.openInventory();
    assert(controller.isOpen());
    controller.closeInventory();
    assert(!controller.isOpen());
    controller.toggleInventory();
    assert(controller.isOpen());
    
    // Test filter
    controller.setCategoryFilter(ItemCategory::Block);
    controller.clearCategoryFilter();
    
    // Test key handling
    assert(controller.handleKeyPress('E'));
    assert(!controller.isOpen()); // Should toggle off
    
    assert(controller.handleKeyPress('I'));
    assert(controller.isOpen()); // Should toggle on
    
    // Test number keys
    assert(controller.handleKeyPress('1'));
    assert(controller.handleKeyPress('5'));
    assert(controller.handleKeyPress('9'));
    
    controller.shutdown();
    
    std::cout << "UI inventory test passed!" << std::endl;
}

void TestRecipeBook() {
    std::cout << "\n=== Recipe Book Test ===" << std::endl;
    
    RecipeBook recipeBook;
    
    // Add test recipes
    CraftingRecipe recipe1;
    recipe1.outputType = 1;
    recipe1.outputCount = 4;
    recipeBook.addRecipe(recipe1);
    
    CraftingRecipe recipe2;
    recipe2.outputType = 2;
    recipe2.outputCount = 1;
    recipeBook.addRecipe(recipe2);
    
    assert(recipeBook.getAllRecipes().size() == 2);
    
    // Test filtering
    recipeBook.setSearchQuery("planks");
    auto filtered = recipeBook.getFilteredRecipes();
    // Results depend on block names
    
    recipeBook.clearFilters();
    assert(recipeBook.getFilteredRecipes().size() == 2);
    
    // Test category filter
    recipeBook.setCategoryFilter(ItemCategory::Block);
    filtered = recipeBook.getFilteredRecipes();
    
    std::cout << "Recipe book test passed!" << std::endl;
}

int main() {
    std::cout << "=== Aether Game Engine - Inventory System Tests ===" << std::endl;
    
    TestItemRegistry();
    TestInventory();
    TestInventoryManager();
    TestCraftingIntegration();
    TestInventoryTooltip();
    TestUIInventory();
    TestRecipeBook();
    
    std::cout << "\n=== All Inventory Tests Passed ===" << std::endl;
    
    return 0;
}
