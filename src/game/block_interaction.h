#pragma once
#include "core/raycast.h"
#include "voxel/world.h"
#include "rendering/camera.h"
#include "platform/input_manager.h"
#include <string>
#include <vector>

namespace vge {

// Block interaction system: raycast from camera, place/remove blocks
class BlockInteraction {
public:
    static constexpr float REACH_DISTANCE = 15.0f;
    static constexpr int HOTBAR_SIZE = 9;

    BlockInteraction();

    // Initialize with block registry IDs for hotbar slots
    void Initialize(World& world);

    // Update per frame — handles input and raycasting
    void Update(const Camera& camera, Input& input, World& world);

    // Get currently targeted block info (for renderer highlight)
    bool HasTarget() const { return currentHit.hit; }
    const RaycastHit& GetTarget() const { return currentHit; }

    // Get selected block type for placement
    BlockTypeID GetSelectedBlockType() const;
    const std::string& GetSelectedBlockName() const;
    int GetSelectedSlot() const { return selectedSlot; }

    // Hotbar selection
    void SelectSlot(int slot); // 0-8
    void NextSlot();
    void PrevSlot();

private:
    Raycast raycast;
    RaycastHit currentHit;

    // Hotbar: 9 slots with block IDs
    std::vector<std::string> hotbarBlocks;
    int selectedSlot;

    // Input cooldown to prevent spam
    float placeCooldown;
    float removeCooldown;
    static constexpr float COOLDOWN_TIME = 0.2f;

    void PerformRaycast(const Camera& camera, World& world);
    void HandleInput(Input& input, World& world, float dt);
    void PlaceBlock(World& world);
    void RemoveBlock(World& world);
};

} // namespace vge
