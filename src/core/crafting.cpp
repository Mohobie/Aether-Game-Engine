#include "crafting.h"
#include "voxel/block_registry.h"
#include <iostream>

namespace vge {

CraftingRecipe::CraftingRecipe() : outputType(BlockType::Air), outputCount(1) {}

CraftingRecipe::CraftingRecipe(const std::vector<std::vector<BlockType>>& pattern, 
                               BlockType output, int count)
    : pattern(pattern), outputType(output), outputCount(count) {}

bool CraftingRecipe::Matches(const CraftingGrid& grid) const {
    if (pattern.empty()) return false;
    
    int patternHeight = (int)pattern.size();
    int patternWidth = (int)pattern[0].size();
    
    // Try all positions in the 3x3 grid
    for (int startY = 0; startY <= 3 - patternHeight; ++startY) {
        for (int startX = 0; startX <= 3 - patternWidth; ++startX) {
            if (MatchesAt(grid, startX, startY)) {
                return true;
            }
        }
    }
    
    return false;
}

bool CraftingRecipe::MatchesAt(const CraftingGrid& grid, int startX, int startY) const {
    for (int y = 0; y < (int)pattern.size(); ++y) {
        for (int x = 0; x < (int)pattern[y].size(); ++x) {
            BlockType expected = pattern[y][x];
            BlockType actual = grid.GetItem(startX + x, startY + y);
            
            if (expected != actual) {
                return false;
            }
        }
    }
    return true;
}

CraftingGrid::CraftingGrid() {
    items.resize(3, std::vector<BlockType>(3, BlockType::Air));
}

void CraftingGrid::SetItem(int x, int y, BlockType type) {
    if (x >= 0 && x < 3 && y >= 0 && y < 3) {
        items[y][x] = type;
    }
}

BlockType CraftingGrid::GetItem(int x, int y) const {
    if (x >= 0 && x < 3 && y >= 0 && y < 3) {
        return items[y][x];
    }
    return BlockType::Air;
}

void CraftingGrid::Clear() {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            items[y][x] = BlockType::Air;
        }
    }
}

bool CraftingGrid::IsEmpty() const {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (items[y][x] != BlockType::Air) return false;
        }
    }
    return true;
}

CraftingSystem::CraftingSystem() {
    InitializeRecipes();
}

void CraftingSystem::InitializeRecipes() {
    // Planks from wood
    recipes.push_back(CraftingRecipe(
        {{BlockType::Wood}},
        BlockType::Planks, 4
    ));
    
    // Sticks from planks
    recipes.push_back(CraftingRecipe(
        {{BlockType::Planks},
         {BlockType::Planks}},
        BlockType::Stick, 4
    ));
    
    // Crafting table from planks
    recipes.push_back(CraftingRecipe(
        {{BlockType::Planks, BlockType::Planks},
         {BlockType::Planks, BlockType::Planks}},
        BlockType::CraftingTable, 1
    ));
    
    // Wooden pickaxe
    recipes.push_back(CraftingRecipe(
        {{BlockType::Planks, BlockType::Planks, BlockType::Planks},
         {BlockType::Air, BlockType::Stick, BlockType::Air},
         {BlockType::Air, BlockType::Stick, BlockType::Air}},
        BlockType::Wood, 1  // Using Wood as placeholder for pickaxe
    ));
}

CraftingResult CraftingSystem::TryCraft(const CraftingGrid& grid) {
    CraftingResult result;
    result.success = false;
    
    for (const auto& recipe : recipes) {
        if (recipe.Matches(grid)) {
            result.success = true;
            result.outputType = recipe.outputType;
            result.outputCount = recipe.outputCount;
            return result;
        }
    }
    
    return result;
}

void CraftingSystem::PrintRecipes() const {
    std::cout << "=== Crafting Recipes ===" << std::endl;
    for (int i = 0; i < (int)recipes.size(); ++i) {
        const auto& recipe = recipes[i];
        std::cout << "Recipe " << i << ": " 
                  << BlockRegistry::GetInstance().GetBlock(recipe.outputType).GetName()
                  << " x" << recipe.outputCount << std::endl;
    }
}

} // namespace vge