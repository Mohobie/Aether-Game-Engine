#include "block.h"

namespace vge {

BlockProperties Block::GetProperties(BlockType type) {
    switch (type) {
        case BlockType::Air:
            return {false, false, Vec3(0.0f, 0.0f, 0.0f), 0.0f};
        case BlockType::Grass:
            return {true, true, Vec3(0.2f, 0.8f, 0.2f), 1.0f};
        case BlockType::Dirt:
            return {true, true, Vec3(0.6f, 0.4f, 0.2f), 1.0f};
        case BlockType::Stone:
            return {true, true, Vec3(0.5f, 0.5f, 0.5f), 2.0f};
        case BlockType::Wood:
            return {true, true, Vec3(0.4f, 0.25f, 0.1f), 1.5f};
        case BlockType::Leaves:
            return {true, false, Vec3(0.1f, 0.6f, 0.1f), 0.3f};
        case BlockType::Sand:
            return {true, true, Vec3(0.9f, 0.85f, 0.6f), 0.8f};
        case BlockType::Water:
            return {false, false, Vec3(0.2f, 0.4f, 0.9f), 0.0f};
        case BlockType::Bedrock:
            return {true, true, Vec3(0.2f, 0.2f, 0.2f), 999.0f};
        default:
            return {true, true, Vec3(1.0f, 0.0f, 1.0f), 1.0f};
    }
}

std::string Block::GetName(BlockType type) {
    switch (type) {
        case BlockType::Air: return "Air";
        case BlockType::Grass: return "Grass";
        case BlockType::Dirt: return "Dirt";
        case BlockType::Stone: return "Stone";
        case BlockType::Wood: return "Wood";
        case BlockType::Leaves: return "Leaves";
        case BlockType::Sand: return "Sand";
        case BlockType::Water: return "Water";
        case BlockType::Bedrock: return "Bedrock";
        default: return "Unknown";
    }
}

} // namespace vge
