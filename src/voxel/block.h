#pragma once

#include "core/types.h"

namespace vge {
    struct Block {
        BlockID id;
        uint8_t metadata;
        uint8_t light_level;
    };
}
