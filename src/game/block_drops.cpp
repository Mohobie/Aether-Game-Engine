#include "block_drops.h"
#include <cstdlib>

namespace vge {

BlockDropSystem::BlockDropSystem() {
    InitializeDefaultDrops();
}

BlockDropSystem::~BlockDropSystem() {
}

void BlockDropSystem::RegisterDrops(const std::string& blockId, const std::vector<BlockDrop>& drops) {
    dropTable[blockId] = drops;
}

std::vector<std::pair<std::string, int>> BlockDropSystem::GetDrops(
    const std::string& blockId,
    const std::string& toolType,
    int toolTier
) const {
    std::vector<std::pair<std::string, int>> result;
    
    auto it = dropTable.find(blockId);
    if (it == dropTable.end()) {
        // Default: drop the block itself
        result.push_back({blockId, 1});
        return result;
    }
    
    for (const auto& drop : it->second) {
        // Check tool requirement
        if (drop.requiresTool) {
            if (toolType.empty()) continue; // No tool, no drop
            if (drop.requiredToolType != toolType) continue; // Wrong tool type
            if (toolTier < drop.requiredToolTier) continue; // Tool tier too low
        }
        
        // Check chance
        float roll = static_cast<float>(rand()) / RAND_MAX;
        if (roll > drop.chance) continue;
        
        // Determine count
        int count = drop.minCount;
        if (drop.maxCount > drop.minCount) {
            count += rand() % (drop.maxCount - drop.minCount + 1);
        }
        
        result.push_back({drop.itemId, count});
    }
    
    return result;
}

bool BlockDropSystem::CanHarvest(const std::string& blockId, const std::string& toolType, int toolTier) const {
    auto it = dropTable.find(blockId);
    if (it == dropTable.end()) return true; // Default: can always harvest
    
    for (const auto& drop : it->second) {
        if (!drop.requiresTool) return true;
        if (toolType.empty()) continue;
        if (drop.requiredToolType != toolType) continue;
        if (toolTier >= drop.requiredToolTier) return true;
    }
    
    return false;
}

void BlockDropSystem::InitializeDefaultDrops() {
    // Stone -> Cobblestone (requires pickaxe)
    RegisterDrops("stone", {
        {"cobblestone", 1, 1, 1.0f, true, "pickaxe", 0}
    });
    
    // Coal Ore -> Coal (requires pickaxe)
    RegisterDrops("coal_ore", {
        {"coal", 1, 1, 1.0f, true, "pickaxe", 0}
    });
    
    // Iron Ore -> Raw Iron (requires stone pickaxe+)
    RegisterDrops("iron_ore", {
        {"raw_iron", 1, 1, 1.0f, true, "pickaxe", 1}
    });
    
    // Gold Ore -> Raw Gold (requires iron pickaxe+)
    RegisterDrops("gold_ore", {
        {"raw_gold", 1, 1, 1.0f, true, "pickaxe", 2}
    });
    
    // Diamond Ore -> Diamond (requires iron pickaxe+)
    RegisterDrops("diamond_ore", {
        {"diamond", 1, 1, 1.0f, true, "pickaxe", 2}
    });
    
    // Emerald Ore -> Emerald (requires iron pickaxe+)
    RegisterDrops("emerald_ore", {
        {"emerald", 1, 1, 1.0f, true, "pickaxe", 2}
    });
    
    // Wood -> Wood (requires axe)
    RegisterDrops("wood", {
        {"wood", 1, 1, 1.0f, true, "axe", 0}
    });
    
    // Leaves -> Sapling (20% chance, no tool)
    RegisterDrops("leaves", {
        {"sapling", 1, 1, 0.2f, false, "", 0}
    });
    
    // Grass -> Dirt (no tool)
    RegisterDrops("grass", {
        {"dirt", 1, 1, 1.0f, false, "", 0}
    });
    
    // Dirt -> Dirt (no tool)
    RegisterDrops("dirt", {
        {"dirt", 1, 1, 1.0f, false, "", 0}
    });
    
    // Sand -> Sand (requires shovel)
    RegisterDrops("sand", {
        {"sand", 1, 1, 1.0f, true, "shovel", 0}
    });
    
    // Gravel -> Flint (10% chance, requires shovel)
    RegisterDrops("gravel", {
        {"gravel", 1, 1, 0.9f, true, "shovel", 0},
        {"flint", 1, 1, 0.1f, true, "shovel", 0}
    });
    
    // Glass -> Nothing (breaks)
    RegisterDrops("glass", {});
    
    // Flower -> Flower (no tool)
    RegisterDrops("flower", {
        {"flower", 1, 1, 1.0f, false, "", 0}
    });
    
    // Cactus -> Cactus (no tool)
    RegisterDrops("cactus", {
        {"cactus", 1, 1, 1.0f, false, "", 0}
    });
}

} // namespace vge
