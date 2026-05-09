#pragma once

#include "block.h"
#include <array>

namespace VoxelEngine {
    constexpr int CHUNK_SIZE = 32;
    constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    
    struct Chunk {
        std::array<Block, CHUNK_VOLUME> blocks;
        std::array<uint8_t, CHUNK_VOLUME> lightLevels; // 0-15
        bool dirty = true;
        bool loaded = false;
        
        Chunk() : lightLevels{} {} // Initialize light to 0
        
        int GetLightLevel(int x, int y, int z) const {
            if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return 0;
            return lightLevels[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE];
        }
        
        void SetLightLevel(int x, int y, int z, int level) {
            if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return;
            lightLevels[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] = (uint8_t)level;
        }
    };
}
