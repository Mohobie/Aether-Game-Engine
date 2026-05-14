#pragma once
#include <unordered_map>
#include <memory>
#include "math/vec3.h"

namespace vge {

class Chunk;
class World;
class WorldGenerator;

class ChunkManager {
private:
    std::unordered_map<uint64_t, Chunk*> loadedChunks;
    int viewDistance;
    World* world;
    WorldGenerator* generator;
    
public:
    ChunkManager();
    ~ChunkManager();
    
    void Initialize(World* world, WorldGenerator* generator);
    void Update(const Vec3& playerPosition);
    
    Chunk* LoadChunk(int x, int y, int z);
    void UnloadChunk(int x, int y, int z);
    void UnloadDistantChunks(int centerX, int centerY, int centerZ, int radius);
    void Clear();
    
    int GetViewDistance() const { return viewDistance; }
    void SetViewDistance(int distance) { viewDistance = distance; }
};

} // namespace vge
