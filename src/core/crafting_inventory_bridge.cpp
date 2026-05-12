#include "crafting_inventory_bridge.h"
#include "core/item_system.h"
#include "core/logger.h"
#include "voxel/block_registry.h"
#include <algorithm>

namespace vge {

// ==================== CraftingInventoryBridge ====================

CraftingInventoryBridge::CraftingInventoryBridge(InventoryManager* invManager, CraftingSystem* craftSystem)
    : inventoryManager(invManager), craftingSystem(craftSystem) {
}

bool CraftingInventoryBridge::initialize() {
    if (!inventoryManager || !craftingSystem) {
        Logger::Error("[CraftingInventoryBridge] Invalid inventory manager or crafting system");
        return false;
    }
    
    Logger::Info("[CraftingInventoryBridge] Initialized");
    return true;
}

void CraftingInventoryBridge::shutdown() {
    inventoryManager = nullptr;
    craftingSystem = nullptr;
}

void CraftingInventoryBridge::setCraftingInventory(const std::string& craftingInvID) {
    craftingInventoryID = craftingInvID;
}

void CraftingInventoryBridge::setResultInventory(const std::string& resultInvID) {
    resultInventoryID = resultInvID;
}

CraftingGrid CraftingInventoryBridge::getCraftingGridFromInventory() {
    CraftingGrid grid;
    
    Inventory* inv = inventoryManager->GetInventory(craftingInventoryID);
    if (!inv) {
        Logger::Info("[CraftingInventoryBridge] Crafting inventory not found: " + craftingInventoryID);
        return grid;
    }
    
    // Assuming 3x3 crafting grid mapped to first 9 slots
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            int slotIndex = y * 3 + x;
            if (slotIndex < inv->GetTotalSlots()) {
                const InventorySlot& slot = inv->GetSlot(slotIndex);
                if (!slot.IsEmpty()) {
                    // Convert item ID to block type ID
                    const ItemDef& itemDef = ItemRegistry::GetInstance().GetItem(slot.item.itemID);
                    grid.SetItem(x, y, slot.item.itemID);
                }
            }
        }
    }
    
    return grid;
}

bool CraftingInventoryBridge::checkRecipeMatch(CraftingResult& outResult) {
    if (!craftingSystem) return false;
    
    CraftingGrid grid = getCraftingGridFromInventory();
    CraftingResult2 result = craftingSystem->TryCraft(grid);
    outResult.success = result.success;
    outResult.outputItemID = result.outputItemID;
    outResult.outputCount = result.outputCount;
    outResult.recipeID = result.recipeID;
    outResult.craftTime = result.craftTime;
    
    if (outResult.success) {
        Logger::Info("[CraftingInventoryBridge] Recipe matched: output item " + 
                     outResult.outputItemID);
        if (onRecipeMatched) {
            // Find the matching recipe
            for (const auto& recipe : craftingSystem->GetRecipes()) {
                if (recipe.MatchesGrid(grid)) {
                    onRecipeMatched(recipe);
                    break;
                }
            }
        }
    }
    
    return outResult.success;
}

bool CraftingInventoryBridge::craftItem(CraftingResult& outResult) {
    if (!checkRecipeMatch(outResult)) {
        Logger::Info("[CraftingInventoryBridge] No matching recipe found");
        if (onCraftFail) onCraftFail();
        return false;
    }
    
    // Consume ingredients
    if (!consumeCraftingIngredients()) {
        Logger::Error("[CraftingInventoryBridge] Failed to consume ingredients");
        if (onCraftFail) onCraftFail();
        return false;
    }
    
    // Place result
    if (!placeResult(outResult)) {
        Logger::Error("[CraftingInventoryBridge] Failed to place result");
        if (onCraftFail) onCraftFail();
        return false;
    }
    
    Logger::Info("[CraftingInventoryBridge] Crafting successful");
    if (onCraftSuccess) onCraftSuccess(outResult);
    return true;
}

bool CraftingInventoryBridge::consumeCraftingIngredients() {
    Inventory* inv = inventoryManager->GetInventory(craftingInventoryID);
    if (!inv) return false;
    
    // Clear all items in crafting grid
    for (int i = 0; i < std::min(9, inv->GetTotalSlots()); ++i) {
        inv->ClearSlot(i);
    }
    
    return true;
}

