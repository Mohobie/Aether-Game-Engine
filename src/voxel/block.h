#pragma once
#include "core/types.h"
#include <cstdint>

namespace vge {

enum class BlockType : uint16_t {
    Air = 0,
    Stone = 1,
    Dirt = 2,
    Grass = 3,
    Sand = 4,
    Water = 5,
    Wood = 6,
    Leaves = 7,
    Glass = 8,
    Planks = 9,
    CraftingTable = 10,
    Furnace = 11,
    Torch = 12,
    Glowstone = 13,
    Bedrock = 14,
    CoalOre = 15,
    IronOre = 16,
    GoldOre = 17,
    DiamondOre = 18,
    Snow = 19,
    Ice = 20,
    Cactus = 21,
    Flower = 22,
    TallGrass = 23,
    Lava = 24,
    Stick = 25,
    Count = 26
};

struct Block {
    BlockType type;
    uint8_t metadata;
    uint8_t light_level;
    
    Block() : type(BlockType::Air), metadata(0), light_level(0) {}
    Block(BlockType t) : type(t), metadata(0), light_level(0) {}
    
    bool IsSolid() const;
    bool IsOpaque() const;
    const char* GetName() const;
};

} // namespace vge