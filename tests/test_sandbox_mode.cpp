#include <iostream>
#include <cassert>
#include "game/sandbox_mode.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "voxel/chunk_manager.h"
#include "rendering/camera.h"
#include "platform/input.h"
#include "rendering/renderer.h"
#include "voxel/block_registry.h"

using namespace vge;

// Mock/Stub classes for testing
class TestCamera : public Camera {
public:
    TestCamera() : Camera() {}
};

class TestInput : public Input {
public:
    TestInput() : Input() {}
    void SimulateKeyPress(int key) {
        // Would need to expose internal state for full testing
        (void)key;
    }
};

class TestRenderer : public Renderer {
public:
    TestRenderer() : Renderer() {}
};

void test_sandbox_initialization() {
    std::cout << "Test: Sandbox Initialization..." << std::endl;

    World world;
    WorldGenerator generator;
    ChunkManager chunkManager;
    TestCamera camera;
    TestInput input;
    TestRenderer renderer;

    chunkManager.Initialize(&world, &generator);

    SandboxMode sandbox(&world, &generator, &chunkManager, &camera, &input, &renderer);

    assert(!sandbox.IsInitialized());
    bool result = sandbox.Initialize();
    assert(result);
    assert(sandbox.IsInitialized());

    std::cout << "  PASSED: Sandbox initializes correctly" << std::endl;

    sandbox.Shutdown();
    assert(!sandbox.IsInitialized());

    std::cout << "  PASSED: Sandbox shuts down correctly" << std::endl;
}

void test_sandbox_terrain_generation() {
    std::cout << "Test: Terrain Generation..." << std::endl;

    World world;
    WorldGenerator generator;
    ChunkManager chunkManager;
    TestCamera camera;
    TestInput input;
    TestRenderer renderer;

    chunkManager.Initialize(&world, &generator);

    SandboxMode sandbox(&world, &generator, &chunkManager, &camera, &input, &renderer);
    sandbox.Initialize();

    // Check that terrain was generated (chunks should exist)
    int chunksFound = 0;
    for (int cx = -5; cx < 5; cx++) {
        for (int cz = -5; cz < 5; cz++) {
            Chunk* chunk = world.GetChunk(cx, 0, cz);
            if (chunk && chunk->loaded) {
                chunksFound++;

                // Check that surface blocks exist
                BlockTypeID block = chunk->GetBlock(0, 4, 0);
                assert(block != BLOCK_AIR);
            }
        }
    }

    assert(chunksFound > 0);
    std::cout << "  PASSED: Generated " << chunksFound << " chunks" << std::endl;

    sandbox.Shutdown();
}

void test_sandbox_player_spawn() {
    std::cout << "Test: Player Spawn..." << std::endl;

    World world;
    WorldGenerator generator;
    ChunkManager chunkManager;
    TestCamera camera;
    TestInput input;
    TestRenderer renderer;

    chunkManager.Initialize(&world, &generator);

    SandboxMode sandbox(&world, &generator, &chunkManager, &camera, &input, &renderer);
    sandbox.Initialize();

    Vec3 pos = sandbox.GetPlayerPosition();
    assert(pos.x == 0.0f);
    assert(pos.y == 20.0f);
    assert(pos.z == 0.0f);

    std::cout << "  PASSED: Player spawned at correct position" << std::endl;

    sandbox.Shutdown();
}

void test_sandbox_block_editing() {
    std::cout << "Test: Block Editing..." << std::endl;

    World world;
    WorldGenerator generator;
    ChunkManager chunkManager;
    TestCamera camera;
    TestInput input;
    TestRenderer renderer;

    chunkManager.Initialize(&world, &generator);

    SandboxMode sandbox(&world, &generator, &chunkManager, &camera, &input, &renderer);
    sandbox.Initialize();

    // Set selected block
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID stoneId = registry.GetBlockId("stone");
    if (stoneId == BLOCK_AIR) stoneId = 3;

    sandbox.SetSelectedBlock(stoneId);
    assert(sandbox.GetSelectedBlock() == stoneId);

    std::cout << "  PASSED: Block selection works" << std::endl;

    sandbox.Shutdown();
}

void test_sandbox_editor_mode() {
    std::cout << "Test: Editor Mode Toggle..." << std::endl;

    World world;
    WorldGenerator generator;
    ChunkManager chunkManager;
    TestCamera camera;
    TestInput input;
    TestRenderer renderer;

    chunkManager.Initialize(&world, &generator);

    SandboxMode sandbox(&world, &generator, &chunkManager, &camera, &input, &renderer);
    sandbox.Initialize();

    assert(!sandbox.IsEditorModeActive());

    sandbox.ToggleEditorMode();
    assert(sandbox.IsEditorModeActive());

    sandbox.ToggleEditorMode();
    assert(!sandbox.IsEditorModeActive());

    std::cout << "  PASSED: Editor mode toggles correctly" << std::endl;

    sandbox.Shutdown();
}

void test_sandbox_debug_overlay() {
    std::cout << "Test: Debug Overlay Toggle..." << std::endl;

    World world;
    WorldGenerator generator;
    ChunkManager chunkManager;
    TestCamera camera;
    TestInput input;
    TestRenderer renderer;

    chunkManager.Initialize(&world, &generator);

    SandboxMode sandbox(&world, &generator, &chunkManager, &camera, &input, &renderer);
    sandbox.Initialize();

    assert(!sandbox.IsDebugOverlayVisible());

    sandbox.ToggleDebugOverlay();
    assert(sandbox.IsDebugOverlayVisible());

    sandbox.ToggleDebugOverlay();
    assert(!sandbox.IsDebugOverlayVisible());

    std::cout << "  PASSED: Debug overlay toggles correctly" << std::endl;

    sandbox.Shutdown();
}

void test_sandbox_fly_speed() {
    std::cout << "Test: Fly Speed..." << std::endl;

    World world;
    WorldGenerator generator;
    ChunkManager chunkManager;
    TestCamera camera;
    TestInput input;
    TestRenderer renderer;

    chunkManager.Initialize(&world, &generator);

    SandboxMode sandbox(&world, &generator, &chunkManager, &camera, &input, &renderer);
    sandbox.Initialize();

    float defaultSpeed = sandbox.GetFlySpeed();
    assert(defaultSpeed > 0.0f);

    sandbox.SetFlySpeed(25.0f);
    assert(sandbox.GetFlySpeed() == 25.0f);

    std::cout << "  PASSED: Fly speed can be changed" << std::endl;

    sandbox.Shutdown();
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Sandbox Mode Unit Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_sandbox_initialization();
        test_sandbox_terrain_generation();
        test_sandbox_player_spawn();
        test_sandbox_block_editing();
        test_sandbox_editor_mode();
        test_sandbox_debug_overlay();
        test_sandbox_fly_speed();

        std::cout << "\n========================================" << std::endl;
        std::cout << "  ALL TESTS PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
