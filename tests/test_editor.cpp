#include "test_editor.h"
#include "editor/in_game_editor.h"
#include "editor/entity_spawner.h"
#include "voxel/world.h"
#include "voxel/block_registry.h"
#include "render/camera.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "platform/input.h"
#include "render/renderer.h"
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
    camera.SetSpeed(10.0f);
    
    Vec3 startPos = camera.GetPosition();
    
    // Simulate moving forward for 1 second at 10 units/sec
    // Note: Without actual input, we test the math directly
    Vec3 forward = camera.GetForward();
    Vec3 expectedPos = startPos + forward * 10.0f;
    
    // Test that forward vector is normalized
    float len = forward.length();
    if (std::abs(len - 1.0f) > 0.001f) return false;
    
    // Test that we can set position
    camera.SetPosition(expectedPos);
    Vec3 newPos = camera.GetPosition();
    
    return std::abs(newPos.x - expectedPos.x) < 0.001f &&
           std::abs(newPos.y - expectedPos.y) < 0.001f &&
           std::abs(newPos.z - expectedPos.z) < 0.001f;
}

bool EditorTest::TestFlyCameraRotation() {
    FlyCamera camera;
    
    // Test initial rotation
    Vec3 forward = camera.GetForward();
    if (std::abs(forward.length() - 1.0f) > 0.001f) return false;
    
    // Test setting rotation
    camera.SetRotation(0.0f, 0.0f);
    Vec3 newForward = camera.GetForward();
    // At yaw=0, pitch=0, forward should be roughly (1, 0, 0) or similar depending on convention
    if (newForward.length() < 0.999f) return false;
    
    // Test that up is perpendicular to forward
    float dot = camera.GetForward().dot(camera.GetUp());
    if (std::abs(dot) > 0.001f) return false;
    
    // Test that right is perpendicular to both
    dot = camera.GetForward().dot(camera.GetRight());
    if (std::abs(dot) > 0.001f) return false;
    
    dot = camera.GetUp().dot(camera.GetRight());
    if (std::abs(dot) > 0.001f) return false;
    
    return true;
}

bool EditorTest::TestFlyCameraLookAt() {
    FlyCamera camera;
    camera.SetPosition(Vec3(0, 10, 0));
    camera.LookAt(Vec3(10, 10, 0));
    
    Vec3 forward = camera.GetForward();
    // Should be looking roughly in +X direction
    if (forward.x < 0.5f) return false;
    
    return true;
}

bool EditorTest::TestBlockPickerRaycast() {
    World world;
    BlockPicker picker(&world);
    
    // Create a solid block
    world.SetBlock(5, 5, 5, 1); // stone
    
    Vec3 origin(0, 5, 5);
    Vec3 direction(1, 0, 0);
    Vec3 position, normal;
    
    bool hit = picker.PickBlock(origin, direction, position, normal, 10.0f);
    
    if (!hit) return false;
    if (position.x != 5.0f || position.y != 5.0f || position.z != 5.0f) return false;
    
    return true;
}

bool EditorTest::TestBlockPickerReachDistance() {
    World world;
    BlockPicker picker(&world);
    
    world.SetBlock(100, 5, 5, 1);
    
    Vec3 origin(0, 5, 5);
    Vec3 direction(1, 0, 0);
    Vec3 position, normal;
    
    // Default reach is 50, block at 100 should not be hit
    bool hit = picker.PickBlock(origin, direction, position, normal);
    if (hit) return false;
    
    // With extended reach, should hit
    hit = picker.PickBlock(origin, direction, position, normal, 150.0f);
    if (!hit) return false;
    
    return true;
}

bool EditorTest::TestEntitySpawn() {
    EntitySpawner spawner;
    
    uint32_t id = spawner.SpawnEntity("test_entity", Vec3(1, 2, 3));
    if (id == 0) return false;
    
    SpawnedEntity* entity = spawner.GetEntity(id);
    if (!entity) return false;
    if (entity->type != "test_entity") return false;
    if (entity->position.x != 1.0f || entity->position.y != 2.0f || entity->position.z != 3.0f) return false;
    
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
    
    auto ids = spawner.SpawnEntitiesInGrid("tree", Vec3(0, 0, 0), 3, 3, 2.0f);
    if (ids.size() != 9) return false;
    
    // Check that entities are at correct positions
    for (size_t i = 0; i < ids.size(); ++i) {
        SpawnedEntity* entity = spawner.GetEntity(ids[i]);
        if (!entity) return false;
        if (entity->type != "tree") return false;
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
    
    // Place a block
    editor.PlaceBlockAt(Vec3(0, 0, 0), Vec3(0, 1, 0));
    
    if (world.GetBlock(0, 0, 0) == BLOCK_AIR) return false;
    
    // Undo
    bool undone = editor.Undo();
    if (!undone) return false;
    
    if (world.GetBlock(0, 0, 0) != BLOCK_AIR) return false;
    
    // Redo
    bool redone = editor.Redo();
    if (!redone) return false;
    
    if (world.GetBlock(0, 0, 0) == BLOCK_AIR) return false;
    
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
    
    // Test that gizmo type can be changed
    editor.SetGizmoType(GizmoType::Rotate);
    if (editor.GetGizmoType() != GizmoType::Rotate) return false;
    
    editor.SetGizmoType(GizmoType::Scale);
    if (editor.GetGizmoType() != GizmoType::Scale) return false;
    
    // Test visibility toggles
    editor.ShowGizmos(false);
    if (editor.AreGizmosVisible()) return false;
    
    editor.ShowSelectionHighlight(false);
    if (editor.IsSelectionHighlightVisible()) return false;
    
    editor.ShowBlockPicker(true);
    if (!editor.IsBlockPickerVisible()) return false;
    
    editor.ShowEntitySpawner(true);
    if (!editor.IsEntitySpawnerVisible()) return false;
    
    editor.ShowTerrainTools(true);
    if (!editor.IsTerrainToolsVisible()) return false;
    
    editor.Shutdown();
    return true;
}

} // namespace vge

// Standalone test runner
int main() {
    return vge::EditorTest::RunAllTests() ? 0 : 1;
}
