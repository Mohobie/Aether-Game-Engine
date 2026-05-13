#pragma once
#include "voxel/block_types.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace vge {

// ============================================
// Block Drop Definition
// ============================================
struct BlockDrop {
    std::string itemId;
    int minCount;
    int maxCount;
    float chance; // 0.0-1.0
    bool requiresTool;
    std::string requiredToolType; // "pickaxe", "axe", "shovel"
    int requiredToolTier; // 0=wood, 1=stone, 2=iron, 3=diamond
};

// ============================================
// Block Drop System
// ============================================
class BlockDropSystem {
public:
    BlockDropSystem();
    ~BlockDropSystem();

    // Register drops for a block
    void RegisterDrops(const std::string& blockId, const std::vector<BlockDrop>& drops);
    
    // Get drops for a block (checks tool requirements)
    std::vector<std::pair<std::string, int>> GetDrops(
        const std::string& blockId,
        const std::string& toolType = "",
        int toolTier = 0
    ) const;
    
    // Check if block drops anything with current tool
    bool CanHarvest(const std::string& blockId, const std::string& toolType, int toolTier) const;
    
    // Initialize default drops
    void InitializeDefaultDrops();

private:
    std::unordered_map<std::string, std::vector<BlockDrop>> dropTable;
};

} // namespace vge
