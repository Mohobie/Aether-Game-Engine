#pragma once

#include "block.h"
#include <array>

namespace vge {
    constexpr int CHUNK_SIZE = 32;
    constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    
    struct Chunk {
        std::array<Block, CHUNK_VOLUME> blocks;
        std::array<uint8_t, CHUNK_VOLUME> lightLevels; // 0-15
        bool dirty = true;
        bool loaded = false;
        int chunkX, chunkY, chunkZ;
        
        Chunk() : lightLevels{}, chunkX(0), chunkY(0), chunkZ(0) {}
        Chunk(int x, int y, int z) : lightLevels{}, chunkX(x), chunkY(y), chunkZ(z) {}
        
        int GetChunkX() const { return chunkX; }
        int GetChunkY() const { return chunkY; }
        int GetChunkZ() const { return chunkZ; }
        
        BlockType GetBlock(int x, int y, int z) const {
            if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) 
                return BlockType::Air;
            return blocks[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE].type;
        }
        
        void SetBlock(int x, int y, int z, BlockType type) {
            if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) 
                return;
            blocks[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE].type = type;
            dirty = true;
        }
        
        int GetLightLevel(int x, int y, int z) const {
            if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return 0;
            return lightLevels[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE];
        }
        
        void SetLightLevel(int x, int y, int z, int level) {
            if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return;
            lightLevels[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] = (uint8_t)level;
        }
        
        bool IsEmpty() const {
            for (const auto& block : blocks) {
                if (block.type != BlockType::Air) return false;
            }
            return true;
        }
        
        bool IsDirty() const { return dirty; }
        void SetDirty(bool value) { dirty = value; }
    };
}
