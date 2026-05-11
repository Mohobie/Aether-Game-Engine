#include "crafting.h"
#include "voxel/block_types.h"
#include "voxel/block_registry.h"
#include "core/logger.h"
#include <algorithm>
#include <sstream>

namespace vge {

// ==================== RecipeIngredient ====================

bool RecipeIngredient::Matches(const std::string& testID) const {
    if (itemID == testID) return true;
    for (const auto& sub : substitutes) {
        if (sub == testID) return true;
    }
    return false;
}

void RecipeIngredient::AddSubstitute(const std::string& substituteID) {
    substitutes.push_back(substituteID);
}

// ==================== CraftingRecipe ====================

CraftingRecipe::CraftingRecipe() 
    : outputType(BLOCK_AIR), outputCount(1), shapeless(false), 
      requiredLevel(0), discovered(false), hidden(false), 
      craftTime(0.0f), experienceReward(0) {
    pattern.resize(3, std::vector<std::string>(3, ""));
}

bool CraftingRecipe::MatchesInventory(const Inventory& inventory) const {
    auto required = GetRequiredItems();
    for (const auto& [itemID, count] : required) {
        if (!inventory.HasItem(itemID, count)) {
            return false;
        }
    }
    return true;
}

bool CraftingRecipe::MatchesGrid(const CraftingGrid& grid) const {
    if (shapeless) {
        auto gridCounts = grid.GetIngredientCounts();
        auto required = GetRequiredItems();
        for (const auto& [itemID, count] : required) {
            auto it = gridCounts.find(itemID);
            if (it == gridCounts.end() || it->second < count) {
                return false;
            }
        }
        return true;
    }
    
    // Shaped matching
    for (int startY = 0; startY <= CraftingGrid::GRID_SIZE - 3; ++startY) {
        for (int startX = 0; startX <= CraftingGrid::GRID_SIZE - 3; ++startX) {
            bool match = true;
            for (int y = 0; y < 3 && match; ++y) {
                for (int x = 0; x < 3 && match; ++x) {
                    std::string expected = pattern[y][x];
                    std::string actual = grid.GetItem(startX + x, startY + y);
                    if (expected != actual) {
                        match = false;
                    }
                }
            }
            if (match) return true;
        }
    }
    return false;
}

std::vector<std::pair<std::string, int>> CraftingRecipe::GetRequiredItems() const {
    std::vector<std::pair<std::string, int>> result;
    
    if (shapeless) {
        for (const auto& ing : ingredients) {
            result.push_back({ing.itemID, ing.count});
        }
    } else {
        std::unordered_map<std::string, int> counts;
        for (const auto& row : pattern) {
            for (const auto& itemID : row) {
                if (!itemID.empty()) {
                    counts[itemID]++;
                }
            }
        }
        for (const auto& [id, cnt] : counts) {
            result.push_back({id, cnt});
        }
    }
    
    return result;
}

// ==================== CraftingGrid ====================

CraftingGrid::CraftingGrid() {
    items.resize(GRID_SIZE, std::vector<std::string>(GRID_SIZE, ""));
}

void CraftingGrid::SetItem(int x, int y, const std::string& itemID) {
    if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
        items[y][x] = itemID;
    }
}

std::string CraftingGrid::GetItem(int x, int y) const {
    if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
        return items[y][x];
    }
    return "";
}

void CraftingGrid::Clear() {
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            items[y][x] = "";
        }
    }
}

bool CraftingGrid::IsEmpty() const {
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (!items[y][x].empty()) return false;
        }
    }
    return true;
}

std::unordered_map<std::string, int> CraftingGrid::GetIngredientCounts() const {
    std::unordered_map<std::string, int> counts;
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            const std::string& item = items[y][x];
            if (!item.empty()) {
                counts[item]++;
            }
        }
    }
    return counts;
}

// ==================== RecipeProgression ====================

RecipeProgression::RecipeProgression() : playerLevel(1), totalCrafts(0), totalSmelts(0) {}

bool RecipeProgression::IsDiscovered(const std::string& recipeID) const {
    auto it = discoveredRecipes.find(recipeID);
    return it != discoveredRecipes.end() && it->second;
}

