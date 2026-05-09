#pragma once
#include "chunk.h"

namespace vge {
    class WorldGenerator {
    public:
        void GenerateChunk(Chunk& chunk, int x, int y, int z);
        void SetSeed(unsigned int seed);
        float GetNoise(float x, float y, float z);
    private:
        unsigned int seed = 12345;
    };
}