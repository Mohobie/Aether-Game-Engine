#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace game {

struct CraftingIngredient {
    std::string itemID;
    uint32_t count;
};

struct CraftingRecipe {
    std::string resultID;
    uint32_t resultCount;
    std::vector<CraftingIngredient> ingredients;
};

class CraftingRegistry {
public:
    void addRecipe(CraftingRecipe recipe);
    std::vector<CraftingRecipe> findRecipes(const std::string& itemID) const;
    const CraftingRecipe* match(const std::unordered_map<std::string, uint32_t>& available) const;
    const std::vector<CraftingRecipe>& all() const { return recipes; }
private:
    std::vector<CraftingRecipe> recipes;
};

} // namespace game