void RecipeProgression::DiscoverRecipe(const std::string& recipeID) {
    discoveredRecipes[recipeID] = true;
    Logger::Info("[RecipeProgression] Discovered recipe: " + recipeID);
}

void RecipeProgression::DiscoverAll() {
    for (auto& [id, discovered] : discoveredRecipes) {
        discovered = true;
    }
}

void RecipeProgression::OnItemObtained(const std::string& itemID) {
    auto it = itemDiscoveryTriggers.find(itemID);
    if (it != itemDiscoveryTriggers.end()) {
        for (const auto& recipeID : it->second) {
            if (!IsDiscovered(recipeID)) {
                DiscoverRecipe(recipeID);
            }
        }
    }
}

void RecipeProgression::OnRecipeCrafted(const std::string& recipeID) {
    craftCounts[recipeID]++;
    totalCrafts++;
}

void RecipeProgression::OnLevelUp(int newLevel) {
    playerLevel = newLevel;
    auto it = levelDiscoveryTriggers.find(std::to_string(newLevel));
    if (it != levelDiscoveryTriggers.end()) {
        for (const auto& recipeID : it->second) {
            if (!IsDiscovered(recipeID)) {
                DiscoverRecipe(recipeID);
            }
        }
    }
}

int RecipeProgression::GetCraftCount(const std::string& recipeID) const {
    auto it = craftCounts.find(recipeID);
    return it != craftCounts.end() ? it->second : 0;
}

void RecipeProgression::SetPlayerLevel(int level) {
    playerLevel = level;
}

void RecipeProgression::RegisterItemTrigger(const std::string& itemID, const std::string& recipeID) {
    itemDiscoveryTriggers[itemID].push_back(recipeID);
    if (discoveredRecipes.find(recipeID) == discoveredRecipes.end()) {
        discoveredRecipes[recipeID] = false;
    }
}

void RecipeProgression::RegisterLevelTrigger(int level, const std::string& recipeID) {
    levelDiscoveryTriggers[std::to_string(level)].push_back(recipeID);
    if (discoveredRecipes.find(recipeID) == discoveredRecipes.end()) {
        discoveredRecipes[recipeID] = false;
    }
}

std::vector<std::string> RecipeProgression::GetDiscoveredRecipes() const {
    std::vector<std::string> result;
    for (const auto& [id, discovered] : discoveredRecipes) {
        if (discovered) {
            result.push_back(id);
        }
    }
    return result;
}

std::vector<std::string> RecipeProgression::GetUndiscoveredRecipes(const std::vector<CraftingRecipe>& allRecipes) const {
    std::vector<std::string> result;
    for (const auto& recipe : allRecipes) {
        if (!IsDiscovered(recipe.recipeID)) {
            result.push_back(recipe.recipeID);
        }
    }
    return result;
}

std::string RecipeProgression::Serialize() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"level\":" << playerLevel << ",";
    ss << "\"totalCrafts\":" << totalCrafts << ",";
    ss << "\"totalSmelts\":" << totalSmelts << ",";
    ss << "\"discovered\":[";
    bool first = true;
    for (const auto& [id, discovered] : discoveredRecipes) {
        if (discovered) {
            if (!first) ss << ",";
            first = false;
            ss << "\"" << id << "\"";
        }
    }
    ss << "],\"craftCounts\":{";
    first = true;
    for (const auto& [id, count] : craftCounts) {
        if (!first) ss << ",";
        first = false;
        ss << "\"" << id << "\":" << count;
    }
    ss << "}}";
    return ss.str();
}

bool RecipeProgression::Deserialize(const std::string& data) {
    Logger::Info("[RecipeProgression] Deserialize not yet implemented");
    return false;
}

// ==================== CraftingSystem ====================

CraftingSystem::CraftingSystem() {
    InitializeDefaultRecipes();
    InitializeDefaultSmelting();
}

