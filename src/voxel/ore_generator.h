#pragma once
#include "voxel/world.h"
#include "math/vec3.h"
#include <vector>

namespace vge {

// ============================================
// Ore Type Definition
// ============================================
struct OreType {
    std::string blockId;
    BlockTypeID blockType;
    float rarity;        // 0.0-1.0, lower = rarer
    int minHeight;       // Minimum Y level
    int maxHeight;       // Maximum Y level
    int veinSize;        // Average vein size
    int veinsPerChunk;   // Average veins per chunk
};

// ============================================
// Ore Generator
// ============================================
class OreGenerator {
public:
    OreGenerator();
    ~OreGenerator();

    // Register ore types
    void RegisterOre(const OreType& ore);
    
    // Generate ores in a chunk
    void GenerateOresInChunk(World& world, int chunkX, int chunkY, int chunkZ);
    
    // Generate all ores in world (for existing terrain)
    void GenerateOresInWorld(World& world, int radius);

private:
    std::vector<OreType> ores;
    
    // Random number generation
    float RandomFloat();
    int RandomInt(int min, int max);
    
    // Generate a single ore vein
    void GenerateVein(World& world, int startX, int startY, int startZ, 
                      const OreType& ore);
};

} // namespace vge
