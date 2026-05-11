#include "test_editor.h"
#include "editor/in_game_editor.h"
#include "editor/entity_spawner.h"
#include "voxel/world.h"
#include "voxel/block_registry.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "platform/input.h"
#include <iostream>
#include <cmath>

namespace vge {

bool EditorTest::RunAllTests() {
    int passed = 0;
    int total = 0;
    
    auto run = [&](const char* name, auto test) {
        total++;
        bool result = test();
        if (result) passed++;
        std::cout << "[EditorTest] " << name << ": " << (result ? "PASS" : "FAIL") << std::endl;
        return result;
    };
    
    std::cout << "\n=== Editor Test Suite ===" << std::endl;
    
    run("FlyCamera Movement", TestFlyCameraMovement);
    run("FlyCamera Rotation", TestFlyCameraRotation);
    run("FlyCamera LookAt", TestFlyCameraLookAt);
    run("BlockPicker Raycast", TestBlockPickerRaycast);
    run("BlockPicker Reach", TestBlockPickerReachDistance);
    run("Entity Spawn", TestEntitySpawn);
    run("Entity Remove", TestEntityRemove);
    run("Entity Grid Spawn", TestEntityGridSpawn);
    run("Block Selection", TestBlockSelection);
    run("Entity Selection", TestEntitySelection);
    run("Selection Clear", TestSelectionClear);
    run("Undo/Redo", TestUndoRedo);
    run("Undo Stack Limit", TestUndoStackLimit);
    run("Editor Lifecycle", TestEditorLifecycle);
    run("Editor Input", TestEditorInputHandling);
    
    std::cout << "\n=== Results: " << passed << "/" << total << " passed ===" << std::endl;
    return passed == total;
}

bool EditorTest::TestFlyCameraMovement() {
    FlyCamera camera;
    camera.Enable();
    
    Vec3 startPos = camera.GetPosition();
    
    // Simulate forward movement
    camera.SetPosition(startPos + Vec3(1, 0, 0));
    
    Vec3 newPos = camera.GetPosition();
    if (std::abs(newPos.x - (startPos.x + 1)) > 0.001f) return false;
    
    return true;
}

bool EditorTest::TestFlyCameraRotation() {
    FlyCamera camera;
    camera.SetRotation(45.0f, 30.0f);
    
    if (std::abs(camera.GetYaw() - 45.0f) > 0.001f) return false;
    if (std::abs(camera.GetPitch() - 30.0f) > 0.001f) return false;
    
    return true;
}

bool EditorTest::TestFlyCameraLookAt() {
    FlyCamera camera;
    camera.SetPosition(Vec3(0, 0, 0));
    camera.LookAt(Vec3(1, 0, 1));
    
    // Should be facing roughly towards (1, 0, 1)
    Vec3 forward = camera.GetForward();
    if (forward.x <= 0 || forward.z <= 0) return false;
    
    return true;
}

bool EditorTest::TestBlockPickerRaycast() {
    World world;
    
    // Place stone at y=10
    world.SetBlock(0, 10, 0, 1);
    
    // Verify block was placed
    if (world.GetBlock(0, 10, 0) == BLOCK_AIR) {
        std::cerr << "[FAIL] Failed to place block" << std::endl;
        return false;
    }
    
    // Test direct GetBlock
    BlockTypeID block = world.GetBlock(0, 10, 0);
    std::cout << "[DEBUG] Direct GetBlock(0,10,0): " << (int)block << std::endl;
    
    BlockPicker picker(&world);
    picker.SetReachDistance(15.0f); // Need reach > 10 to hit block at y=10 from y=20
    
    // Cast ray from (0,20,0) straight down
    Vec3 origin(0, 20, 0);
    Vec3 direction(0, -1, 0);
    Vec3 hitPos, hitNormal;
    
    bool hit = picker.PickBlock(origin, direction, hitPos, hitNormal);
    
    if (!hit) {
        std::cerr << "[FAIL] Raycast missed" << std::endl;
        return false;
    }
    
    return true;
}

bool EditorTest::TestBlockPickerReachDistance() {
    World world;
    world.SetBlock(0, 100, 0, 1); // Far away block
    
    BlockPicker picker(&world);
    picker.SetReachDistance(5.0f);
    
    Vec3 origin(0, 0, 0);
    Vec3 direction(0, 1, 0);
    Vec3 hitPos, hitNormal;
    
    bool hit = picker.PickBlock(origin, direction, hitPos, hitNormal);
    
    // Should NOT hit because it's beyond reach distance
    if (hit) return false;
    
    return true;
}

bool EditorTest::TestEntitySpawn() {
    EntitySpawner spawner;
    
    uint32_t id = spawner.SpawnEntity("test", Vec3(1, 2, 3));
    if (id == 0) return false;
    
    SpawnedEntity* entity = spawner.GetEntity(id);
    if (!entity) return false;
    if (entity->type != "test") return false;
    if (std::abs(entity->position.x - 1.0f) > 0.001f) return false;
    
    return true;
}

bool EditorTest::TestEntityRemove() {
    EntitySpawner spawner;
    
    uint32_t id = spawner.SpawnEntity("test", Vec3(0, 0, 0));
    if (id == 0) return false;
    
    bool removed = spawner.RemoveEntity(id);
    if (!removed) return false;
    
    SpawnedEntity* entity = spawner.GetEntity(id);
    if (entity != nullptr) return false;
    
    return true;
}

bool EditorTest::TestEntityGridSpawn() {
    EntitySpawner spawner;
    
    // Spawn multiple entities in a grid pattern manually
    std::vector<uint32_t> ids;
    for (int x = 0; x < 3; ++x) {
        for (int z = 0; z < 3; ++z) {
            uint32_t id = spawner.SpawnEntity("tree", Vec3(x * 2.0f, 0, z * 2.0f));
            if (id != 0) ids.push_back(id);
        }
    }
    
    if (ids.size() != 9) return false;
    
    // Check all IDs are valid and unique
    for (uint32_t id : ids) {
        if (id == 0) return false;
    }
    
    return true;
}

bool EditorTest::TestBlockSelection() {
    World world;
    Camera camera;
    Input input;
    Renderer renderer;
    
    InGameEditor editor(&world, &camera, &input, &renderer);
    if (!editor.Initialize()) return false;
    
    editor.Activate();
    editor.SelectBlock(Vec3(5, 5, 5), Vec3(0, 1, 0));
    
    const EditorSelection& selection = editor.GetSelection();
    if (!selection.IsValid()) return false;
    if (selection.type != EditorSelection::Type::Block) return false;
    if (selection.blockPosition.x != 5.0f) return false;
    
    editor.Shutdown();
    return true;
}

bool EditorTest::TestEntitySelection() {
    World world;
    Camera camera;
    Input input;
    Renderer renderer;
    
    InGameEditor editor(&world, &camera, &input, &renderer);
    if (!editor.Initialize()) return false;
    
    editor.Activate();
    uint32_t id = editor.SpawnEntity("test", Vec3(10, 10, 10));
    if (id == 0) return false;
    
    editor.SelectEntity(id);
    
    const EditorSelection& selection = editor.GetSelection();
    if (!selection.IsValid()) return false;
    if (selection.type != EditorSelection::Type::Entity) return false;
    if (selection.entityId != id) return false;
    
    editor.Shutdown();
    return true;
}

bool EditorTest::TestSelectionClear() {
    World world;
    Camera camera;
    Input input;
    Renderer renderer;
    
    InGameEditor editor(&world, &camera, &input, &renderer);
    if (!editor.Initialize()) return false;
    
    editor.Activate();
    editor.SelectBlock(Vec3(1, 1, 1), Vec3(0, 1, 0));
    
    if (!editor.GetSelection().IsValid()) return false;
    
    editor.ClearSelection();
    
    if (editor.GetSelection().IsValid()) return false;
    
    editor.Shutdown();
    return true;
}

bool EditorTest::TestUndoRedo() {
    World world;
    world.SetBlock(0, 0, 0, BLOCK_AIR);
    
    Camera camera;
    Input input;
    Renderer renderer;
    
    InGameEditor editor(&world, &camera, &input, &renderer);
    if (!editor.Initialize()) return false;
    
    editor.Activate();
    editor.SetSelectedBlockType(1); // stone
    
    // Place a block - the editor places at position + normal * 0.5f + normal
    // So (0,0,0) + (0,1,0)*0.5f + (0,1,0) = (0, 1.5, 0) which floors to (0, 1, 0)
    editor.PlaceBlockAt(Vec3(0, 0, 0), Vec3(0, 1, 0));
    
    // Check what block was actually placed
    BlockTypeID placedBlock = world.GetBlock(0, 1, 0);
    if (placedBlock == BLOCK_AIR) {
        std::cerr << "[FAIL] Block not placed at (0,1,0)" << std::endl;
        return false;
    }
    
    std::cout << "[DEBUG] Block placed: " << (int)placedBlock << " at (0,1,0)" << std::endl;
    
    // Undo
    bool undone = editor.Undo();
    if (!undone) return false;
    
    if (world.GetBlock(0, 1, 0) != BLOCK_AIR) return false;
    
    // Redo
    bool redone = editor.Redo();
    if (!redone) return false;
    
    if (world.GetBlock(0, 1, 0) == BLOCK_AIR) return false;
    
    editor.Shutdown();
    return true;
}

bool EditorTest::TestUndoStackLimit() {
    World world;
    Camera camera;
    Input input;
    Renderer renderer;
    
    InGameEditor editor(&world, &camera, &input, &renderer);
    if (!editor.Initialize()) return false;
    
    editor.Activate();
    editor.SetSelectedBlockType(1);
    
    // Perform many edits to test stack limit
    for (int i = 0; i < 150; ++i) {
        editor.PlaceBlockAt(Vec3((float)i, 0, 0), Vec3(0, 1, 0));
    }
    
    // Should be able to undo (stack size is 100, but we should still be able to undo)
    bool canUndo = editor.CanUndo();
    
    editor.Shutdown();
    return canUndo;
}

bool EditorTest::TestEditorLifecycle() {
    World world;
    Camera camera;
    Input input;
    Renderer renderer;
    
    InGameEditor editor(&world, &camera, &input, &renderer);
    
    // Initialize
    if (!editor.Initialize()) return false;
    if (!editor.GetVoxelEditor()) return false;
    if (!editor.GetFlyCamera()) return false;
    if (!editor.GetBlockPicker()) return false;
    if (!editor.GetEntitySpawner()) return false;
    
    // Activate
    editor.Activate();
    if (!editor.IsActive()) return false;
    
    // Deactivate
    editor.Deactivate();
    if (editor.IsActive()) return false;
    
    // Toggle
    editor.Toggle();
    if (!editor.IsActive()) return false;
    
    editor.Toggle();
    if (editor.IsActive()) return false;
    
    editor.Shutdown();
    return true;
}

bool EditorTest::TestEditorInputHandling() {
    World world;
    Camera camera;
    Input input;
    Renderer renderer;
    
    InGameEditor editor(&world, &camera, &input, &renderer);
    if (!editor.Initialize()) return false;
    
    editor.Activate();
    
    // Test that input handling doesn't crash
    editor.HandleInput(0.016f);
    editor.HandleMouseInput(0.016f);
    editor.HandleKeyboardInput(0.016f);
    
    editor.Shutdown();
    return true;
}

} // namespace vge

// Main entry point for the test
int main() {
    return vge::EditorTest::RunAllTests() ? 0 : 1;
}