void CraftingSystem::InitializeDefaultRecipes() {
    CraftingRecipe planks;
    planks.recipeID = "planks_from_wood";
    planks.displayName = "Wooden Planks";
    planks.outputItemID = "planks";
    planks.outputCount = 4;
    planks.pattern = {{"wood", "", ""}, {"", "", ""}, {"", "", ""}};
    AddRecipe(planks);
    
    CraftingRecipe sticks;
    sticks.recipeID = "sticks_from_planks";
    sticks.displayName = "Sticks";
    sticks.outputItemID = "stick";
    sticks.outputCount = 4;
    sticks.pattern = {{"planks", "", ""}, {"planks", "", ""}, {"", "", ""}};
    AddRecipe(sticks);
    
    CraftingRecipe table;
    table.recipeID = "crafting_table";
    table.displayName = "Crafting Table";
    table.outputItemID = "crafting_table";
    table.outputCount = 1;
    table.pattern = {{"planks", "planks", ""}, {"planks", "planks", ""}, {"", "", ""}};
    AddRecipe(table);
    
    CraftingRecipe pickaxe;
    pickaxe.recipeID = "wooden_pickaxe";
    pickaxe.displayName = "Wooden Pickaxe";
    pickaxe.outputItemID = "wooden_pickaxe";
    pickaxe.outputCount = 1;
    pickaxe.pattern = {{"planks", "planks", "planks"}, {"", "stick", ""}, {"", "stick", ""}};
    pickaxe.requiredLevel = 2;
    pickaxe.hidden = true;
    AddRecipe(pickaxe);
    
    CraftingRecipe stonePickaxe;
    stonePickaxe.recipeID = "stone_pickaxe";
    stonePickaxe.displayName = "Stone Pickaxe";
    stonePickaxe.outputItemID = "stone_pickaxe";
    stonePickaxe.outputCount = 1;
    stonePickaxe.pattern = {{"cobblestone", "cobblestone", "cobblestone"}, {"", "stick", ""}, {"", "stick", ""}};
    stonePickaxe.requiredStation = "crafting_table";
    stonePickaxe.requiredLevel = 3;
    stonePickaxe.hidden = true;
    AddRecipe(stonePickaxe);
    
    CraftingRecipe ironIngot;
    ironIngot.recipeID = "iron_ingot_from_nuggets";
    ironIngot.displayName = "Iron Ingot";
    ironIngot.outputItemID = "iron_ingot";
    ironIngot.outputCount = 1;
    ironIngot.shapeless = true;
    ironIngot.ingredients.push_back(RecipeIngredient("iron_nugget", 9));
    AddRecipe(ironIngot);
    
    CraftingRecipe torch;
    torch.recipeID = "torch";
    torch.displayName = "Torch";
    torch.outputItemID = "torch";
    torch.outputCount = 4;
    torch.pattern = {{"coal", "", ""}, {"stick", "", ""}, {"", "", ""}};
    RecipeIngredient coal("coal", 1);
    coal.AddSubstitute("charcoal");
    torch.ingredients.push_back(coal);
    torch.ingredients.push_back(RecipeIngredient("stick", 1));
    AddRecipe(torch);
}

