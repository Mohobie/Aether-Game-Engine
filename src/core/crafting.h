#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>
#include "voxel/block_types.h"
#include "core/item_system.h"

namespace vge {

// ============================================
// Modern CraftingResult (forward declare first)
// ============================================
struct CraftingResult2 {
    bool success;
    std::string outputItemID;
    int outputCount;
    std::string recipeID;
    float craftTime;
    
    CraftingResult2() : success(false), outputCount(0), craftTime(0.0f) {}
};

// ============================================
// Legacy CraftingResult
// ============================================
struct CraftingResult {
    bool success;
    std::string outputItemID;
    BlockTypeID outputType;
    int outputCount;
    std::string recipeID;
    float craftTime;
    
    CraftingResult() : success(false), outputType(BLOCK_AIR), outputCount(0), craftTime(0.0f) {}
};

// ============================================
// Ingredient with substitution support
// ============================================
struct RecipeIngredient {
    std::string itemID;                    // Primary item ID
    std::vector<std::string> substitutes; // Alternative items that work
    int count;                            // Quantity needed
    
    RecipeIngredient() : count(1) {}
    RecipeIngredient(const std::string& id, int cnt = 1) : itemID(id), count(cnt) {}
    
    // Check if an item matches this ingredient (including substitutes)
    bool Matches(const std::string& testID) const;
    void AddSubstitute(const std::string& substituteID);
};

// ============================================
// Crafting Recipe (modern - uses item IDs)
// ============================================
struct CraftingRecipe {
    std::string recipeID;                  // Unique recipe identifier
    std::string displayName;              // Human-readable name
    std::string description;              // Recipe description
    
    // Grid pattern (3x3 max, empty strings = empty slot)
    std::vector<std::vector<std::string>> pattern;
    
    // Alternative: ingredient list for shapeless recipes
    std::vector<RecipeIngredient> ingredients;
    bool shapeless;                        // If true, pattern doesn't matter
    
    // Output
    std::string outputItemID;
    int outputCount;
    
    // Requirements
    std::string requiredStation;           // e.g., "crafting_table", "furnace", "anvil"
    int requiredLevel;                     // Player progression level required
    bool discovered;                     // Has player discovered this recipe?
    bool hidden;                         // Hidden until discovered
    
    // Metadata
    float craftTime;                      // Time to craft (seconds, 0 = instant)
    int experienceReward;               // XP given on craft
    
    CraftingRecipe();
    
    // Legacy compatibility
    std::vector<std::vector<BlockTypeID>> blockPattern;
    BlockTypeID outputType;
    
    // Check if inventory items match this recipe
    bool MatchesInventory(const Inventory& inventory) const;
    
    // Check if a crafting grid matches
    bool MatchesGrid(const class CraftingGrid& grid) const;
    
    // Get required ingredients as flat list
    std::vector<std::pair<std::string, int>> GetRequiredItems() const;
};

// ============================================
// Crafting Grid (3x3)
// ============================================
class CraftingGrid {
private:
    std::vector<std::vector<std::string>> items;  // Item IDs instead of BlockTypeID
    
public:
    static constexpr int GRID_SIZE = 3;
    
    CraftingGrid();
    
    void SetItem(int x, int y, const std::string& itemID);
    std::string GetItem(int x, int y) const;
    void Clear();
    bool IsEmpty() const;
    
    // Convert to ingredient counts for shapeless matching
    std::unordered_map<std::string, int> GetIngredientCounts() const;
};

// ============================================
// Crafting Queue Entry
// ============================================
struct CraftingQueueEntry {
    std::string recipeID;
    int quantity;                         // How many to craft
    float progress;                       // 0.0 to 1.0
    float totalTime;                      // Total craft time
    bool completed;
    
    CraftingQueueEntry() : quantity(1), progress(0.0f), totalTime(0.0f), completed(false) {}
};

// ============================================
// Smelting Recipe
// ============================================
struct SmeltingRecipe {
    std::string recipeID;
    std::string inputItemID;
    std::string outputItemID;
    int outputCount;
    float smeltTime;                      // Seconds to smelt one
    int fuelConsumption;                  // Fuel units per item
    int experienceReward;
    
