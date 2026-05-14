#pragma once
#include "core/crafting.h"
#include "voxel/block_types.h"
#include "math/vec3.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace vge {

// ============================================
// Furnace Block State
// ============================================
struct FurnaceBlockState {
    Vec3 position;
    bool isLit;
    float fuelBurnTime;
    float maxFuelBurnTime;
    float smeltProgress;
    std::string inputItem;
    int inputCount;
    std::string fuelItem;
    int fuelCount;
    std::string outputItem;
    int outputCount;
    float storedExperience;
    bool active;

    FurnaceBlockState()
        : isLit(false), fuelBurnTime(0.0f), maxFuelBurnTime(0.0f),
          smeltProgress(0.0f), inputCount(0), fuelCount(0),
          outputCount(0), storedExperience(0.0f), active(false) {}
};

// ============================================
// Furnace Manager - Handles all furnace blocks in world
// ============================================
class FurnaceManager {
private:
    std::unordered_map<std::string, FurnaceBlockState> furnaces;
    CraftingSystem* craftingSystem;
    std::vector<std::string> fuelItems;

    std::string GetKey(int x, int y, int z) const;
    void UpdateFurnace(FurnaceBlockState& furnace, float deltaTime);
    bool CanSmelt(const FurnaceBlockState& furnace) const;
    void CompleteSmelt(FurnaceBlockState& furnace);
    float GetFuelValue(const std::string& itemID) const;

public:
    explicit FurnaceManager(CraftingSystem* craftSystem);

    // Furnace placement/removal
    void PlaceFurnace(int x, int y, int z);
    void RemoveFurnace(int x, int y, int z);
    bool HasFurnace(int x, int y, int z) const;
    FurnaceBlockState* GetFurnace(int x, int y, int z);

    // Item management
    bool AddInput(int x, int y, int z, const std::string& itemID, int count);
    bool AddFuel(int x, int y, int z, const std::string& itemID, int count);
    bool RemoveOutput(int x, int y, int z, int count);
    bool TakeInput(int x, int y, int z, int count);
    bool TakeFuel(int x, int y, int z, int count);

    // Experience
    float CollectExperience(int x, int y, int z);

    // Update all furnaces
    void Update(float deltaTime);

    // Query
    bool IsLit(int x, int y, int z) const;
    float GetSmeltProgress(int x, int y, int z) const;
    float GetFuelProgress(int x, int y, int z) const;

    // Fuel registration
    void RegisterFuel(const std::string& itemID, float burnTime);
    bool IsFuel(const std::string& itemID) const;

    // Get all furnace positions
    std::vector<Vec3> GetAllFurnacePositions() const;

    // Serialization
    std::string Serialize() const;
    bool Deserialize(const std::string& data);

    // Static recipe registration
    static void RegisterFurnaceCraftingRecipe(CraftingSystem* crafting);
    static void RegisterDefaultSmeltingRecipes(CraftingSystem* crafting);
};

} // namespace vge
