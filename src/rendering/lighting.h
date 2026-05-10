#pragma once
#include "voxel/block_types.h"

namespace vge {

class Chunk;

class LightEngine {
private:
    int skyLight;
    
    void PropagateBlockLight(Chunk* chunk);
    void PropagateLightFromSource(Chunk* chunk, int sourceX, int sourceY, int sourceZ, int intensity);
    int GetBlockLightEmission(BlockTypeID type) const;
    
public:
    LightEngine();
    
    void InitializeChunk(Chunk* chunk);
    void UpdateBlockLight(Chunk* chunk, int x, int y, int z);
    void RecalculateChunk(Chunk* chunk);
    
    int GetSkyLight() const { return skyLight; }
    void SetSkyLight(int level) { skyLight = level; }
};

} // namespace vge
