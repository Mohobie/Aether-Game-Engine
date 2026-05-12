#include "game/crafting_system.h"
#include "core/logger.h"
#include <algorithm>

namespace vge {

// ============================================
// CraftingSystem
// ============================================
CraftingSystem::CraftingSystem() {
    InitializeDefaultRecipes();
}

void CraftingSystem::InitializeDefaultRecipes() {
    // Wood -> Planks (shapeless)
    CraftingRecipe planks;
    planks.recipeID = "planks_from_wood";
    planks.displayName = "Wooden Planks";
    planks.outputItemID = "planks";
    planks.outputCount = 4;
    planks.shapeless = true;
    planks.ingredients.push_back(RecipeIngredient("wood", 1));
    AddRecipe(planks);

    // Planks -> Sticks
    CraftingRecipe sticks;
    sticks.recipeID = "sticks_from_planks";
    sticks.displayName = "Sticks";
    sticks.outputItemID = "stick";
    sticks.outputCount = 4;
    sticks.pattern = {{"planks", "", ""}, {"planks", "", ""}, {"", "", ""}};
    AddRecipe(sticks);

    // Sticks + Stone -> Pickaxe
    CraftingRecipe pickaxe;
    pickaxe.recipeID = "stone_pickaxe";
    pickaxe.displayName = "Stone Pickaxe";
    pickaxe.outputItemID = "stone_pickaxe";
    pickaxe.outputCount = 1;
    pickaxe.pattern = {{"cobblestone", "cobblestone", "cobblestone"}, {"", "stick", ""}, {"", "stick", ""}};
    AddRecipe(pickaxe);

    // Wooden Pickaxe
    CraftingRecipe woodPickaxe;
    woodPickaxe.recipeID = "wooden_pickaxe";
    woodPickaxe.displayName = "Wooden Pickaxe";
    woodPickaxe.outputItemID = "wooden_pickaxe";
    woodPickaxe.outputCount = 1;
    woodPickaxe.pattern = {{"planks", "planks", "planks"}, {"", "stick", ""}, {"", "stick", ""}};
    AddRecipe(woodPickaxe);

    // Crafting Table
    CraftingRecipe table;
    table.recipeID = "crafting_table";
    table.displayName = "Crafting Table";
    table.outputItemID = "crafting_table";
    table.outputCount = 1;
    table.pattern = {{"planks", "planks", ""}, {"planks", "planks", ""}, {"", "", ""}};
    AddRecipe(table);

    // Torch
    CraftingRecipe torch;
    torch.recipeID = "torch";
    torch.displayName = "Torch";
    torch.outputItemID = "torch";
    torch.outputCount = 4;
    torch.pattern = {{"coal", "", ""}, {"stick", "", ""}, {"", "", ""}};
    AddRecipe(torch);
}

void CraftingSystem::AddRecipe(const CraftingRecipe& recipe) {
    size_t index = recipes.size();
    recipes.push_back(recipe);
    recipeIndexByID[recipe.recipeID] = index;
    recipesByOutput[recipe.outputItemID].push_back(index);
    auto required = recipe.GetRequiredItems();
    for (const auto& [itemID, count] : required) {
        recipesByInput[itemID].push_back(index);
    }
}

void CraftingSystem::RemoveRecipe(const std::string& recipeID) {
    auto it = recipeIndexByID.find(recipeID);
    if (it == recipeIndexByID.end()) return;
    size_t index = it->second;
    recipes.erase(recipes.begin() + index);
    recipeIndexByID.clear();
    recipesByOutput.clear();
    recipesByInput.clear();
    for (size_t i = 0; i < recipes.size(); ++i) {
        recipeIndexByID[recipes[i].recipeID] = i;
        recipesByOutput[recipes[i].outputItemID].push_back(i);
        auto required = recipes[i].GetRequiredItems();
        for (const auto& [itemID, count] : required) {
            recipesByInput[itemID].push_back(i);
        }
    }
}

const CraftingRecipe* CraftingSystem::GetRecipe(const std::string& recipeID) const {
    auto it = recipeIndexByID.find(recipeID);
    if (it != recipeIndexByID.end()) return &recipes[it->second];
    return nullptr;
}

const CraftingRecipe* CraftingSystem::FindRecipe(const CraftingGrid& grid, const std::string& station) const {
    for (const auto& recipe : recipes) {
        if (!station.empty() && recipe.requiredStation != station) continue;
        if (recipe.MatchesGrid(grid)) return &recipe;
    }
    return nullptr;
}

const CraftingRecipe* CraftingSystem::FindShapelessRecipe(const Inventory& inventory, const std::string& station) const {
    for (const auto& recipe : recipes) {
        if (!recipe.shapeless) continue;
        if (!station.empty() && recipe.requiredStation != station) continue;
        if (recipe.MatchesInventory(inventory)) return &recipe;
    }
    return nullptr;
}

std::vector<const CraftingRecipe*> CraftingSystem::GetRecipesForOutput(const std::string& itemID) const {
    std::vector<const CraftingRecipe*> result;
    auto it = recipesByOutput.find(itemID);
    if (it != recipesByOutput.end()) {
        for (size_t index : it->second) result.push_back(&recipes[index]);
    }
    return result;
}

std::vector<const CraftingRecipe*> CraftingSystem::GetRecipesUsingInput(const std::string& itemID) const {
    std::vector<const CraftingRecipe*> result;
    auto it = recipesByInput.find(itemID);
    if (it != recipesByInput.end()) {
        for (size_t index : it->second) result.push_back(&recipes[index]);
    }
    return result;
}

std::vector<const CraftingRecipe*> CraftingSystem::GetAllRecipes() const {
    std::vector<const CraftingRecipe*> result;
    for (const auto& recipe : recipes) result.push_back(&recipe);
    return result;
}

CraftingResult2 CraftingSystem::TryCraft(const CraftingGrid& grid, const Inventory* playerInventory,
                                        const std::string& station) const {
    CraftingResult2 result;
    result.success = false;
    const CraftingRecipe* recipe = FindRecipe(grid, station);
    if (!recipe && playerInventory) {
        recipe = FindShapelessRecipe(*playerInventory, station);
    }
    if (recipe) {
        result.success = true;
        result.outputItemID = recipe->outputItemID;
        result.outputCount = recipe->outputCount;
        result.recipeID = recipe->recipeID;
        result.craftTime = recipe->craftTime;
    }
    return result;
}

bool CraftingSystem::CanCraft(const CraftingRecipe& recipe, const Inventory& inventory) const {
    return recipe.MatchesInventory(inventory);
}

std::vector<std::pair<std::string, int>> CraftingSystem::GetMissingIngredients(
    const CraftingRecipe& recipe, const Inventory& inventory) const {
    std::vector<std::pair<std::string, int>> missing;
    auto required = recipe.GetRequiredItems();
    for (const auto& [itemID, count] : required) {
        int available = inventory.GetItemCount(itemID);
        if (available < count) missing.push_back({itemID, count - available});
    }
    return missing;
}

void CraftingSystem::PrintRecipes() const {
    Logger::Info("=== Crafting Recipes (" + std::to_string(recipes.size()) + ") ===");
    for (const auto& recipe : recipes) {
        Logger::Info("  " + recipe.recipeID + " -> " + recipe.outputItemID + " x" + std::to_string(recipe.outputCount));
    }
}

// ============================================
// RecipeProgression
// ============================================
RecipeProgression::RecipeProgression() : playerLevel(1) {
}

bool RecipeProgression::IsDiscovered(const std::string& recipeID) const {
    auto it = discoveredRecipes.find(recipeID);
    return it != discoveredRecipes.end() && it->second;
}

void RecipeProgression::DiscoverRecipe(const std::string& recipeID) {
    discoveredRecipes[recipeID] = true;
    Logger::Info("[RecipeProgression] Discovered recipe: " + recipeID);
}

void RecipeProgression::DiscoverAll() {
    for (auto& [id, discovered] : discoveredRecipes) discovered = true;
}

void RecipeProgression::OnItemObtained(const std::string& itemID) {
    auto it = itemDiscoveryTriggers.find(itemID);
    if (it != itemDiscoveryTriggers.end()) {
        for (const auto& recipeID : it->second) {
            if (!IsDiscovered(recipeID)) DiscoverRecipe(recipeID);
        }
    }
}

void RecipeProgression::OnRecipeCrafted(const std::string& recipeID) {
    // Could track craft counts here
    (void)recipeID;
}

void RecipeProgression::RegisterItemTrigger(const std::string& itemID, const std::string& recipeID) {
    itemDiscoveryTriggers[itemID].push_back(recipeID);
    if (discoveredRecipes.find(recipeID) == discoveredRecipes.end()) {
        discoveredRecipes[recipeID] = false;
    }
}

std::vector<std::string> RecipeProgression::GetDiscoveredRecipes() const {
    std::vector<std::string> result;
    for (const auto& [id, discovered] : discoveredRecipes) {
        if (discovered) result.push_back(id);
    }
    return result;
}

} // namespace vge
