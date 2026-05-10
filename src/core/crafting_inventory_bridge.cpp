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
        Logger::Warning("[CraftingInventoryBridge] Crafting inventory not found: " + craftingInventoryID);
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
                    grid.SetItem(x, y, itemDef.blockType);
                }
            }
        }
    }
    
    return grid;
}

bool CraftingInventoryBridge::checkRecipeMatch(CraftingResult& outResult) {
    if (!craftingSystem) return false;
    
    CraftingGrid grid = getCraftingGridFromInventory();
    outResult = craftingSystem->TryCraft(grid);
    
    if (outResult.success) {
        Logger::Info("[CraftingInventoryBridge] Recipe matched: output type " + 
                     std::to_string(outResult.outputType));
        if (onRecipeMatched) {
            // Find the matching recipe
            for (const auto& recipe : craftingSystem->GetRecipes()) {
                if (recipe.Matches(grid)) {
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
        Logger::Warning("[CraftingInventoryBridge] Result inventory not found: " + resultInventoryID);
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
        Logger::Warning("[CraftingInventoryBridge] Could not add result to inventory");
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
            BlockTypeID blockType = recipe.pattern[y][x];
            if (blockType != BLOCK_AIR) {
                const BlockDef& blockDef = BlockRegistry::GetInstance().GetBlock(blockType);
                int slotIndex = y * 3 + x;
                inv->AddItemToSlot(slotIndex, blockDef.id, 1);
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
        for (BlockTypeID blockType : row) {
            if (blockType != BLOCK_AIR) {
                const BlockDef& blockDef = BlockRegistry::GetInstance().GetBlock(blockType);
                required[blockDef.id]++;
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
        
        // TODO: Map block types to item categories
        bool matchesCategory = category == ItemCategory::Misc || 
                               categoryFilter == ItemCategory::Misc;
        
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
        for (BlockTypeID blockType : row) {
            if (blockType != BLOCK_AIR) {
                const BlockDef& blockDef = BlockRegistry::GetInstance().GetBlock(blockType);
                required[blockDef.id]++;
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
        for (BlockTypeID blockType : row) {
            if (blockType != BLOCK_AIR) {
                const BlockDef& blockDef = BlockRegistry::GetInstance().GetBlock(blockType);
                required[blockDef.id]++;
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
