#pragma once
#include "core/item_system.h"
#include "core/crafting.h"
#include <vector>
#include <memory>

namespace vge {

// Forward declarations
class Inventory;
class InventoryManager;
class CraftingSystem;

// Crafting integration with inventory system
class CraftingInventoryBridge {
private:
    InventoryManager* inventoryManager;
    CraftingSystem* craftingSystem;
    
    // Track crafting grid state
    std::string craftingInventoryID;
    std::string resultInventoryID;
    
public:
    CraftingInventoryBridge(InventoryManager* invManager, CraftingSystem* craftSystem);
    ~CraftingInventoryBridge() = default;
    
    bool initialize();
    void shutdown();
    
    // Set up crafting inventories
    void setCraftingInventory(const std::string& craftingInvID);
    void setResultInventory(const std::string& resultInvID);
    
    // Convert inventory grid to crafting grid
    CraftingGrid getCraftingGridFromInventory();
    
    // Check if current grid matches a recipe
    bool checkRecipeMatch(CraftingResult& outResult);
    
    // Perform crafting operation
    bool craftItem(CraftingResult& outResult);
    
    // Consume ingredients from crafting grid
    bool consumeCraftingIngredients();
    
    // Place result in result slot
    bool placeResult(const CraftingResult& result);
    
    // Clear crafting grid
    void clearCraftingGrid();
    
    // Auto-fill recipe into crafting grid (if player has ingredients)
    bool autoFillRecipe(const CraftingRecipe& recipe);
    
    // Check if player has ingredients for a recipe
    bool hasIngredients(const CraftingRecipe& recipe, const std::string& playerInventoryID);
    
    // Get available recipes (that player can craft)
    std::vector<CraftingRecipe> getAvailableRecipes(const std::string& playerInventoryID);
    
    // Event callbacks
    std::function<void(const CraftingResult&)> onCraftSuccess;
    std::function<void()> onCraftFail;
    std::function<void(const CraftingRecipe&)> onRecipeMatched;
};

// Recipe book / crafting menu
class RecipeBook {
private:
    std::vector<CraftingRecipe> allRecipes;
    std::vector<CraftingRecipe> filteredRecipes;
    std::string searchQuery;
    ItemCategory categoryFilter;
    
public:
    RecipeBook();
    ~RecipeBook() = default;
    
    void addRecipe(const CraftingRecipe& recipe);
    void removeRecipe(const CraftingRecipe& recipe);
    void clearRecipes();
    
    // Filtering
    void setSearchQuery(const std::string& query);
    void setCategoryFilter(ItemCategory category);
    void clearFilters();
    
    std::vector<CraftingRecipe> getFilteredRecipes() const;
    std::vector<CraftingRecipe> getAllRecipes() const { return allRecipes; }
    
    // Check if recipe can be crafted with given inventory
    bool canCraft(const CraftingRecipe& recipe, const Inventory& inventory) const;
    
    // Get missing ingredients for a recipe
    std::vector<std::pair<std::string, int>> getMissingIngredients(
        const CraftingRecipe& recipe, const Inventory& inventory) const;
};

} // namespace vge
