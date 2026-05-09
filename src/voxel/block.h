#pragma once

#include "core/types.h"

namespace VoxelEngine {
    struct Block {
        BlockID id;
        uint8_t metadata;
        uint8_t light_level;
    };
}
