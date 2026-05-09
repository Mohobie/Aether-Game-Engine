#include "block_registry.h"
#include <unordered_map>

namespace VoxelEngine {
    static std::unordered_map<BlockID, BlockProperties> s_registry;
    
    BlockRegistry& BlockRegistry::Instance() {
        static BlockRegistry instance;
        return instance;
    }
    
    void BlockRegistry::Register(BlockID id, const BlockProperties& props) {
        s_registry[id] = props;
    }
    
    const BlockProperties& BlockRegistry::Get(BlockID id) {
        static BlockProperties defaultProps{"Unknown", true, false, 0};
        auto it = s_registry.find(id);
        if (it != s_registry.end()) {
            return it->second;
        }
        return defaultProps;
    }
}
