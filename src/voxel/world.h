#pragma once
#include "voxel/chunk.h"
#include "voxel/block_types.h"
#include <map>
#include <memory>

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
    
public:
    World();
    ~World();
    
    void Clear();
    
    Chunk* GetChunk(int x, int y, int z);
    Chunk* GetOrCreateChunk(int x, int y, int z);
    
    BlockTypeID GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, BlockTypeID type);
    void SetBlock(int x, int y, int z, const std::string& blockId);
    
    int GetSeed() const { return seed; }
    void SetSeed(int s) { seed = s; }
};

} // namespace vge