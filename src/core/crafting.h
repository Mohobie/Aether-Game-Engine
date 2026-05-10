#pragma once
#include <vector>
#include "voxel/block_types.h"

namespace vge {

class CraftingGrid {
private:
    std::vector<std::vector<BlockTypeID>> items;
    
public:
    CraftingGrid();
    
    void SetItem(int x, int y, BlockTypeID type);
    BlockTypeID GetItem(int x, int y) const;
    void Clear();
    bool IsEmpty() const;
};

struct CraftingRecipe {
    std::vector<std::vector<BlockTypeID>> pattern;
    BlockTypeID outputType;
    int outputCount;
    
    CraftingRecipe();
    CraftingRecipe(const std::vector<std::vector<BlockTypeID>>& pattern, 
                   BlockTypeID output, int count);
    
    bool Matches(const CraftingGrid& grid) const;
    bool MatchesAt(const CraftingGrid& grid, int startX, int startY) const;
};

struct CraftingResult {
    bool success;
    BlockTypeID outputType;
    int outputCount;
    
    CraftingResult() : success(false), outputType(BLOCK_AIR), outputCount(0) {}
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