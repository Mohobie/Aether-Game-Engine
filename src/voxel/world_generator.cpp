#include "world_generator.h"
#include <cmath>
#include <random>

namespace VoxelEngine {
    void WorldGenerator::SetSeed(unsigned int s) {
        seed = s;
    }
    
    float WorldGenerator::GetNoise(float x, float y, float z) {
        // Simple noise function
        return std::sin(x * 0.1f) * std::cos(y * 0.1f) * std::sin(z * 0.1f);
    }
    
    void WorldGenerator::GenerateChunk(Chunk& chunk, ChunkCoord x, ChunkCoord y, ChunkCoord z) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                for (int k = 0; k < CHUNK_SIZE; k++) {
                    float noise = GetNoise(x * CHUNK_SIZE + i, y * CHUNK_SIZE + j, z * CHUNK_SIZE + k);
                    int index = i * CHUNK_SIZE * CHUNK_SIZE + j * CHUNK_SIZE + k;
                    if (noise > 0.0f) {
                        chunk.blocks[index].id = 1; // Stone
                    } else {
                        chunk.blocks[index].id = 0; // Air
                    }
                }
            }
        }
        chunk.loaded = true;
        chunk.dirty = true;
    }
}
