#pragma once
#include "chunk.h"
#include "block_types.h"

namespace vge {

enum class BiomeType {
    Plains,
    Forest,
    Desert,
    Snow
};

class Biome {
public:
    static BiomeType GetBiomeType(float temperature, float humidity);
    static float GetBaseHeight(BiomeType type);
    static float GetHeightVariation(BiomeType type);
    static BlockTypeID GetSurfaceBlock(BiomeType type);
    static BlockTypeID GetSubsurfaceBlock(BiomeType type);
    
    // Generate a column of blocks for a chunk
    static void GenerateChunkColumn(Chunk* chunk, int x, int z, float worldX, float worldZ, int seed);
};

} // namespace vge