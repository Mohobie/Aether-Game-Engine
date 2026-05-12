#pragma once
#include "core/crafting.h"
#include "core/item_system.h"
#include <memory>
#include <functional>

namespace vge {

// ============================================
// Simplified Crafting System for Game Layer
// ============================================
class CraftingSystem {
private:
    std::vector<CraftingRecipe> recipes;
    std::unordered_map<std::string, size_t> recipeIndexByID;
    std::unordered_map<std::string, std::vector<size_t>> recipesByOutput;
    std::unordered_map<std::string, std::vector<size_t>> recipesByInput;
    
    void InitializeDefaultRecipes();
    
public:
    CraftingSystem();
    
    // Recipe management
    void AddRecipe(const CraftingRecipe& recipe);
    void RemoveRecipe(const std::string& recipeID);
    const CraftingRecipe* GetRecipe(const std::string& recipeID) const;
    const CraftingRecipe* FindRecipe(const CraftingGrid& grid, const std::string& station = "") const;
    const CraftingRecipe* FindShapelessRecipe(const Inventory& inventory, const std::string& station = "") const;
    
    // Queries
    std::vector<const CraftingRecipe*> GetRecipesForOutput(const std::string& itemID) const;
    std::vector<const CraftingRecipe*> GetRecipesUsingInput(const std::string& itemID) const;
    std::vector<const CraftingRecipe*> GetAllRecipes() const;
    
    // Crafting
    CraftingResult2 TryCraft(const CraftingGrid& grid, const Inventory* playerInventory = nullptr,
                            const std::string& station = "") const;
    
    // Check if player can craft
    bool CanCraft(const CraftingRecipe& recipe, const Inventory& inventory) const;
    
    // Get missing ingredients
    std::vector<std::pair<std::string, int>> GetMissingIngredients(
        const CraftingRecipe& recipe, const Inventory& inventory) const;
    
    // Debug
    void PrintRecipes() const;
    size_t GetRecipeCount() const { return recipes.size(); }
    
    // Legacy compatibility - returns internal vector
    const std::vector<CraftingRecipe>& GetRecipes() const { return recipes; }
};

// ============================================
// Recipe Progression / Discovery
// ============================================
class RecipeProgression {
private:
    std::unordered_map<std::string, bool> discoveredRecipes;
    std::unordered_map<std::string, std::vector<std::string>> itemDiscoveryTriggers;
    int playerLevel;
    
public:
    RecipeProgression();
    
    bool IsDiscovered(const std::string& recipeID) const;
    void DiscoverRecipe(const std::string& recipeID);
    void DiscoverAll();
    
    void OnItemObtained(const std::string& itemID);
    void OnRecipeCrafted(const std::string& recipeID);
    
    void RegisterItemTrigger(const std::string& itemID, const std::string& recipeID);
    
    std::vector<std::string> GetDiscoveredRecipes() const;
    
    int GetPlayerLevel() const { return playerLevel; }
    void SetPlayerLevel(int level) { playerLevel = level; }
};

} // namespace vge