void CraftingSystem::InitializeDefaultSmelting() {
    SmeltingRecipe iron;
    iron.recipeID = "smelt_iron";
    iron.inputItemID = "iron_ore";
    iron.outputItemID = "iron_ingot";
    iron.outputCount = 1;
    iron.smeltTime = 10.0f;
    iron.fuelConsumption = 1;
    iron.experienceReward = 2;
    AddSmeltingRecipe(iron);
    
    SmeltingRecipe gold;
    gold.recipeID = "smelt_gold";
    gold.inputItemID = "gold_ore";
    gold.outputItemID = "gold_ingot";
    gold.outputCount = 1;
    gold.smeltTime = 10.0f;
    gold.fuelConsumption = 1;
    gold.experienceReward = 3;
    AddSmeltingRecipe(gold);
    
    SmeltingRecipe glass;
    glass.recipeID = "smelt_glass";
    glass.inputItemID = "sand";
    glass.outputItemID = "glass";
    glass.outputCount = 1;
    glass.smeltTime = 5.0f;
    glass.fuelConsumption = 1;
    glass.experienceReward = 1;
    AddSmeltingRecipe(glass);
    
    SmeltingRecipe stone;
    stone.recipeID = "smelt_stone";
    stone.inputItemID = "cobblestone";
    stone.outputItemID = "stone";
    stone.outputCount = 1;
    stone.smeltTime = 5.0f;
    stone.fuelConsumption = 1;
    stone.experienceReward = 1;
    AddSmeltingRecipe(stone);
    
    SmeltingRecipe brick;
    brick.recipeID = "smelt_brick";
    brick.inputItemID = "clay_ball";
    brick.outputItemID = "brick";
    brick.outputCount = 1;
    brick.smeltTime = 3.0f;
    brick.fuelConsumption = 1;
    brick.experienceReward = 1;
    AddSmeltingRecipe(brick);
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
    recipeIndexByID.erase(it);
    
    recipesByOutput.clear();
    recipesByInput.clear();
    recipeIndexByID.clear();
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
    if (it != recipeIndexByID.end()) {
        return &recipes[it->second];
    }
    return nullptr;
}

const CraftingRecipe* CraftingSystem::FindRecipe(const CraftingGrid& grid, const std::string& station) const {
    for (const auto& recipe : recipes) {
        if (!station.empty() && recipe.requiredStation != station) continue;
        if (recipe.MatchesGrid(grid)) {
            return &recipe;
        }
    }
    return nullptr;
}

const CraftingRecipe* CraftingSystem::FindShapelessRecipe(const Inventory& inventory, const std::string& station) const {
    for (const auto& recipe : recipes) {
        if (!recipe.shapeless) continue;
        if (!station.empty() && recipe.requiredStation != station) continue;
        if (recipe.MatchesInventory(inventory)) {
            return &recipe;
        }
    }
    return nullptr;
}

void CraftingSystem::AddSmeltingRecipe(const SmeltingRecipe& recipe) {
    size_t index = smeltingRecipes.size();
    smeltingRecipes.push_back(recipe);
    smeltingIndexByID[recipe.recipeID] = index;
    smeltingByOutput[recipe.outputItemID].push_back(index);
}

const SmeltingRecipe* CraftingSystem::GetSmeltingRecipe(const std::string& recipeID) const {
    auto it = smeltingIndexByID.find(recipeID);
    if (it != smeltingIndexByID.end()) {
        return &smeltingRecipes[it->second];
    }
    return nullptr;
}

const SmeltingRecipe* CraftingSystem::FindSmeltingRecipe(const std::string& inputItemID) const {
    for (const auto& recipe : smeltingRecipes) {
        if (recipe.inputItemID == inputItemID) {
            return &recipe;
        }
    }
    return nullptr;
}

std::vector<const CraftingRecipe*> CraftingSystem::GetRecipesForOutput(const std::string& itemID) const {
    std::vector<const CraftingRecipe*> result;
    auto it = recipesByOutput.find(itemID);
    if (it != recipesByOutput.end()) {
        for (size_t index : it->second) {
            result.push_back(&recipes[index]);
        }
    }
    return result;
}

std::vector<const CraftingRecipe*> CraftingSystem::GetRecipesUsingInput(const std::string& itemID) const {
    std::vector<const CraftingRecipe*> result;
    auto it = recipesByInput.find(itemID);
    if (it != recipesByInput.end()) {
        for (size_t index : it->second) {
            result.push_back(&recipes[index]);
        }
    }
    return result;
}

std::vector<const CraftingRecipe*> CraftingSystem::GetAllRecipes() const {
    std::vector<const CraftingRecipe*> result;
    for (const auto& recipe : recipes) {
        result.push_back(&recipe);
    }
    return result;
}

