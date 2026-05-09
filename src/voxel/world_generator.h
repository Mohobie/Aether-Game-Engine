#pragma once
#include "chunk.h"
#include <random>

namespace vge {
    class WorldGenerator {
    public:
        WorldGenerator();
        void GenerateChunk(Chunk& chunk, int cx, int cy, int cz);
        void SetSeed(unsigned int seed);
        float GetNoise(float x, float y, float z);
        float GetHeightNoise(float x, float z);
        void GenerateTree(Chunk& chunk, int x, int y, int z);
    private:
        unsigned int seed = 12345;
        std::mt19937 rng;
    };
}