#pragma once

#include "block.h"
#include <array>

namespace VoxelEngine {
    constexpr int CHUNK_SIZE = 32;
    constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    
    struct Chunk {
        std::array<Block, CHUNK_VOLUME> blocks;
        bool dirty = true;
        bool loaded = false;
    };
}