bool CraftingInventoryBridge::placeResult(const CraftingResult& result) {
    Inventory* inv = inventoryManager->GetInventory(resultInventoryID);
    if (!inv) {
        Logger::Info("[CraftingInventoryBridge] Result inventory not found: " + resultInventoryID);
        return false;
    }
    
    // Convert block type to item ID
    const BlockDef& blockDef = BlockRegistry::GetInstance().GetBlock(result.outputType);
    std::string itemID = blockDef.id;
    
    if (itemID.empty()) {
        Logger::Error("[CraftingInventoryBridge] Cannot convert block type to item ID");
        return false;
    }
    
    bool success = inv->AddItem(itemID, result.outputCount);
    if (!success) {
        Logger::Info("[CraftingInventoryBridge] Could not add result to inventory");
        return false;
    }
    
    return true;
}

void CraftingInventoryBridge::clearCraftingGrid() {
    Inventory* inv = inventoryManager->GetInventory(craftingInventoryID);
    if (!inv) return;
    
    for (int i = 0; i < std::min(9, inv->GetTotalSlots()); ++i) {
        inv->ClearSlot(i);
    }
    
    Logger::Info("[CraftingInventoryBridge] Crafting grid cleared");
}

bool CraftingInventoryBridge::autoFillRecipe(const CraftingRecipe& recipe) {
    Inventory* inv = inventoryManager->GetInventory(craftingInventoryID);
    if (!inv) return false;
    
    clearCraftingGrid();
    
    // Fill crafting grid with recipe pattern
    for (int y = 0; y < std::min((int)recipe.pattern.size(), 3); ++y) {
        for (int x = 0; x < std::min((int)recipe.pattern[y].size(), 3); ++x) {
            std::string itemID = recipe.pattern[y][x];
            if (!itemID.empty()) {
                int slotIndex = y * 3 + x;
                inv->AddItemToSlot(slotIndex, itemID, 1);
            }
        }
    }
    
    Logger::Info("[CraftingInventoryBridge] Auto-filled recipe");
    return true;
}

bool CraftingInventoryBridge::hasIngredients(const CraftingRecipe& recipe, const std::string& playerInventoryID) {
    Inventory* inv = inventoryManager->GetInventory(playerInventoryID);
    if (!inv) return false;
    
    // Count required ingredients
    std::unordered_map<std::string, int> required;
    for (const auto& row : recipe.pattern) {
        for (const std::string& itemID : row) {
            if (!itemID.empty()) {
                required[itemID]++;
            }
        }
    }
    
    // Check if inventory has all ingredients
    for (const auto& [itemID, amount] : required) {
        if (!inv->HasItem(itemID, amount)) {
            return false;
        }
    }
    
    return true;
}

std::vector<CraftingRecipe> CraftingInventoryBridge::getAvailableRecipes(const std::string& playerInventoryID) {
    std::vector<CraftingRecipe> available;
    
    if (!craftingSystem) return available;
    
    for (const auto& recipe : craftingSystem->GetRecipes()) {
        if (hasIngredients(recipe, playerInventoryID)) {
            available.push_back(recipe);
        }
    }
    
    return available;
}

// ==================== RecipeBook ====================

RecipeBook::RecipeBook() : categoryFilter(ItemCategory::Misc) {
}

void RecipeBook::addRecipe(const CraftingRecipe& recipe) {
    allRecipes.push_back(recipe);
}

void RecipeBook::removeRecipe(const CraftingRecipe& recipe) {
    // Find and remove matching recipe
    auto it = std::remove_if(allRecipes.begin(), allRecipes.end(),
        [&recipe](const CraftingRecipe& r) {
            return r.outputType == recipe.outputType && r.outputCount == recipe.outputCount;
        });
    allRecipes.erase(it, allRecipes.end());
}

void RecipeBook::clearRecipes() {
    allRecipes.clear();
    filteredRecipes.clear();
}

void RecipeBook::setSearchQuery(const std::string& query) {
    searchQuery = query;
    
    // Update filtered recipes
    filteredRecipes.clear();
    for (const auto& recipe : allRecipes) {
        const BlockDef& blockDef = BlockRegistry::GetInstance().GetBlock(recipe.outputType);
        
        bool matchesSearch = searchQuery.empty() || 
                             blockDef.name.find(searchQuery) != std::string::npos;
        
        if (matchesSearch) {
            filteredRecipes.push_back(recipe);
        }
    }
}

