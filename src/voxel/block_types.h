#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "math/vec3.h"

namespace vge {

// Generic block definition (replaces hardcoded BlockType enum)
struct BlockDef {
    std::string id;           // Unique string ID (e.g., "stone", "dirt")
    std::string name;         // Display name
    bool solid = true;
    bool opaque = true;
    float hardness = 1.0f;
    Vec3 color = Vec3(1.0f, 1.0f, 1.0f);
    int emission = 0;         // Light emission 0-15
    std::string texture;      // Texture path
    std::string model;        // Model path (optional, defaults to cube)
    
    bool IsSolid() const { return solid; }
    bool IsOpaque() const { return opaque; }
    float GetHardness() const { return hardness; }
    const std::string& GetName() const { return name; }
    Vec3 GetColor() const { return color; }
    int GetEmission() const { return emission; }
};

// Runtime block type (16-bit ID for storage)
using BlockTypeID = uint16_t;
constexpr BlockTypeID BLOCK_AIR = 0;

// Generic block instance (stored in chunks)
struct Block {
    BlockTypeID typeId;
    uint8_t metadata;
    uint8_t light_level;
    
    Block() : typeId(BLOCK_AIR), metadata(0), light_level(0) {}
    Block(BlockTypeID t) : typeId(t), metadata(0), light_level(0) {}
    
    bool IsSolid() const;
    bool IsOpaque() const;
    const std::string& GetName() const;
    Vec3 GetColor() const;
    int GetEmission() const;
    float GetHardness() const;
};

// Block registry - loads from JSON
class BlockRegistry {
private:
    std::vector<BlockDef> blocks;
    std::unordered_map<std::string, BlockTypeID> idToIndex;
    BlockTypeID nextId = 1;  // 0 is reserved for air
    
    BlockRegistry();
    
public:
    static BlockRegistry& GetInstance();
    
    // Load blocks from JSON file
    bool LoadFromFile(const std::string& path);
    bool LoadFromJson(const std::string& json);
    
    // Register a block definition
    BlockTypeID RegisterBlock(const BlockDef& def);
    
    // Get block by ID
    const BlockDef& GetBlock(BlockTypeID id) const;
    
    // Get block by string ID
    const BlockDef& GetBlock(const std::string& id) const;
    BlockTypeID GetBlockId(const std::string& id) const;
    
    // Check if block exists
    bool HasBlock(const std::string& id) const;
    bool HasBlock(BlockTypeID id) const;
    
    // Get total count
    size_t GetCount() const { return blocks.size(); }
    
    // Get all block IDs
    std::vector<std::string> GetAllBlockIds() const;
    
    // Save current registry to JSON
    bool SaveToFile(const std::string& path) const;
};

} // namespace vge