#include "ore_generator.h"
#include "voxel/block_registry.h"
#include <cstdlib>
#include <cmath>

namespace vge {

OreGenerator::OreGenerator() {
    // Seed random
    std::srand(12345);
}

OreGenerator::~OreGenerator() {
}

void OreGenerator::RegisterOre(const OreType& ore) {
    ores.push_back(ore);
}

float OreGenerator::RandomFloat() {
    return static_cast<float>(std::rand()) / RAND_MAX;
}

int OreGenerator::RandomInt(int min, int max) {
    return min + std::rand() % (max - min + 1);
}

void OreGenerator::GenerateVein(World& world, int startX, int startY, int startZ, 
                                const OreType& ore) {
    // Simple vein generation - random walk from start point
    int size = RandomInt(ore.veinSize / 2, ore.veinSize);
    
    int x = startX;
    int y = startY;
    int z = startZ;
    
    for (int i = 0; i < size; ++i) {
        // Place ore block
        BlockTypeID currentBlock = world.GetBlock(x, y, z);
        if (currentBlock == BlockRegistry::GetInstance().GetBlockId("stone")) {
            world.SetBlock(x, y, z, ore.blockType);
        }
        
        // Random walk
        int dir = RandomInt(0, 5);
        switch (dir) {
            case 0: x++; break;
            case 1: x--; break;
            case 2: y++; break;
            case 3: y--; break;
            case 4: z++; break;
            case 5: z--; break;
        }
    }
}

void OreGenerator::GenerateOresInChunk(World& world, int chunkX, int chunkY, int chunkZ) {
    for (const auto& ore : ores) {
        // Check if this chunk is in the right height range
        int chunkWorldY = chunkY * CHUNK_SIZE;
        if (chunkWorldY + CHUNK_SIZE < ore.minHeight || chunkWorldY > ore.maxHeight) {
            continue;
        }
        
        // Generate veins
        int veinsToGenerate = RandomInt(0, ore.veinsPerChunk * 2);
        for (int v = 0; v < veinsToGenerate; ++v) {
            // Random position within chunk
            int x = chunkX * CHUNK_SIZE + RandomInt(0, CHUNK_SIZE - 1);
            int y = RandomInt(ore.minHeight, ore.maxHeight);
            int z = chunkZ * CHUNK_SIZE + RandomInt(0, CHUNK_SIZE - 1);
            
            GenerateVein(world, x, y, z, ore);
        }
    }
}

void OreGenerator::GenerateOresInWorld(World& world, int radius) {
    for (int cx = -radius; cx <= radius; ++cx) {
        for (int cy = -10; cy <= 5; ++cy) {
            for (int cz = -radius; cz <= radius; ++cz) {
                GenerateOresInChunk(world, cx, cy, cz);
            }
        }
    }
}

} // namespace vge
