#pragma once

#include "core/types.h"
#include <string>

namespace VoxelEngine {
    struct BlockProperties {
        std::string name;
        bool solid;
        bool transparent;
        uint8_t light_emission;
    };
    
    class BlockRegistry {
    public:
        static BlockRegistry& Instance();
        void Register(BlockID id, const BlockProperties& props);
        const BlockProperties& Get(BlockID id);
    };
}
