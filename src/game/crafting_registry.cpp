#include "game/crafting_registry.hpp"

namespace game {

void CraftingRegistry::addRecipe(CraftingRecipe recipe) {
    recipes.push_back(std::move(recipe));
}

std::vector<CraftingRecipe> CraftingRegistry::findRecipes(const std::string& itemID) const {
    std::vector<CraftingRecipe> out;
    for (const auto& r : recipes) if (r.resultID == itemID) out.push_back(r);
    return out;
}

const CraftingRecipe* CraftingRegistry::match(const std::unordered_map<std::string, uint32_t>& available) const {
    for (const auto& r : recipes) {
        bool ok = true;
        for (const auto& ing : r.ingredients) {
            auto it = available.find(ing.itemID);
            if (it == available.end() || it->second < ing.count) { ok = false; break; }
        }
        if (ok) return &r;
    }
    return nullptr;
}

} // namespace game