    SmeltingRecipe() : outputCount(1), smeltTime(10.0f), fuelConsumption(1), experienceReward(0) {}
};

// ============================================
// Smelting Slot / Furnace State
// ============================================
struct SmeltingSlot {
    std::string inputItemID;
    int inputCount;
    std::string fuelItemID;
    int fuelCount;
    std::string outputItemID;
    int outputCount;
    
    float progress;                       // Smelting progress 0.0-1.0
    float fuelBurnTime;                  // Remaining fuel time
    float maxFuelBurnTime;               // Max fuel time for current fuel
    bool isBurning;
    
    SmeltingSlot() : inputCount(0), fuelCount(0), outputCount(0), 
                     progress(0.0f), fuelBurnTime(0.0f), maxFuelBurnTime(0.0f), isBurning(false) {}
    
    bool HasInput() const { return !inputItemID.empty() && inputCount > 0; }
    bool HasFuel() const { return !fuelItemID.empty() && fuelCount > 0; }
    bool HasOutput() const { return !outputItemID.empty() && outputCount > 0; }
    bool IsActive() const { return isBurning; }
};

// ============================================
// Recipe Discovery / Progression
// ============================================
class RecipeProgression {
private:
    std::unordered_map<std::string, bool> discoveredRecipes;  // recipeID -> discovered
    std::unordered_map<std::string, int> craftCounts;         // recipeID -> times crafted
    int playerLevel;
    int totalCrafts;
    int totalSmelts;
    
    // Discovery triggers
    std::unordered_map<std::string, std::vector<std::string>> itemDiscoveryTriggers; // itemID -> recipes unlocked
    std::unordered_map<std::string, std::vector<std::string>> levelDiscoveryTriggers; // level -> recipes unlocked
    
public:
    RecipeProgression();
    
    // Discovery
    bool IsDiscovered(const std::string& recipeID) const;
    void DiscoverRecipe(const std::string& recipeID);
    void DiscoverAll();
    
    // Trigger discovery by item pickup/craft
    void OnItemObtained(const std::string& itemID);
    void OnRecipeCrafted(const std::string& recipeID);
    void OnLevelUp(int newLevel);
    
    // Progress tracking
    int GetCraftCount(const std::string& recipeID) const;
    int GetTotalCrafts() const { return totalCrafts; }
    int GetTotalSmelts() const { return totalSmelts; }
    int GetPlayerLevel() const { return playerLevel; }
    void SetPlayerLevel(int level);
    
    // Register discovery triggers
    void RegisterItemTrigger(const std::string& itemID, const std::string& recipeID);
    void RegisterLevelTrigger(int level, const std::string& recipeID);
    
    // Get discovered recipes
    std::vector<std::string> GetDiscoveredRecipes() const;
    std::vector<std::string> GetUndiscoveredRecipes(const std::vector<CraftingRecipe>& allRecipes) const;
    
    // Serialization
    std::string Serialize() const;
    bool Deserialize(const std::string& data);
};

// ============================================
// Main Crafting System
// ============================================
class CraftingSystem {
private:
    std::vector<CraftingRecipe> recipes;
    std::vector<SmeltingRecipe> smeltingRecipes;
    
    // Recipe lookup by ID
    std::unordered_map<std::string, size_t> recipeIndexByID;
    std::unordered_map<std::string, size_t> smeltingIndexByID;
    
    // Recipe lookup by output
    std::unordered_map<std::string, std::vector<size_t>> recipesByOutput;
    std::unordered_map<std::string, std::vector<size_t>> smeltingByOutput;
    
    // Recipe lookup by input (for quick matching)
    std::unordered_map<std::string, std::vector<size_t>> recipesByInput;
    
    void InitializeDefaultRecipes();
    void InitializeDefaultSmelting();
    
public:
    CraftingSystem();
    
    // Recipe management
    void AddRecipe(const CraftingRecipe& recipe);
    void RemoveRecipe(const std::string& recipeID);
    const CraftingRecipe* GetRecipe(const std::string& recipeID) const;
    const CraftingRecipe* FindRecipe(const CraftingGrid& grid, const std::string& station = "") const;
    const CraftingRecipe* FindShapelessRecipe(const Inventory& inventory, const std::string& station = "") const;
    
    // Smelting management
    void AddSmeltingRecipe(const SmeltingRecipe& recipe);
    const SmeltingRecipe* GetSmeltingRecipe(const std::string& recipeID) const;
    const SmeltingRecipe* FindSmeltingRecipe(const std::string& inputItemID) const;
    