void RecipeBook::setCategoryFilter(ItemCategory category) {
    categoryFilter = category;
    
    // Update filtered recipes
    filteredRecipes.clear();
    for (const auto& recipe : allRecipes) {
        const BlockDef& blockDef = BlockRegistry::GetInstance().GetBlock(recipe.outputType);
        
        // Map block types to item categories based on block ID patterns
        ItemCategory blockCategory = ItemCategory::Misc;
        
        // Check block ID patterns to determine category
        if (blockDef.id.find("ore") != std::string::npos || 
            blockDef.id.find("ingot") != std::string::npos ||
            blockDef.id.find("gem") != std::string::npos ||
            blockDef.id.find("coal") != std::string::npos ||
            blockDef.id.find("iron") != std::string::npos ||
            blockDef.id.find("gold") != std::string::npos ||
            blockDef.id.find("diamond") != std::string::npos) {
            blockCategory = ItemCategory::Material; // Ores and materials
        }
        else if (blockDef.id.find("food") != std::string::npos ||
            blockDef.id.find("meat") != std::string::npos ||
            blockDef.id.find("fruit") != std::string::npos ||
            blockDef.id.find("vegetable") != std::string::npos ||
            blockDef.id.find("bread") != std::string::npos ||
            blockDef.id.find("apple") != std::string::npos) {
            blockCategory = ItemCategory::Consumable; // Food items
        }
        else if (blockDef.id.find("sword") != std::string::npos ||
            blockDef.id.find("axe") != std::string::npos ||
            blockDef.id.find("pickaxe") != std::string::npos ||
            blockDef.id.find("shovel") != std::string::npos ||
            blockDef.id.find("hoe") != std::string::npos ||
            blockDef.id.find("bow") != std::string::npos) {
            blockCategory = ItemCategory::Weapon; // Weapons
        }
        else if (blockDef.id.find("armor") != std::string::npos ||
            blockDef.id.find("helmet") != std::string::npos ||
            blockDef.id.find("chestplate") != std::string::npos ||
            blockDef.id.find("leggings") != std::string::npos ||
            blockDef.id.find("boots") != std::string::npos ||
            blockDef.id.find("shield") != std::string::npos) {
            blockCategory = ItemCategory::Armor; // Armor
        }
        else if (blockDef.id.find("potion") != std::string::npos ||
            blockDef.id.find("herb") != std::string::npos ||
            blockDef.id.find("medicine") != std::string::npos) {
            blockCategory = ItemCategory::Consumable; // Potions and medicine
        }
        else if (blockDef.id.find("plank") != std::string::npos ||
            blockDef.id.find("stone") != std::string::npos ||
            blockDef.id.find("brick") != std::string::npos ||
            blockDef.id.find("glass") != std::string::npos ||
            blockDef.id.find("wood") != std::string::npos ||
            blockDef.id.find("dirt") != std::string::npos ||
            blockDef.id.find("sand") != std::string::npos ||
            blockDef.id.find("gravel") != std::string::npos ||
            blockDef.id.find("concrete") != std::string::npos) {
            blockCategory = ItemCategory::Block; // Building blocks
        }
        else if (blockDef.id.find("stick") != std::string::npos ||
            blockDef.id.find("string") != std::string::npos ||
            blockDef.id.find("feather") != std::string::npos ||
            blockDef.id.find("leather") != std::string::npos ||
            blockDef.id.find("wool") != std::string::npos) {
            blockCategory = ItemCategory::Material; // Crafting materials
        }
        
        bool matchesCategory = category == ItemCategory::Misc || 
                               blockCategory == category;
        
        if (matchesCategory) {
            filteredRecipes.push_back(recipe);
        }
    }
}

void RecipeBook::clearFilters() {
    searchQuery.clear();
    categoryFilter = ItemCategory::Misc;
    filteredRecipes = allRecipes;
}

std::vector<CraftingRecipe> RecipeBook::getFilteredRecipes() const {
    if (searchQuery.empty() && categoryFilter == ItemCategory::Misc) {
        return allRecipes;
    }
    return filteredRecipes;
}

bool RecipeBook::canCraft(const CraftingRecipe& recipe, const Inventory& inventory) const {
    // Count required ingredients
    std::unordered_map<std::string, int> required;
    for (const auto& row : recipe.pattern) {
        for (const std::string& itemID : row) {
            if (!itemID.empty()) {
                required[itemID]++;
            }
        }
    }
    
    // Check if inventory has all ingredients
    for (const auto& [itemID, amount] : required) {
        if (!inventory.HasItem(itemID, amount)) {
            return false;
        }
    }
    
    return true;
}

std::vector<std::pair<std::string, int>> RecipeBook::getMissingIngredients(
    const CraftingRecipe& recipe, const Inventory& inventory) const {
    
    std::vector<std::pair<std::string, int>> missing;
    
    // Count required ingredients
    std::unordered_map<std::string, int> required;
    for (const auto& row : recipe.pattern) {
        for (const std::string& itemID : row) {
            if (!itemID.empty()) {
                required[itemID]++;
            }
        }
    }
    
    // Check what's missing
    for (const auto& [itemID, amount] : required) {
        int available = inventory.GetItemCount(itemID);
        if (available < amount) {
            missing.push_back({itemID, amount - available});
        }
    }
    
    return missing;
}

} // namespace vge
