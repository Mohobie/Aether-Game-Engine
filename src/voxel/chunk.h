#pragma once

#include "block.h"
#include "math/vec3.h"
#include <array>

namespace vge {
    constexpr int CHUNK_SIZE = 32;
    constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    
    struct Chunk {
        std::array<BlockType, CHUNK_VOLUME> blocks;
        std::array<uint8_t, CHUNK_VOLUME> lightLevels; // 0-15
        bool dirty;
        bool loaded;
        int chunkX, chunkY, chunkZ;
        bool modified;
        
        Chunk();
        Chunk(int x, int y, int z);
        ~Chunk();
        
        int GetChunkX() const { return chunkX; }
        int GetChunkY() const { return chunkY; }
        int GetChunkZ() const { return chunkZ; }
        
        BlockType GetBlock(int x, int y, int z) const;
        void SetBlock(int x, int y, int z, BlockType type);
        
        int GetLightLevel(int x, int y, int z) const;
        void SetLightLevel(int x, int y, int z, int level);
        
        bool IsEmpty() const;
        bool IsDirty() const { return dirty; }
        void SetDirty(bool value) { dirty = value; }
        
        Vec3 GetWorldPosition() const;
    };
}