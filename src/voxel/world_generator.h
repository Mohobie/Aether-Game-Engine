#pragma once
#include "chunk.h"

namespace VoxelEngine {
    class WorldGenerator {
    public:
        void GenerateChunk(Chunk& chunk, ChunkCoord x, ChunkCoord y, ChunkCoord z);
        void SetSeed(unsigned int seed);
        float GetNoise(float x, float y, float z);
    private:
        unsigned int seed = 12345;
    };
}
