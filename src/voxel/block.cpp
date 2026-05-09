#include "block.h"
#include "block_registry.h"

namespace vge {

bool Block::IsSolid() const {
    return BlockRegistry::GetInstance().GetBlock(type).IsSolid();
}

bool Block::IsOpaque() const {
    return BlockRegistry::GetInstance().GetBlock(type).IsOpaque();
}

const char* Block::GetName() const {
    return BlockRegistry::GetInstance().GetBlock(type).GetName();
}

} // namespace vge