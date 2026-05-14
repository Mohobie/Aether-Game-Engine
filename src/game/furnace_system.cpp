#include "game/furnace_system.h"
#include "core/logger.h"
#include "voxel/block_registry.h"
#include <sstream>

namespace vge {

std::string FurnaceManager::GetKey(int x, int y, int z) const {
    return std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
}

FurnaceManager::FurnaceManager(CraftingSystem* craftSystem)
    : craftingSystem(craftSystem) {
    // Register default fuel values (burn time in seconds)
    RegisterFuel("coal", 80.0f);
    RegisterFuel("charcoal", 80.0f);
    RegisterFuel("wood", 15.0f);
    RegisterFuel("planks", 15.0f);
    RegisterFuel("stick", 5.0f);
    RegisterFuel("sapling", 5.0f);
    RegisterFuel("lava_bucket", 1000.0f);
    RegisterFuel("coal_block", 800.0f);
}

void FurnaceManager::RegisterFuel(const std::string& itemID, float burnTime) {
    fuelItems.push_back(itemID);
    Logger::Info("[FurnaceManager] Registered fuel: " + itemID + " (" + std::to_string((int)burnTime) + "s)");
}

bool FurnaceManager::IsFuel(const std::string& itemID) const {
    for (const auto& fuel : fuelItems) {
        if (fuel == itemID) return true;
    }
    return false;
}

float FurnaceManager::GetFuelValue(const std::string& itemID) const {
    if (itemID == "coal" || itemID == "charcoal") return 80.0f;
    if (itemID == "wood" || itemID == "planks") return 15.0f;
    if (itemID == "stick" || itemID == "sapling") return 5.0f;
    if (itemID == "lava_bucket") return 1000.0f;
    if (itemID == "coal_block") return 800.0f;
    return 0.0f;
}

void FurnaceManager::PlaceFurnace(int x, int y, int z) {
    std::string key = GetKey(x, y, z);
    if (furnaces.find(key) == furnaces.end()) {
        FurnaceBlockState state;
        state.position = Vec3((float)x, (float)y, (float)z);
        furnaces[key] = state;
        Logger::Info("[FurnaceManager] Placed furnace at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
    }
}

void FurnaceManager::RemoveFurnace(int x, int y, int z) {
    std::string key = GetKey(x, y, z);
    auto it = furnaces.find(key);
    if (it != furnaces.end()) {
        furnaces.erase(it);
        Logger::Info("[FurnaceManager] Removed furnace at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
    }
}

bool FurnaceManager::HasFurnace(int x, int y, int z) const {
    return furnaces.find(GetKey(x, y, z)) != furnaces.end();
}

FurnaceBlockState* FurnaceManager::GetFurnace(int x, int y, int z) {
    std::string key = GetKey(x, y, z);
    auto it = furnaces.find(key);
    if (it != furnaces.end()) {
        return &it->second;
    }
    return nullptr;
}

bool FurnaceManager::AddInput(int x, int y, int z, const std::string& itemID, int count) {
    FurnaceBlockState* furnace = GetFurnace(x, y, z);
    if (!furnace) return false;

    if (furnace->inputItem.empty() || furnace->inputItem == itemID) {
        furnace->inputItem = itemID;
        furnace->inputCount += count;
        return true;
    }
    return false;
}

bool FurnaceManager::AddFuel(int x, int y, int z, const std::string& itemID, int count) {
    if (!IsFuel(itemID)) return false;

    FurnaceBlockState* furnace = GetFurnace(x, y, z);
    if (!furnace) return false;

    if (furnace->fuelItem.empty() || furnace->fuelItem == itemID) {
        furnace->fuelItem = itemID;
        furnace->fuelCount += count;
        return true;
    }
    return false;
}

bool FurnaceManager::RemoveOutput(int x, int y, int z, int count) {
    FurnaceBlockState* furnace = GetFurnace(x, y, z);
    if (!furnace || furnace->outputCount < count) return false;

    furnace->outputCount -= count;
    if (furnace->outputCount <= 0) {
        furnace->outputItem.clear();
        furnace->outputCount = 0;
    }
    return true;
}

bool FurnaceManager::TakeInput(int x, int y, int z, int count) {
    FurnaceBlockState* furnace = GetFurnace(x, y, z);
    if (!furnace || furnace->inputCount < count) return false;

    furnace->inputCount -= count;
    if (furnace->inputCount <= 0) {
        furnace->inputItem.clear();
        furnace->inputCount = 0;
    }
    return true;
}

bool FurnaceManager::TakeFuel(int x, int y, int z, int count) {
    FurnaceBlockState* furnace = GetFurnace(x, y, z);
    if (!furnace || furnace->fuelCount < count) return false;

    furnace->fuelCount -= count;
    if (furnace->fuelCount <= 0) {
        furnace->fuelItem.clear();
        furnace->fuelCount = 0;
    }
    return true;
}

float FurnaceManager::CollectExperience(int x, int y, int z) {
    FurnaceBlockState* furnace = GetFurnace(x, y, z);
    if (!furnace) return 0.0f;

    float xp = furnace->storedExperience;
    furnace->storedExperience = 0.0f;
    return xp;
}

bool FurnaceManager::CanSmelt(const FurnaceBlockState& furnace) const {
    if (furnace.inputItem.empty() || furnace.inputCount <= 0) return false;
    if (!craftingSystem) return false;

    const SmeltingRecipe* recipe = craftingSystem->FindSmeltingRecipe(furnace.inputItem);
    if (!recipe) return false;

    // Check if output slot can accept the result
    if (!furnace.outputItem.empty() && furnace.outputItem != recipe->outputItemID) {
        return false;
    }
    if (!furnace.outputItem.empty() && furnace.outputCount >= 64) {
        return false;
    }

    return true;
}

void FurnaceManager::CompleteSmelt(FurnaceBlockState& furnace) {
    if (!craftingSystem) return;

    const SmeltingRecipe* recipe = craftingSystem->FindSmeltingRecipe(furnace.inputItem);
    if (!recipe) return;

    // Consume input
    furnace.inputCount -= 1;
    if (furnace.inputCount <= 0) {
        furnace.inputItem.clear();
        furnace.inputCount = 0;
    }

    // Produce output
    if (furnace.outputItem.empty()) {
        furnace.outputItem = recipe->outputItemID;
    }
    furnace.outputCount += recipe->outputCount;
    if (furnace.outputCount > 64) furnace.outputCount = 64;

    // Award experience
    furnace.storedExperience += (float)recipe->experienceReward;

    furnace.smeltProgress = 0.0f;
    Logger::Info("[FurnaceManager] Smelted " + recipe->inputItemID + " -> " + recipe->outputItemID);
}

void FurnaceManager::UpdateFurnace(FurnaceBlockState& furnace, float deltaTime) {
    // If we have fuel burning, decrement it
    if (furnace.fuelBurnTime > 0.0f) {
        furnace.fuelBurnTime -= deltaTime;
        furnace.isLit = true;

        // If we can smelt, progress the smelting
        if (CanSmelt(furnace)) {
            const SmeltingRecipe* recipe = craftingSystem->FindSmeltingRecipe(furnace.inputItem);
            if (recipe) {
                furnace.smeltProgress += deltaTime / recipe->smeltTime;
                if (furnace.smeltProgress >= 1.0f) {
                    CompleteSmelt(furnace);
                }
            }
        }

        // If fuel ran out, try to consume more fuel
        if (furnace.fuelBurnTime <= 0.0f) {
            furnace.fuelBurnTime = 0.0f;
            furnace.isLit = false;

            if (furnace.fuelCount > 0 && CanSmelt(furnace)) {
                // Consume one fuel item
                furnace.fuelCount -= 1;
                furnace.fuelBurnTime = GetFuelValue(furnace.fuelItem);
                furnace.maxFuelBurnTime = furnace.fuelBurnTime;
                furnace.isLit = true;

                if (furnace.fuelCount <= 0) {
                    furnace.fuelItem.clear();
                    furnace.fuelCount = 0;
                }
            }
        }
    } else {
        // No fuel burning - try to start
        furnace.isLit = false;
        if (furnace.fuelCount > 0 && CanSmelt(furnace)) {
            furnace.fuelCount -= 1;
            furnace.fuelBurnTime = GetFuelValue(furnace.fuelItem);
            furnace.maxFuelBurnTime = furnace.fuelBurnTime;
            furnace.isLit = true;

            if (furnace.fuelCount <= 0) {
                furnace.fuelItem.clear();
                furnace.fuelCount = 0;
            }
        }
    }
}

void FurnaceManager::Update(float deltaTime) {
    for (auto& [key, furnace] : furnaces) {
        UpdateFurnace(furnace, deltaTime);
    }
}

bool FurnaceManager::IsLit(int x, int y, int z) const {
    auto it = furnaces.find(GetKey(x, y, z));
    if (it != furnaces.end()) {
        return it->second.isLit;
    }
    return false;
}

float FurnaceManager::GetSmeltProgress(int x, int y, int z) const {
    auto it = furnaces.find(GetKey(x, y, z));
    if (it != furnaces.end()) {
        return it->second.smeltProgress;
    }
    return 0.0f;
}

float FurnaceManager::GetFuelProgress(int x, int y, int z) const {
    auto it = furnaces.find(GetKey(x, y, z));
    if (it != furnaces.end()) {
        if (it->second.maxFuelBurnTime > 0.0f) {
            return it->second.fuelBurnTime / it->second.maxFuelBurnTime;
        }
    }
    return 0.0f;
}

std::vector<Vec3> FurnaceManager::GetAllFurnacePositions() const {
    std::vector<Vec3> positions;
    for (const auto& [key, furnace] : furnaces) {
        positions.push_back(furnace.position);
    }
    return positions;
}

std::string FurnaceManager::Serialize() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"furnaces\":[";
    bool first = true;
    for (const auto& [key, furnace] : furnaces) {
        if (!first) ss << ",";
        first = false;
        ss << "{";
        ss << "\"x\":" << (int)furnace.position.x << ",";
        ss << "\"y\":" << (int)furnace.position.y << ",";
        ss << "\"z\":" << (int)furnace.position.z << ",";
        ss << "\"inputItem\":\"" << furnace.inputItem << "\",";
        ss << "\"inputCount\":" << furnace.inputCount << ",";
        ss << "\"fuelItem\":\"" << furnace.fuelItem << "\",";
        ss << "\"fuelCount\":" << furnace.fuelCount << ",";
        ss << "\"outputItem\":\"" << furnace.outputItem << "\",";
        ss << "\"outputCount\":" << furnace.outputCount << ",";
        ss << "\"storedXP\":" << furnace.storedExperience;
        ss << "}";
    }
    ss << "]}";
    return ss.str();
}

bool FurnaceManager::Deserialize(const std::string& data) {
    (void)data;
    Logger::Info("[FurnaceManager] Deserialize not yet implemented");
    return false;
}

void FurnaceManager::RegisterFurnaceCraftingRecipe(CraftingSystem* crafting) {
    if (!crafting) return;

    CraftingRecipe furnace;
    furnace.recipeID = "furnace";
    furnace.displayName = "Furnace";
    furnace.outputItemID = "furnace";
    furnace.outputCount = 1;
    // Ring of cobblestone
    furnace.pattern = {
        {"cobblestone", "cobblestone", "cobblestone"},
        {"cobblestone", "",            "cobblestone"},
        {"cobblestone", "cobblestone", "cobblestone"}
    };
    crafting->AddRecipe(furnace);
    Logger::Info("[FurnaceManager] Registered furnace crafting recipe");
}

void FurnaceManager::RegisterDefaultSmeltingRecipes(CraftingSystem* crafting) {
    if (!crafting) return;

    // Iron ore -> Iron ingot
    SmeltingRecipe iron;
    iron.recipeID = "smelt_iron";
    iron.inputItemID = "iron_ore";
    iron.outputItemID = "iron_ingot";
    iron.outputCount = 1;
    iron.smeltTime = 10.0f;
    iron.experienceReward = 2;
    crafting->AddSmeltingRecipe(iron);

    // Gold ore -> Gold ingot
    SmeltingRecipe gold;
    gold.recipeID = "smelt_gold";
    gold.inputItemID = "gold_ore";
    gold.outputItemID = "gold_ingot";
    gold.outputCount = 1;
    gold.smeltTime = 10.0f;
    gold.experienceReward = 3;
    crafting->AddSmeltingRecipe(gold);

    // Sand -> Glass
    SmeltingRecipe glass;
    glass.recipeID = "smelt_glass";
    glass.inputItemID = "sand";
    glass.outputItemID = "glass";
    glass.outputCount = 1;
    glass.smeltTime = 5.0f;
    glass.experienceReward = 1;
    crafting->AddSmeltingRecipe(glass);

    // Cobblestone -> Stone
    SmeltingRecipe stone;
    stone.recipeID = "smelt_stone";
    stone.inputItemID = "cobblestone";
    stone.outputItemID = "stone";
    stone.outputCount = 1;
    stone.smeltTime = 5.0f;
    stone.experienceReward = 1;
    crafting->AddSmeltingRecipe(stone);

    // Clay ball -> Brick
    SmeltingRecipe brick;
    brick.recipeID = "smelt_brick";
    brick.inputItemID = "clay_ball";
    brick.outputItemID = "brick";
    brick.outputCount = 1;
    brick.smeltTime = 3.0f;
    brick.experienceReward = 1;
    crafting->AddSmeltingRecipe(brick);

    // Raw iron -> Iron ingot
    SmeltingRecipe rawIron;
    rawIron.recipeID = "smelt_raw_iron";
    rawIron.inputItemID = "raw_iron";
    rawIron.outputItemID = "iron_ingot";
    rawIron.outputCount = 1;
    rawIron.smeltTime = 10.0f;
    rawIron.experienceReward = 2;
    crafting->AddSmeltingRecipe(rawIron);

    // Raw gold -> Gold ingot
    SmeltingRecipe rawGold;
    rawGold.recipeID = "smelt_raw_gold";
    rawGold.inputItemID = "raw_gold";
    rawGold.outputItemID = "gold_ingot";
    rawGold.outputCount = 1;
    rawGold.smeltTime = 10.0f;
    rawGold.experienceReward = 3;
    crafting->AddSmeltingRecipe(rawGold);

    // Wood -> Charcoal
    SmeltingRecipe charcoal;
    charcoal.recipeID = "smelt_charcoal";
    charcoal.inputItemID = "wood";
    charcoal.outputItemID = "charcoal";
    charcoal.outputCount = 1;
    charcoal.smeltTime = 5.0f;
    charcoal.experienceReward = 1;
    crafting->AddSmeltingRecipe(charcoal);

    Logger::Info("[FurnaceManager] Registered 8 default smelting recipes");
}

} // namespace vge
