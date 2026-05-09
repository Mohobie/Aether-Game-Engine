#pragma once
#include <vector>
#include "voxel/block.h"

namespace vge {

class CraftingGrid {
private:
    std::vector<std::vector<BlockType>> items;
    
public:
    CraftingGrid();
    
    void SetItem(int x, int y, BlockType type);
    BlockType GetItem(int x, int y) const;
    void Clear();
    bool IsEmpty() const;
};

struct CraftingRecipe {
    std::vector<std::vector<BlockType>> pattern;
    BlockType outputType;
    int outputCount;
    
    CraftingRecipe();
    CraftingRecipe(const std::vector<std::vector<BlockType>>& pattern, 
                   BlockType output, int count);
    
    bool Matches(const CraftingGrid& grid) const;
    bool MatchesAt(const CraftingGrid& grid, int startX, int startY) const;
};

struct CraftingResult {
    bool success;
    BlockType outputType;
    int outputCount;
    std::vector<std::tuple<int, int, BlockType>> consumed; // x, y, type
    
    CraftingResult() : success(false), outputType(BlockType::Air), outputCount(0) {}
};

class CraftingSystem {
private:
    std::vector<CraftingRecipe> recipes;
    
    void InitializeRecipes();
    
public:
    CraftingSystem();
    
    CraftingResult TryCraft(const CraftingGrid& grid);
    void PrintRecipes() const;
    
    const std::vector<CraftingRecipe>& GetRecipes() const { return recipes; }
};

} // namespace vge