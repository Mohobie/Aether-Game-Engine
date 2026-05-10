#pragma once
#include "block_types.h"
#include "math/vec3.h"

namespace vge {

// Legacy BlockInfo for backward compatibility - wraps BlockDef
struct BlockInfo {
    BlockTypeID typeId;
    char name[32];
    bool solid;
    bool opaque;
    float hardness;
    Vec3 color;
    int emission;
    
    BlockInfo() : typeId(BLOCK_AIR), solid(true), opaque(true), hardness(1.0f), emission(0) {
        name[0] = '\0';
        color = Vec3(1.0f, 1.0f, 1.0f);
    }
    
    bool IsSolid() const { return solid; }
    bool IsOpaque() const { return opaque; }
    float GetHardness() const { return hardness; }
    const char* GetName() const { return name; }
    Vec3 GetColor() const { return color; }
    int GetEmission() const { return emission; }
};

// Legacy BlockRegistry - wraps new registry
class OldBlockRegistry {
private:
    BlockInfo blocks[64]; // Fixed size for legacy
    int blockCount;
    
    OldBlockRegistry();
    
public:
    static OldBlockRegistry& GetInstance();
    
    void RegisterBlock(BlockTypeID type, const char* name, bool solid, bool opaque, float hardness, Vec3 color);
    const BlockInfo& GetBlock(BlockTypeID type) const;
    const BlockInfo& GetBlockByName(const char* name) const;
};

} // namespace vge