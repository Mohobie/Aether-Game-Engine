#include "rendering/lighting.h"
#include "voxel/chunk.h"
#include "voxel/world.h"
#include "voxel/block.h"
#include "voxel/block_registry.h"
#include <algorithm>

namespace vge {

LightEngine::LightEngine() : skyLight(15) {}

void LightEngine::InitializeChunk(Chunk* chunk) {
    if (!chunk) return;
    
    // Initialize all blocks with sky light
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            // Find highest solid block
            int highestSolid = -1;
            for (int y = CHUNK_SIZE - 1; y >= 0; --y) {
                BlockTypeID block = chunk->GetBlock(x, y, z);
                if (block != BlockRegistry::GetInstance().GetBlockId("air") && BlockRegistry::GetInstance().GetBlock(block).IsSolid()) {
                    highestSolid = y;
                    break;
                }
            }
            
            // Set light levels
            for (int y = CHUNK_SIZE - 1; y >= 0; --y) {
                if (y > highestSolid) {
                    // Above ground - full sky light
                    chunk->SetLightLevel(x, y, z, 15);
                } else {
                    // Below ground - no sky light
                    chunk->SetLightLevel(x, y, z, 0);
                }
            }
        }
    }
    
    // Propagate block light from light-emitting blocks
    PropagateBlockLight(chunk);
}

void LightEngine::PropagateBlockLight(Chunk* chunk) {
    if (!chunk) return;
    
    // Find all light-emitting blocks
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                BlockTypeID block = chunk->GetBlock(x, y, z);
                int emission = GetBlockLightEmission(block);
                
                if (emission > 0) {
                    chunk->SetLightLevel(x, y, z, emission);
                    
                    // Propagate to neighbors
                    PropagateLightFromSource(chunk, x, y, z, emission);
                }
            }
        }
    }
}

void LightEngine::PropagateLightFromSource(Chunk* chunk, int sourceX, int sourceY, int sourceZ, int intensity) {
    // Simple flood fill for light propagation
    int range = intensity - 1;
    
    for (int dx = -range; dx <= range; ++dx) {
        for (int dy = -range; dy <= range; ++dy) {
            for (int dz = -range; dz <= range; ++dz) {
                int x = sourceX + dx;
                int y = sourceY + dy;
                int z = sourceZ + dz;
                
                if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
                    continue;
                }
                
                int distance = std::abs(dx) + std::abs(dy) + std::abs(dz);
                int lightLevel = intensity - distance;
                
                if (lightLevel > 0) {
                    int currentLevel = chunk->GetLightLevel(x, y, z);
                    if (lightLevel > currentLevel) {
                        chunk->SetLightLevel(x, y, z, lightLevel);
                    }
                }
            }
        }
    }
}

int LightEngine::GetBlockLightEmission(BlockTypeID type) const {
    if (type == BlockRegistry::GetInstance().GetBlockId("glowstone")) return 15;
    if (type == BlockRegistry::GetInstance().GetBlockId("torch")) return 14;
    if (type == BlockRegistry::GetInstance().GetBlockId("lava")) return 15;
    return 0;
}

void LightEngine::UpdateBlockLight(Chunk* chunk, int x, int y, int z) {
    if (!chunk) return;
    
    BlockTypeID block = chunk->GetBlock(x, y, z);
    
    if (block == BlockRegistry::GetInstance().GetBlockId("air")) {
        // Block removed - might need to recalculate
        RecalculateChunk(chunk);
    } else {
        // Block placed - update light
        int emission = GetBlockLightEmission(block);
        if (emission > 0) {
            chunk->SetLightLevel(x, y, z, emission);
            PropagateLightFromSource(chunk, x, y, z, emission);
        }
    }
}

void LightEngine::RecalculateChunk(Chunk* chunk) {
    if (!chunk) return;
    
    // Clear all light levels
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                chunk->SetLightLevel(x, y, z, 0);
            }
        }
    }
    
    // Re-initialize
    InitializeChunk(chunk);
}

} // namespace vge