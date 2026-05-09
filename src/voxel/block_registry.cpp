#include "block_registry.h"
#include <cstring>

namespace vge {

BlockRegistry::BlockRegistry() {
    // Initialize default blocks
    RegisterBlock(BlockType::Air, "Air", false, false, 0.0f, Vec3(0.0f, 0.0f, 0.0f));
    RegisterBlock(BlockType::Stone, "Stone", true, true, 2.0f, Vec3(0.5f, 0.5f, 0.5f));
    RegisterBlock(BlockType::Dirt, "Dirt", true, true, 1.0f, Vec3(0.55f, 0.27f, 0.07f));
    RegisterBlock(BlockType::Grass, "Grass", true, true, 1.0f, Vec3(0.2f, 0.8f, 0.2f));
    RegisterBlock(BlockType::Sand, "Sand", true, true, 1.0f, Vec3(0.9f, 0.85f, 0.6f));
    RegisterBlock(BlockType::Water, "Water", false, false, 0.0f, Vec3(0.2f, 0.4f, 0.9f));
    RegisterBlock(BlockType::Wood, "Wood", true, true, 2.0f, Vec3(0.6f, 0.4f, 0.2f));
    RegisterBlock(BlockType::Leaves, "Leaves", true, false, 0.5f, Vec3(0.2f, 0.6f, 0.1f));
    RegisterBlock(BlockType::Glass, "Glass", true, false, 0.5f, Vec3(0.8f, 0.9f, 1.0f));
    RegisterBlock(BlockType::Planks, "Planks", true, true, 1.5f, Vec3(0.8f, 0.6f, 0.3f));
    RegisterBlock(BlockType::CraftingTable, "Crafting Table", true, true, 1.5f, Vec3(0.7f, 0.5f, 0.2f));
    RegisterBlock(BlockType::Furnace, "Furnace", true, true, 3.5f, Vec3(0.4f, 0.4f, 0.4f));
    RegisterBlock(BlockType::Torch, "Torch", false, false, 0.0f, Vec3(1.0f, 0.9f, 0.5f));
    RegisterBlock(BlockType::Glowstone, "Glowstone", true, true, 1.5f, Vec3(1.0f, 1.0f, 0.6f));
    RegisterBlock(BlockType::Bedrock, "Bedrock", true, true, 999.0f, Vec3(0.2f, 0.2f, 0.2f));
    RegisterBlock(BlockType::CoalOre, "Coal Ore", true, true, 2.5f, Vec3(0.3f, 0.3f, 0.3f));
    RegisterBlock(BlockType::IronOre, "Iron Ore", true, true, 3.0f, Vec3(0.6f, 0.5f, 0.4f));
    RegisterBlock(BlockType::GoldOre, "Gold Ore", true, true, 3.0f, Vec3(0.9f, 0.8f, 0.2f));
    RegisterBlock(BlockType::DiamondOre, "Diamond Ore", true, true, 3.5f, Vec3(0.2f, 0.8f, 0.9f));
    RegisterBlock(BlockType::Snow, "Snow", true, true, 0.8f, Vec3(1.0f, 1.0f, 1.0f));
    RegisterBlock(BlockType::Ice, "Ice", true, false, 0.8f, Vec3(0.7f, 0.9f, 1.0f));
    RegisterBlock(BlockType::Cactus, "Cactus", true, false, 0.6f, Vec3(0.2f, 0.6f, 0.2f));
    RegisterBlock(BlockType::Flower, "Flower", false, false, 0.0f, Vec3(1.0f, 0.2f, 0.8f));
    RegisterBlock(BlockType::TallGrass, "Tall Grass", false, false, 0.0f, Vec3(0.3f, 0.7f, 0.2f));
    RegisterBlock(BlockType::Lava, "Lava", false, false, 0.0f, Vec3(1.0f, 0.3f, 0.0f));
    RegisterBlock(BlockType::Stick, "Stick", false, false, 0.0f, Vec3(0.6f, 0.4f, 0.2f));
}

BlockRegistry& BlockRegistry::GetInstance() {
    static BlockRegistry instance;
    return instance;
}

void BlockRegistry::RegisterBlock(BlockType type, const char* name, bool solid, bool opaque, float hardness, Vec3 color) {
    BlockInfo info;
    info.type = type;
    std::strncpy(info.name, name, sizeof(info.name) - 1);
    info.name[sizeof(info.name) - 1] = '\0';
    info.solid = solid;
    info.opaque = opaque;
    info.hardness = hardness;
    info.color = color;
    info.emission = 0;
    
    // Set emission for light sources
    if (type == BlockType::Torch) info.emission = 14;
    else if (type == BlockType::Glowstone) info.emission = 15;
    else if (type == BlockType::Lava) info.emission = 15;
    
    blocks[static_cast<int>(type)] = info;
}

const BlockInfo& BlockRegistry::GetBlock(BlockType type) const {
    return blocks[static_cast<int>(type)];
}

const BlockInfo& BlockRegistry::GetBlockByName(const char* name) const {
    for (int i = 0; i < static_cast<int>(BlockType::Count); ++i) {
        if (std::strcmp(blocks[i].name, name) == 0) {
            return blocks[i];
        }
    }
    return blocks[0]; // Return air as default
}

} // namespace vge