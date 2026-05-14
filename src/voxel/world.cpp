#include "world.h"
#include "chunk.h"
#include "block_types.h"
#include "block_registry.h"
#include "world_generator.h"
#include <cmath>

namespace vge {

namespace {

int ToChunkCoord(int value) {
    int coord = value / CHUNK_SIZE;
    if (value < 0 && (value % CHUNK_SIZE) != 0) {
        --coord;
    }
    return coord;
}

} // namespace

World::World()
    : seed(0)
    , viewDistance(8)
    , maxCachedChunks(1024)
    , unloadDistance(32) {}

World::~World() {
    Clear();
}

void World::Initialize() {
    // The world is ready after construction. This method exists so the
    // implementation matches the documented public API.
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

const Chunk* World::GetChunk(int x, int y, int z) const {
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
        chunk->loaded = true;  // Mark as loaded when created
        chunks[{x, y, z}] = std::move(newChunk);
    }
    return chunk;
}

BlockTypeID World::GetBlock(int x, int y, int z) const {
    int chunkX = ToChunkCoord(x);
    int chunkY = ToChunkCoord(y);
    int chunkZ = ToChunkCoord(z);
    
    ChunkCoord coord = {chunkX, chunkY, chunkZ};
    auto it = chunks.find(coord);
    if (it != chunks.end()) {
        int localX = x - chunkX * CHUNK_SIZE;
        int localY = y - chunkY * CHUNK_SIZE;
        int localZ = z - chunkZ * CHUNK_SIZE;
        return it->second->GetBlock(localX, localY, localZ);
    }
    return BLOCK_AIR;
}

void World::SetBlock(int x, int y, int z, BlockTypeID type) {
    int chunkX = ToChunkCoord(x);
    int chunkY = ToChunkCoord(y);
    int chunkZ = ToChunkCoord(z);
    
    Chunk* chunk = GetOrCreateChunk(chunkX, chunkY, chunkZ);
    int localX = x - chunkX * CHUNK_SIZE;
    int localY = y - chunkY * CHUNK_SIZE;
    int localZ = z - chunkZ * CHUNK_SIZE;
    chunk->SetBlock(localX, localY, localZ, type);
    chunk->loaded = true;  // Mark chunk as loaded when blocks are added
}

void World::SetBlock(int x, int y, int z, const std::string& blockId) {
    BlockTypeID typeId = BlockRegistry::GetInstance().GetBlockId(blockId);
    SetBlock(x, y, z, typeId);
}

void World::GenerateTerrain(int chunkX, int chunkZ, int radius) {
    WorldGenerator generator;
    generator.SetSeed(seed);

    for (int cx = chunkX - radius; cx <= chunkX + radius; ++cx) {
        for (int cz = chunkZ - radius; cz <= chunkZ + radius; ++cz) {
            for (int cy = -1; cy <= 1; ++cy) {
                Chunk* chunk = GetChunk(cx, cy, cz);
                if (!chunk) {
                    chunk = GetOrCreateChunk(cx, cy, cz);
                    generator.GenerateChunk(*chunk, cx, cy, cz);
                }
            }
        }
    }
}

bool World::HasChunk(int x, int z) const {
    for (const auto& [coord, chunk] : chunks) {
        if (coord.x == x && coord.z == z && chunk) {
            return true;
        }
    }
    return false;
}

bool World::HasChunk(int x, int y, int z) const {
    return GetChunk(x, y, z) != nullptr;
}

void World::LoadChunkAsync(int x, int z) {
    // The documented API exposes async loading. The current implementation
    // performs the work immediately so callers can rely on the entry point
    // without introducing unsynchronized background writes to World.
    GenerateTerrain(x, z);
}

void World::Update(const Vec3& playerPosition) {
    const int playerChunkX = ToChunkCoord(static_cast<int>(std::floor(playerPosition.x)));
    const int playerChunkZ = ToChunkCoord(static_cast<int>(std::floor(playerPosition.z)));

    for (int dx = -viewDistance; dx <= viewDistance; ++dx) {
        for (int dz = -viewDistance; dz <= viewDistance; ++dz) {
            const int cx = playerChunkX + dx;
            const int cz = playerChunkZ + dz;
            if (!HasChunk(cx, cz)) {
                GenerateTerrain(cx, cz);
            }
        }
    }

    const int effectiveUnloadDistance = unloadDistance > viewDistance ? unloadDistance : (viewDistance + 2);
    for (auto it = chunks.begin(); it != chunks.end();) {
        const int dx = it->first.x - playerChunkX;
        const int dz = it->first.z - playerChunkZ;
        const float distance = std::sqrt(static_cast<float>(dx * dx + dz * dz));

        if (!it->second->modified &&
            (distance > effectiveUnloadDistance || chunks.size() > maxCachedChunks)) {
            it = chunks.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace vge
