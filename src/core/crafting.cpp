#include "crafting.h"
#include "voxel/block_types.h"
#include "voxel/block_registry.h"
#include <iostream>

namespace vge {

CraftingRecipe::CraftingRecipe() : outputType(BLOCK_AIR), outputCount(1) {}

CraftingRecipe::CraftingRecipe(const std::vector<std::vector<BlockTypeID>>& pattern, 
                               BlockTypeID output, int count)
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
            BlockTypeID expected = pattern[y][x];
            BlockTypeID actual = grid.GetItem(startX + x, startY + y);
            
            if (expected != actual) {
                return false;
            }
        }
    }
    return true;
}

CraftingGrid::CraftingGrid() {
    items.resize(3, std::vector<BlockTypeID>(3, BLOCK_AIR));
}

void CraftingGrid::SetItem(int x, int y, BlockTypeID type) {
    if (x >= 0 && x < 3 && y >= 0 && y < 3) {
        items[y][x] = type;
    }
}

BlockTypeID CraftingGrid::GetItem(int x, int y) const {
    if (x >= 0 && x < 3 && y >= 0 && y < 3) {
        return items[y][x];
    }
    return BLOCK_AIR;
}

void CraftingGrid::Clear() {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            items[y][x] = BLOCK_AIR;
        }
    }
}

bool CraftingGrid::IsEmpty() const {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (items[y][x] != BLOCK_AIR) return false;
        }
    }
    return true;
}

CraftingSystem::CraftingSystem() {
    InitializeRecipes();
}

void CraftingSystem::InitializeRecipes() {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    
    // Get block IDs from registry (data-driven)
    BlockTypeID woodId = registry.GetBlockId("wood");
    BlockTypeID planksId = registry.GetBlockId("planks");
    BlockTypeID stickId = registry.GetBlockId("stick");
    BlockTypeID craftingTableId = registry.GetBlockId("crafting_table");
    
    // Planks from wood
    if (woodId != BLOCK_AIR && planksId != BLOCK_AIR) {
        recipes.push_back(CraftingRecipe(
            {{woodId}},
            planksId, 4
        ));
    }
    
    // Sticks from planks
    if (planksId != BLOCK_AIR && stickId != BLOCK_AIR) {
        recipes.push_back(CraftingRecipe(
            {{planksId},
             {planksId}},
            stickId, 4
        ));
    }
    
    // Crafting table from planks
    if (planksId != BLOCK_AIR && craftingTableId != BLOCK_AIR) {
        recipes.push_back(CraftingRecipe(
            {{planksId, planksId},
             {planksId, planksId}},
            craftingTableId, 1
        ));
    }
}

CraftingResult2 CraftingSystem::TryCraft(const CraftingGrid& grid, const Inventory* playerInventory, 
                           const std::string& station) const {
    CraftingResult2 result;
    result.success = false;
    
    // TODO: Implement modern crafting logic
    (void)grid;
    (void)playerInventory;
    (void)station;
    
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