    // Query
    std::vector<const CraftingRecipe*> GetRecipesForOutput(const std::string& itemID) const;
    std::vector<const CraftingRecipe*> GetRecipesUsingInput(const std::string& itemID) const;
    std::vector<const CraftingRecipe*> GetAllRecipes() const;
    std::vector<const SmeltingRecipe*> GetAllSmeltingRecipes() const;
    
    // Crafting
    CraftingResult2 TryCraft(const CraftingGrid& grid, const Inventory* playerInventory = nullptr, 
                           const std::string& station = "") const;
    
    // Check if player can craft (has ingredients + discovered)
    bool CanCraft(const CraftingRecipe& recipe, const Inventory& inventory, 
                  const RecipeProgression& progression) const;
    
    // Get missing ingredients
    std::vector<std::pair<std::string, int>> GetMissingIngredients(
        const CraftingRecipe& recipe, const Inventory& inventory) const;
    
    // Print/debug
    void PrintRecipes() const;
    size_t GetRecipeCount() const { return recipes.size(); }
    size_t GetSmeltingRecipeCount() const { return smeltingRecipes.size(); }
    
    // Legacy compatibility
    const std::vector<CraftingRecipe>& GetRecipes() const { return recipes; }
};

// ============================================
// Crafting Queue Manager
// ============================================
class CraftingQueue {
private:
    std::queue<CraftingQueueEntry> queue;
    CraftingQueueEntry* currentEntry;
    bool isProcessing;
    
    // Callbacks
    std::function<void(const CraftingQueueEntry&)> onCraftComplete;
    std::function<void(const CraftingQueueEntry&)> onCraftProgress;
    std::function<void()> onQueueEmpty;
    
public:
    CraftingQueue();
    ~CraftingQueue();
    
    // Queue management
    void Enqueue(const std::string& recipeID, int quantity = 1, float craftTime = 0.0f);
    bool Dequeue();
    void Clear();
    bool IsEmpty() const;
    size_t GetQueueSize() const;
    
    // Processing
    void Update(float deltaTime);
    bool IsProcessing() const { return isProcessing; }
    const CraftingQueueEntry* GetCurrentEntry() const { return currentEntry; }
    float GetCurrentProgress() const;
    
    // Callbacks
    void SetOnCraftComplete(std::function<void(const CraftingQueueEntry&)> callback);
    void SetOnCraftProgress(std::function<void(const CraftingQueueEntry&)> callback);
    void SetOnQueueEmpty(std::function<void()> callback);
};

// ============================================
// Smelting System / Furnace Manager
// ============================================
class SmeltingSystem {
private:
    std::vector<SmeltingSlot> slots;
    std::unordered_map<std::string, float> fuelValues;  // itemID -> burn time in seconds
    
    CraftingSystem* craftingSystem;
    
public:
    explicit SmeltingSystem(CraftingSystem* craftSystem, int numSlots = 1);
    
    // Slot management
    SmeltingSlot& GetSlot(int index);
    const SmeltingSlot& GetSlot(int index) const;
    int GetSlotCount() const { return (int)slots.size(); }
    
    // Fuel management
    void RegisterFuel(const std::string& itemID, float burnTime);
    float GetFuelValue(const std::string& itemID) const;
    bool IsFuel(const std::string& itemID) const;
    
    // Operations
    bool AddInput(int slotIndex, const std::string& itemID, int count);
    bool AddFuel(int slotIndex, const std::string& itemID, int count);
    bool RemoveOutput(int slotIndex, int count);
    bool CanSmelt(int slotIndex) const;
    
    // Update (call every frame)
    void Update(float deltaTime);
    
    // Start/stop smelting
    bool StartSmelting(int slotIndex);
    void StopSmelting(int slotIndex);
    
    // Check if slot is currently smelting
    bool IsSmelting(int slotIndex) const;
    float GetSmeltProgress(int slotIndex) const;
    
    // Get remaining fuel time
    float GetRemainingFuelTime(int slotIndex) const;
    
    // Process one tick of smelting
    bool ProcessSmeltingTick(int slotIndex, float deltaTime);
    
    // Serialization
    std::string Serialize() const;
    bool Deserialize(const std::string& data);
};

} // namespace vge