#include "chunk_manager.h"

namespace vge {
static uint64_t HashChunkCoord(int x, int y, int z) {
    return ((uint64_t)(x + 32768) << 32) | ((uint64_t)(y + 32768) << 16) | (uint64_t)(z + 32768);
}

Chunk* ChunkManager::GetChunk(int x, int y, int z) {
    uint64_t hash = HashChunkCoord(x, y, z);
    auto it = chunks.find(hash);
    if (it != chunks.end()) return it->second.get();
    
    auto chunk = std::make_unique<Chunk>(x, y, z);
    Chunk* ptr = chunk.get();
    chunks[hash] = std::move(chunk);
    return ptr;
}

void ChunkManager::UnloadDistantChunks(int cx, int cy, int cz, int radius) {
    for (auto it = chunks.begin(); it != chunks.end(); ) {
        Chunk* chunk = it->second.get();
        Vec3 pos = chunk->GetWorldPosition();
        int dx = (int)(pos.x / Chunk::CHUNK_SIZE) - cx;
        int dy = (int)(pos.y / Chunk::CHUNK_SIZE) - cy;
        int dz = (int)(pos.z / Chunk::CHUNK_SIZE) - cz;
        
        if (std::abs(dx) > radius || std::abs(dy) > radius || std::abs(dz) > radius) {
            it = chunks.erase(it);
        } else {
            ++it;
        }
    }
}
}