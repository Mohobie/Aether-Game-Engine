#pragma once
#include "chunk.h"
#include <vector>
#include <memory>

namespace VoxelEngine {
    class ChunkManager {
    public:
        void LoadChunk(ChunkCoord x, ChunkCoord y, ChunkCoord z);
        void UnloadChunk(ChunkCoord x, ChunkCoord y, ChunkCoord z);
        void Update(float deltaTime);
        size_t GetLoadedChunkCount() const;
    private:
        std::vector<std::unique_ptr<Chunk>> loadedChunks;
    };
}