std::vector<const SmeltingRecipe*> CraftingSystem::GetAllSmeltingRecipes() const {
    std::vector<const SmeltingRecipe*> result;
    for (const auto& recipe : smeltingRecipes) {
        result.push_back(&recipe);
    }
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

bool CraftingSystem::CanCraft(const CraftingRecipe& recipe, const Inventory& inventory, 
                               const RecipeProgression& progression) const {
    if (recipe.hidden && !progression.IsDiscovered(recipe.recipeID)) {
        return false;
    }
    
    if (recipe.requiredLevel > progression.GetPlayerLevel()) {
        return false;
    }
    
    return recipe.MatchesInventory(inventory);
}

std::vector<std::pair<std::string, int>> CraftingSystem::GetMissingIngredients(
    const CraftingRecipe& recipe, const Inventory& inventory) const {
    
    std::vector<std::pair<std::string, int>> missing;
    auto required = recipe.GetRequiredItems();
    
    for (const auto& [itemID, count] : required) {
        int available = inventory.GetItemCount(itemID);
        if (available < count) {
            missing.push_back({itemID, count - available});
        }
    }
    
    return missing;
}

void CraftingSystem::PrintRecipes() const {
    Logger::Info("=== Crafting Recipes (" + std::to_string(recipes.size()) + ") ===");
    for (const auto& recipe : recipes) {
        Logger::Info("  " + recipe.recipeID + " -> " + recipe.outputItemID + " x" + std::to_string(recipe.outputCount));
    }
    
    Logger::Info("=== Smelting Recipes (" + std::to_string(smeltingRecipes.size()) + ") ===");
    for (const auto& recipe : smeltingRecipes) {
        Logger::Info("  " + recipe.recipeID + ": " + recipe.inputItemID + " -> " + recipe.outputItemID);
    }
}

// ==================== CraftingQueue ====================

CraftingQueue::CraftingQueue() : currentEntry(nullptr), isProcessing(false) {}

CraftingQueue::~CraftingQueue() {
    delete currentEntry;
    while (!queue.empty()) {
        queue.pop();
    }
}

void CraftingQueue::Enqueue(const std::string& recipeID, int quantity, float craftTime) {
    CraftingQueueEntry entry;
    entry.recipeID = recipeID;
    entry.quantity = quantity;
    entry.totalTime = craftTime * quantity;
    entry.progress = 0.0f;
    entry.completed = false;
    queue.push(entry);
    Logger::Info("[CraftingQueue] Enqueued: " + recipeID + " x" + std::to_string(quantity));
}

bool CraftingQueue::Dequeue() {
    if (queue.empty()) return false;
    queue.pop();
    return true;
}

void CraftingQueue::Clear() {
    while (!queue.empty()) {
        queue.pop();
    }
    delete currentEntry;
    currentEntry = nullptr;
    isProcessing = false;
}

bool CraftingQueue::IsEmpty() const {
    return queue.empty() && (currentEntry == nullptr || currentEntry->completed);
}

size_t CraftingQueue::GetQueueSize() const {
    return queue.size() + (currentEntry && !currentEntry->completed ? 1 : 0);
}

void CraftingQueue::Update(float deltaTime) {
    if (!isProcessing) {
        if (!queue.empty()) {
            currentEntry = new CraftingQueueEntry(queue.front());
            queue.pop();
            isProcessing = true;
            Logger::Info("[CraftingQueue] Started crafting: " + currentEntry->recipeID);
        } else {
            if (onQueueEmpty) {
                onQueueEmpty();
            }
            return;
        }
    }
    
    if (currentEntry && !currentEntry->completed) {
        currentEntry->progress += deltaTime;
        
        if (onCraftProgress) {
            onCraftProgress(*currentEntry);
        }
        
        if (currentEntry->progress >= currentEntry->totalTime) {
            currentEntry->completed = true;
            isProcessing = false;
            
            if (onCraftComplete) {
                onCraftComplete(*currentEntry);
            }
            
            Logger::Info("[CraftingQueue] Completed: " + currentEntry->recipeID);
            
            delete currentEntry;
            currentEntry = nullptr;
            
            if (!queue.empty()) {
                currentEntry = new CraftingQueueEntry(queue.front());
                queue.pop();
                isProcessing = true;
            } else {
                if (onQueueEmpty) {
                    onQueueEmpty();
                }
            }
        }
    }
}

float CraftingQueue::GetCurrentProgress() const {
    if (currentEntry && currentEntry->totalTime > 0) {
        return currentEntry->progress / currentEntry->totalTime;
    }
    return 0.0f;
}

void CraftingQueue::SetOnCraftComplete(std::function<void(const CraftingQueueEntry&)> callback) {
    onCraftComplete = callback;
}

void CraftingQueue::SetOnCraftProgress(std::function<void(const CraftingQueueEntry&)> callback) {
    onCraftProgress = callback;
}

void CraftingQueue::SetOnQueueEmpty(std::function<void()> callback) {
    onQueueEmpty = callback;
}

// ==================== SmeltingSystem ====================

SmeltingSystem::SmeltingSystem(CraftingSystem* craftSystem, int numSlots) 
    : craftingSystem(craftSystem) {
    slots.resize(numSlots);
    
    RegisterFuel("coal", 80.0f);
    RegisterFuel("charcoal", 80.0f);
    RegisterFuel("wood", 15.0f);
    RegisterFuel("planks", 15.0f);
    RegisterFuel("stick", 5.0f);
    RegisterFuel("lava_bucket", 1000.0f);
}

void SmeltingSystem::RegisterFuel(const std::string& itemID, float burnTime) {
    fuelValues[itemID] = burnTime;
}

float SmeltingSystem::GetFuelValue(const std::string& itemID) const {
    auto it = fuelValues.find(itemID);
    return it != fuelValues.end() ? it->second : 0.0f;
}

bool SmeltingSystem::IsFuel(const std::string& itemID) const {
    return fuelValues.find(itemID) != fuelValues.end();
}

SmeltingSlot& SmeltingSystem::GetSlot(int index) {
    return slots[index];
}

const SmeltingSlot& SmeltingSystem::GetSlot(int index) const {
    return slots[index];
}

bool SmeltingSystem::AddInput(int slotIndex, const std::string& itemID, int count) {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    
    SmeltingSlot& slot = slots[slotIndex];
    if (slot.inputItemID.empty() || slot.inputItemID == itemID) {
        slot.inputItemID = itemID;
        slot.inputCount += count;
        return true;
    }
    return false;
}

bool SmeltingSystem::AddFuel(int slotIndex, const std::string& itemID, int count) {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    if (!IsFuel(itemID)) return false;
    
    SmeltingSlot& slot = slots[slotIndex];
    if (slot.fuelItemID.empty() || slot.fuelItemID == itemID) {
        slot.fuelItemID = itemID;
        slot.fuelCount += count;
        return true;
    }
    return false;
}

bool SmeltingSystem::RemoveOutput(int slotIndex, int count) {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    
    SmeltingSlot& slot = slots[slotIndex];
    if (slot.outputCount < count) return false;
    
    slot.outputCount -= count;
    if (slot.outputCount <= 0) {
        slot.outputItemID.clear();
        slot.outputCount = 0;
    }
    return true;
}

bool SmeltingSystem::CanSmelt(int slotIndex) const {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    
    const SmeltingSlot& slot = slots[slotIndex];
    if (!slot.HasInput() || !slot.HasFuel()) return false;
    
    const SmeltingRecipe* recipe = craftingSystem->FindSmeltingRecipe(slot.inputItemID);
    if (!recipe) return false;
    
    if (!slot.outputItemID.empty() && slot.outputItemID != recipe->outputItemID) {
        return false;
    }
    
    return true;
}

bool SmeltingSystem::StartSmelting(int slotIndex) {
    if (!CanSmelt(slotIndex)) return false;
    
    SmeltingSlot& slot = slots[slotIndex];
    slot.isBurning = true;
    
    const SmeltingRecipe* recipe = craftingSystem->FindSmeltingRecipe(slot.inputItemID);
    if (recipe && slot.outputItemID.empty()) {
        slot.outputItemID = recipe->outputItemID;
    }
    
    Logger::Info("[SmeltingSystem] Started smelting in slot " + std::to_string(slotIndex));
    return true;
}

void SmeltingSystem::StopSmelting(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return;
    slots[slotIndex].isBurning = false;
}

} // namespace vge
