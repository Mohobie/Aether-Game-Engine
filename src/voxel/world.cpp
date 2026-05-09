#include "world.h"
#include "chunk.h"

namespace vge {

World::World() : seed(0) {}

World::~World() {
    Clear();
}

void World::Clear() {
    chunks.clear();
}

Chunk* World::GetChunk(int x, int y, int z) {
    ChunkCoord coord = {x, y, z};
    auto it = chunks.find(coord);
    if (it != chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

Chunk* World::GetOrCreateChunk(int x, int y, int z) {
    Chunk* chunk = GetChunk(x, y, z);
    if (!chunk) {
        auto newChunk = std::make_unique<Chunk>(x, y, z);
        chunk = newChunk.get();
        chunks[{x, y, z}] = std::move(newChunk);
    }
    return chunk;
}

BlockType World::GetBlock(int x, int y, int z) const {
    int chunkX = x / Chunk::CHUNK_SIZE;
    int chunkY = y / Chunk::CHUNK_SIZE;
    int chunkZ = z / Chunk::CHUNK_SIZE;
    
    if (x < 0) chunkX--;
    if (y < 0) chunkY--;
    if (z < 0) chunkZ--;
    
    ChunkCoord coord = {chunkX, chunkY, chunkZ};
    auto it = chunks.find(coord);
    if (it != chunks.end()) {
        int localX = x - chunkX * Chunk::CHUNK_SIZE;
        int localY = y - chunkY * Chunk::CHUNK_SIZE;
        int localZ = z - chunkZ * Chunk::CHUNK_SIZE;
        return it->second->GetBlock(localX, localY, localZ);
    }
    return BlockType::Air;
}

void World::SetBlock(int x, int y, int z, BlockType type) {
    int chunkX = x / Chunk::CHUNK_SIZE;
    int chunkY = y / Chunk::CHUNK_SIZE;
    int chunkZ = z / Chunk::CHUNK_SIZE;
    
    if (x < 0) chunkX--;
    if (y < 0) chunkY--;
    if (z < 0) chunkZ--;
    
    Chunk* chunk = GetOrCreateChunk(chunkX, chunkY, chunkZ);
    int localX = x - chunkX * Chunk::CHUNK_SIZE;
    int localY = y - chunkY * Chunk::CHUNK_SIZE;
    int localZ = z - chunkZ * Chunk::CHUNK_SIZE;
    chunk->SetBlock(localX, localY, localZ, type);
}

} // namespace vge
