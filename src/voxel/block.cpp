#include "block.h"
#include "block_types.h"
#include "block_registry.h"

namespace vge {

bool Block::IsSolid() const {
    return BlockRegistry::GetInstance().GetBlock(typeId).IsSolid();
}

bool Block::IsOpaque() const {
    return BlockRegistry::GetInstance().GetBlock(typeId).IsOpaque();
}

const std::string& Block::GetName() const {
    return BlockRegistry::GetInstance().GetBlock(typeId).GetName();
}

} // namespace vge