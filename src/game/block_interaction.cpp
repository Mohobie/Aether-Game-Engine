#include "game/block_interaction.h"
#include "voxel/block_registry.h"
#include "core/logger.h"
#include <iostream>

namespace vge {

BlockInteraction::BlockInteraction()
    : selectedSlot(0), placeCooldown(0.0f), removeCooldown(0.0f) {
    raycast.maxDistance = REACH_DISTANCE;
}

void BlockInteraction::Initialize(World& world) {
    (void)world;
    // Initialize hotbar with common building blocks
    hotbarBlocks = {
        "stone",    // 1
        "dirt",     // 2
        "grass",    // 3
        "wood",     // 4
        "planks",   // 5
        "glass",    // 6
        "sand",     // 7
        "snow",     // 8
        "glowstone" // 9
    };
    selectedSlot = 0;
}

BlockTypeID BlockInteraction::GetSelectedBlockType() const {
    if (selectedSlot < 0 || selectedSlot >= static_cast<int>(hotbarBlocks.size())) {
        return BLOCK_AIR;
    }
    return BlockRegistry::GetInstance().GetBlockId(hotbarBlocks[selectedSlot]);
}

const std::string& BlockInteraction::GetSelectedBlockName() const {
    static const std::string empty = "air";
    if (selectedSlot < 0 || selectedSlot >= static_cast<int>(hotbarBlocks.size())) {
        return empty;
    }
    return hotbarBlocks[selectedSlot];
}

void BlockInteraction::SelectSlot(int slot) {
    if (slot >= 0 && slot < HOTBAR_SIZE && slot < static_cast<int>(hotbarBlocks.size())) {
        selectedSlot = slot;
        std::cout << "[BlockInteraction] Selected: " << hotbarBlocks[slot] << " (slot " << (slot + 1) << ")" << std::endl;
    }
}

void BlockInteraction::NextSlot() {
    SelectSlot((selectedSlot + 1) % HOTBAR_SIZE);
}

void BlockInteraction::PrevSlot() {
    SelectSlot((selectedSlot - 1 + HOTBAR_SIZE) % HOTBAR_SIZE);
}

void BlockInteraction::PerformRaycast(const Camera& camera, World& world) {
    Vec3 origin = camera.GetPosition();
    Vec3 direction = camera.GetForward();
    currentHit = raycast.Cast(origin, direction, world);
}

void BlockInteraction::HandleInput(Input& input, World& world, float dt) {
    // Update cooldowns
    if (placeCooldown > 0.0f) placeCooldown -= dt;
    if (removeCooldown > 0.0f) removeCooldown -= dt;

    // Number keys 1-9 for hotbar selection
    if (input.IsKeyJustPressed(KeyCode::Key1)) SelectSlot(0);
    if (input.IsKeyJustPressed(KeyCode::Key2)) SelectSlot(1);
    if (input.IsKeyJustPressed(KeyCode::Key3)) SelectSlot(2);
    if (input.IsKeyJustPressed(KeyCode::Key4)) SelectSlot(3);
    if (input.IsKeyJustPressed(KeyCode::Key5)) SelectSlot(4);
    if (input.IsKeyJustPressed(KeyCode::Key6)) SelectSlot(5);
    if (input.IsKeyJustPressed(KeyCode::Key7)) SelectSlot(6);
    if (input.IsKeyJustPressed(KeyCode::Key8)) SelectSlot(7);
    if (input.IsKeyJustPressed(KeyCode::Key9)) SelectSlot(8);

    // Place block (E key = right click equivalent)
    if (input.IsKeyJustPressed(KeyCode::E) && placeCooldown <= 0.0f) {
        PlaceBlock(world);
        placeCooldown = COOLDOWN_TIME;
    }

    // Remove block (Q key = left click equivalent)
    if (input.IsKeyJustPressed(KeyCode::Q) && removeCooldown <= 0.0f) {
        RemoveBlock(world);
        removeCooldown = COOLDOWN_TIME;
    }
}

void BlockInteraction::PlaceBlock(World& world) {
    if (!currentHit.hit) {
        std::cout << "[BlockInteraction] No target to place against" << std::endl;
        return;
    }

    BlockTypeID type = GetSelectedBlockType();
    if (type == BLOCK_AIR) {
        std::cout << "[BlockInteraction] Cannot place air" << std::endl;
        return;
    }

    // Calculate placement position (adjacent to hit face)
    Vec3 placePos = currentHit.blockPosition + currentHit.normal;

    // Prevent placing inside player (simple check: distance from camera)
    // Camera position is roughly at player eye level
    // We need access to camera position - store it or pass it
    // For now, skip the check or do a rough one if we had camera pos stored

    world.SetBlock(
        static_cast<int>(placePos.x),
        static_cast<int>(placePos.y),
        static_cast<int>(placePos.z),
        type
    );

    std::cout << "[BlockInteraction] Placed " << GetSelectedBlockName()
              << " at (" << (int)placePos.x << ", " << (int)placePos.y << ", " << (int)placePos.z << ")"
              << std::endl;
}

void BlockInteraction::RemoveBlock(World& world) {
    if (!currentHit.hit) {
        std::cout << "[BlockInteraction] No block to remove" << std::endl;
        return;
    }

    // Don't remove bedrock
    if (currentHit.blockType == BlockRegistry::GetInstance().GetBlockId("bedrock")) {
        std::cout << "[BlockInteraction] Cannot remove bedrock" << std::endl;
        return;
    }

    world.SetBlock(
        static_cast<int>(currentHit.blockPosition.x),
        static_cast<int>(currentHit.blockPosition.y),
        static_cast<int>(currentHit.blockPosition.z),
        BLOCK_AIR
    );

    std::cout << "[BlockInteraction] Removed block at ("
              << (int)currentHit.blockPosition.x << ", "
              << (int)currentHit.blockPosition.y << ", "
              << (int)currentHit.blockPosition.z << ")"
              << std::endl;
}

void BlockInteraction::Update(const Camera& camera, Input& input, World& world) {
    PerformRaycast(camera, world);
    HandleInput(input, world, 0.016f); // Assume ~60fps for dt
}

} // namespace vge
