#pragma once

#include "chunk.h"
#include <unordered_map>

namespace vge {
    class World {
    public:
        Chunk* GetChunk(ChunkCoord x, ChunkCoord y, ChunkCoord z);
        Block GetBlock(int x, int y, int z);
        void SetBlock(int x, int y, int z, Block block);
        void Update(float deltaTime);
    private:
        std::unordered_map<uint64_t, Chunk> chunks;
    };
}
