#pragma once
#include "voxel/chunk.h"
#include "voxel/block_types.h"
#include "math/vec3.h"
#include <cstddef>
#include <map>
#include <memory>
#include <string>

namespace vge {

struct ChunkCoord {
    int x, y, z;
    
    bool operator<(const ChunkCoord& other) const {
        if (x != other.x) return x < other.x;
        if (y != other.y) return y < other.y;
        return z < other.z;
    }
};

class World {
private:
    std::map<ChunkCoord, std::unique_ptr<Chunk>> chunks;
    int seed;
    int viewDistance;
    std::size_t maxCachedChunks;
    int unloadDistance;
    
public:
    World();
    ~World();
    
    void Initialize();
    void Clear();
    
    Chunk* GetChunk(int x, int y, int z);
    const Chunk* GetChunk(int x, int y, int z) const;
    Chunk* GetOrCreateChunk(int x, int y, int z);
    
    BlockTypeID GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, BlockTypeID type);
    void SetBlock(int x, int y, int z, const std::string& blockId);

    // Documentation-facing compatibility helpers.
    void GenerateTerrain(int chunkX, int chunkZ, int radius = 0);
    bool HasChunk(int x, int z) const;
    bool HasChunk(int x, int y, int z) const;
    void LoadChunkAsync(int x, int z);
    void Update(const Vec3& playerPosition);
    
    void SetSeed(int s) { seed = s; }
    int GetSeed() const { return seed; }

    void SetViewDistance(int distance) { viewDistance = distance > 0 ? distance : 0; }
    int GetViewDistance() const { return viewDistance; }
    void SetRenderDistance(int distance) { SetViewDistance(distance); }
    int GetRenderDistance() const { return GetViewDistance(); }
    void SetMaxCachedChunks(std::size_t count) { maxCachedChunks = count > 0 ? count : 1; }
    std::size_t GetMaxCachedChunks() const { return maxCachedChunks; }
    void SetUnloadDistance(int distance) { unloadDistance = distance > 0 ? distance : 0; }
    int GetUnloadDistance() const { return unloadDistance; }

    // Iterator access for systems that need to iterate all chunks
    const std::map<ChunkCoord, std::unique_ptr<Chunk>>& GetChunks() const { return chunks; }
    std::map<ChunkCoord, std::unique_ptr<Chunk>>& GetChunks() { return chunks; }
};

} // namespace vge
