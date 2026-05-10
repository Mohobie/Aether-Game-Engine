#include "block_registry.h"
#include "block_types.h"
#include <cstring>

namespace vge {

OldBlockRegistry::OldBlockRegistry() : blockCount(0) {
    // Initialize with air
    blocks[0].typeId = BLOCK_AIR;
    std::strcpy(blocks[0].name, "Air");
    blocks[0].solid = false;
    blocks[0].opaque = false;
    blocks[0].hardness = 0.0f;
    blocks[0].color = Vec3(0, 0, 0);
    blocks[0].emission = 0;
    blockCount = 1;
}

OldBlockRegistry& OldBlockRegistry::GetInstance() {
    static OldBlockRegistry instance;
    return instance;
}

void OldBlockRegistry::RegisterBlock(BlockTypeID type, const char* name, bool solid, bool opaque, float hardness, Vec3 color) {
    if (type < 64) {
        blocks[type].typeId = type;
        std::strncpy(blocks[type].name, name, 31);
        blocks[type].name[31] = '\0';
        blocks[type].solid = solid;
        blocks[type].opaque = opaque;
        blocks[type].hardness = hardness;
        blocks[type].color = color;
        if (type >= blockCount) blockCount = type + 1;
    }
}

const BlockInfo& OldBlockRegistry::GetBlock(BlockTypeID type) const {
    if (type < blockCount) {
        return blocks[type];
    }
    return blocks[BLOCK_AIR];
}

const BlockInfo& OldBlockRegistry::GetBlockByName(const char* name) const {
    for (int i = 0; i < blockCount; i++) {
        if (std::strcmp(blocks[i].name, name) == 0) {
            return blocks[i];
        }
    }
    return blocks[BLOCK_AIR];
}

} // namespace vge
