#pragma once
#include "block.h"
#include "math/vec3.h"

namespace vge {

struct BlockInfo {
    BlockType type;
    char name[32];
    bool solid;
    bool opaque;
    float hardness;
    Vec3 color;
    int emission; // Light emission level 0-15
    
    bool IsSolid() const { return solid; }
    bool IsOpaque() const { return opaque; }
    float GetHardness() const { return hardness; }
    const char* GetName() const { return name; }
    Vec3 GetColor() const { return color; }
    int GetEmission() const { return emission; }
};

class BlockRegistry {
private:
    BlockInfo blocks[static_cast<int>(BlockType::Count)];
    
    BlockRegistry();
    
public:
    static BlockRegistry& GetInstance();
    
    void RegisterBlock(BlockType type, const char* name, bool solid, bool opaque, float hardness, Vec3 color);
    const BlockInfo& GetBlock(BlockType type) const;
    const BlockInfo& GetBlockByName(const char* name) const;
};

} // namespace vge
