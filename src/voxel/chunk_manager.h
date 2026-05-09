#pragma once
#include <unordered_map>
#include <memory>
#include "chunk.h"

namespace vge {
class ChunkManager {
    std::unordered_map<uint64_t, std::unique_ptr<Chunk>> chunks;
public:
    Chunk* GetChunk(int x, int y, int z);
    void UnloadDistantChunks(int centerX, int centerY, int centerZ, int radius);
};
}