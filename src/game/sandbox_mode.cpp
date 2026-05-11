#include "game/sandbox_mode.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "voxel/chunk_manager.h"
#include "voxel/block_registry.h"
#include "voxel/voxel_editor.h"
#include "editor/in_game_editor.h"
#include "rendering/camera.h"
#include "platform/input.h"
#include "rendering/renderer.h"
#include "core/logger.h"
#include <cmath>
#include <iostream>
#include <iomanip>

namespace vge {

SandboxMode::SandboxMode(World* w, WorldGenerator* gen, ChunkManager* chunks,
                         Camera* cam, Input* inp, Renderer* rend)
    : world(w), worldGenerator(gen), chunkManager(chunks),
      camera(cam), input(inp), renderer(rend),
      voxelEditor(nullptr), inGameEditor(nullptr),
      blockPicker(nullptr), flyCamera(nullptr),
      initialized(false), editorModeActive(false),
      debugOverlayVisible(false), selectedBlockType(1),
      playerPosition(0, 20, 0), playerYaw(0), playerPitch(0),
      flySpeed(10.0f), mouseSensitivity(0.1f),
      frameCount(0), fpsTimer(0), currentFps(0),
      terrainChunkRadius(5), terrainHeight(4) {}

SandboxMode::~SandboxMode() {
    Shutdown();
}

bool SandboxMode::Initialize() {
    if (initialized) return true;

    Logger::Info("[Sandbox] Initializing sandbox mode...");

    // Create subsystems
    voxelEditor = new VoxelEditor(world);
    inGameEditor = new InGameEditor(world, camera, input, renderer);
    blockPicker = new BlockPicker(world);
    flyCamera = new FlyCamera();

    if (!inGameEditor->Initialize()) {
        Logger::Error("[Sandbox] Failed to initialize in-game editor");
        return false;
    }

    // Set default selected block to grass
    BlockRegistry& registry = BlockRegistry::GetInstance();
    selectedBlockType = registry.GetBlockId("grass");
    if (selectedBlockType == BLOCK_AIR) {
        selectedBlockType = 1; // Fallback
    }

    if (voxelEditor) {
        voxelEditor->SetSelectedBlock(selectedBlockType);
    }

    // Configure fly camera
    flyCamera->SetSpeed(flySpeed);
    flyCamera->SetSensitivity(mouseSensitivity);
    flyCamera->SetPosition(playerPosition);
    flyCamera->Enable();

    // Generate flat terrain
    GenerateFlatTerrain();

    // Spawn player
    SpawnPlayer(Vec3(0, 20, 0));

    initialized = true;
    Logger::Info("[Sandbox] Sandbox mode initialized");
    return true;
}

void SandboxMode::Shutdown() {
    if (!initialized) return;

    Logger::Info("[Sandbox] Shutting down sandbox mode...");

    delete inGameEditor;
    delete voxelEditor;
    delete blockPicker;
    delete flyCamera;

    inGameEditor = nullptr;
    voxelEditor = nullptr;
    blockPicker = nullptr;
    flyCamera = nullptr;

    initialized = false;
    Logger::Info("[Sandbox] Sandbox mode shutdown complete");
}

void SandboxMode::GenerateFlatTerrain() {
    if (!world || !worldGenerator) {
        Logger::Error("[Sandbox] Cannot generate terrain - missing world or generator");
        return;
    }

    Logger::Info("[Sandbox] Generating flat grass terrain (" +
                 std::to_string(terrainChunkRadius * 2) + "x" +
                 std::to_string(terrainChunkRadius * 2) + " chunks)...");

    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID grassId = registry.GetBlockId("grass");
    BlockTypeID dirtId = registry.GetBlockId("dirt");
    BlockTypeID stoneId = registry.GetBlockId("stone");
    BlockTypeID bedrockId = registry.GetBlockId("bedrock");

    // Fallbacks if blocks not registered
    if (grassId == BLOCK_AIR) grassId = 1;
    if (dirtId == BLOCK_AIR) dirtId = 2;
    if (stoneId == BLOCK_AIR) stoneId = 3;
    if (bedrockId == BLOCK_AIR) bedrockId = stoneId;

    // Generate flat terrain across chunk radius
    for (int cx = -terrainChunkRadius; cx < terrainChunkRadius; cx++) {
        for (int cz = -terrainChunkRadius; cz < terrainChunkRadius; cz++) {
            Chunk* chunk = world->GetOrCreateChunk(cx, 0, cz);
            if (!chunk) continue;

            // Clear chunk first
            for (int x = 0; x < CHUNK_SIZE; x++) {
                for (int y = 0; y < CHUNK_SIZE; y++) {
                    for (int z = 0; z < CHUNK_SIZE; z++) {
                        chunk->SetBlock(x, y, z, BLOCK_AIR);
                    }
                }
            }

            // Fill with flat terrain
            for (int x = 0; x < CHUNK_SIZE; x++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    int worldX = cx * CHUNK_SIZE + x;
                    int worldZ = cz * CHUNK_SIZE + z;

                    // Surface at y = terrainHeight
                    int surfaceY = terrainHeight;

                    for (int y = 0; y < CHUNK_SIZE && y <= surfaceY + 2; y++) {
                        int worldY = y;

                        if (worldY == surfaceY) {
                            chunk->SetBlock(x, y, z, grassId);
                        } else if (worldY > surfaceY - 4 && worldY < surfaceY) {
                            chunk->SetBlock(x, y, z, dirtId);
                        } else if (worldY >= 0 && worldY <= surfaceY - 4) {
                            chunk->SetBlock(x, y, z, stoneId);
                        } else if (worldY < 0) {
                            chunk->SetBlock(x, y, z, bedrockId);
                        }
                    }
                }
            }

            chunk->loaded = true;
            chunk->SetDirty(true);
        }
    }

    Logger::Info("[Sandbox] Flat terrain generation complete");
}

void SandboxMode::ClearTerrain() {
    if (!world) return;

    Logger::Info("[Sandbox] Clearing terrain...");
    world->Clear();
    Logger::Info("[Sandbox] Terrain cleared");
}

void SandboxMode::GenerateTerrain(int chunkRadius, int height) {
    terrainChunkRadius = chunkRadius;
    terrainHeight = height;
    GenerateFlatTerrain();
}

void SandboxMode::SpawnPlayer(const Vec3& position) {
    playerPosition = position;
    playerYaw = 0;
    playerPitch = -10.0f;

    if (camera) {
        camera->SetPosition(playerPosition);
        camera->SetRotation(playerYaw, playerPitch, 0);
    }

    if (flyCamera) {
        flyCamera->SetPosition(playerPosition);
        flyCamera->SetRotation(playerYaw, playerPitch);
    }

    Logger::Info("[Sandbox] Player spawned at (" +
                 std::to_string((int)position.x) + ", " +
                 std::to_string((int)position.y) + ", " +
                 std::to_string((int)position.z) + ")");
}

void SandboxMode::SetPlayerPosition(const Vec3& pos) {
    playerPosition = pos;
    if (camera) {
        camera->SetPosition(pos);
    }
    if (flyCamera) {
        flyCamera->SetPosition(pos);
    }
}

void SandboxMode::Update(float deltaTime) {
    if (!initialized || !input) return;

    // Update debug info
    UpdateDebugInfo(deltaTime);

    // Handle F1 - Toggle editor mode
    if (input->IsKeyJustPressed(GLFW_KEY_F1)) {
        ToggleEditorMode();
    }

    // Handle F3 - Toggle debug overlay
    if (input->IsKeyJustPressed(GLFW_KEY_F3)) {
        ToggleDebugOverlay();
    }

    // Update fly camera
    UpdateFlyCamera(deltaTime);

    // Handle block editing
    HandleBlockEditing();

    // Update editor if active
    if (editorModeActive && inGameEditor) {
        inGameEditor->Update(deltaTime, *input);
    }
}

void SandboxMode::UpdateFlyCamera(float deltaTime) {
    if (!camera || !input || !flyCamera) return;

    // Update fly camera with input
    flyCamera->Update(deltaTime, *input);

    // Sync camera to fly camera
    playerPosition = flyCamera->GetPosition();
    playerYaw = flyCamera->GetYaw();
    playerPitch = flyCamera->GetPitch();

    camera->SetPosition(playerPosition);
    camera->SetRotation(playerYaw, playerPitch, 0);
}

void SandboxMode::HandleBlockEditing() {
    if (!input || !blockPicker || !voxelEditor) return;

    // Only edit when not in editor mode (or allow both)
    // Raycast from camera
    Vec3 origin = camera->GetPosition();
    Vec3 direction = camera->GetForward();

    Vec3 hitPos, hitNormal;
    bool hit = blockPicker->PickBlock(origin, direction, hitPos, hitNormal, 10.0f);

    if (!hit) return;

    // E - Place block
    if (input->IsKeyJustPressed(GLFW_KEY_E)) {
        Vec3 placePos = hitPos + hitNormal;
        PlaceBlockAt(placePos, hitNormal);
    }

    // Q - Remove block
    if (input->IsKeyJustPressed(GLFW_KEY_Q)) {
        RemoveBlockAt(hitPos);
    }
}

bool SandboxMode::PlaceBlockAt(const Vec3& position, const Vec3& normal) {
    (void)normal;
    if (!voxelEditor || !world) return false;

    // Round to integer coordinates
    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);

    // Check if position is air
    if (world->GetBlock(x, y, z) != BLOCK_AIR) {
        return false;
    }

    voxelEditor->SetTool(VoxelToolType::Place);
    voxelEditor->SetSelectedBlock(selectedBlockType);
    bool result = voxelEditor->ApplyToolAt(Vec3(x, y, z));

    if (result) {
        Logger::Info("[Sandbox] Placed block at (" +
                     std::to_string(x) + ", " +
                     std::to_string(y) + ", " +
                     std::to_string(z) + ")");
    }

    return result;
}

bool SandboxMode::RemoveBlockAt(const Vec3& position) {
    if (!voxelEditor || !world) return false;

    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);

    // Check if there is a block to remove
    if (world->GetBlock(x, y, z) == BLOCK_AIR) {
        return false;
    }

    voxelEditor->SetTool(VoxelToolType::Remove);
    bool result = voxelEditor->ApplyToolAt(Vec3(x, y, z));

    if (result) {
        Logger::Info("[Sandbox] Removed block at (" +
                     std::to_string(x) + ", " +
                     std::to_string(y) + ", " +
                     std::to_string(z) + ")");
    }

    return result;
}

void SandboxMode::SetSelectedBlock(BlockTypeID type) {
    selectedBlockType = type;
    if (voxelEditor) {
        voxelEditor->SetSelectedBlock(type);
    }
}

void SandboxMode::ToggleEditorMode() {
    editorModeActive = !editorModeActive;

    if (editorModeActive) {
        if (inGameEditor) {
            inGameEditor->Activate();
        }
        Logger::Info("[Sandbox] Editor mode activated");
    } else {
        if (inGameEditor) {
            inGameEditor->Deactivate();
        }
        Logger::Info("[Sandbox] Editor mode deactivated");
    }
}

void SandboxMode::SetEditorModeActive(bool active) {
    if (active != editorModeActive) {
        ToggleEditorMode();
    }
}

void SandboxMode::ToggleDebugOverlay() {
    debugOverlayVisible = !debugOverlayVisible;
    Logger::Info(std::string("[Sandbox] Debug overlay ") +
                 (debugOverlayVisible ? "enabled" : "disabled"));
}

void SandboxMode::SetDebugOverlayVisible(bool visible) {
    debugOverlayVisible = visible;
}

void SandboxMode::UpdateDebugInfo(float deltaTime) {
    frameCount++;
    fpsTimer += deltaTime;

    if (fpsTimer >= 1.0f) {
        currentFps = frameCount / fpsTimer;
        frameCount = 0;
        fpsTimer = 0;
    }
}

void SandboxMode::RenderDebugOverlay() {
    if (!debugOverlayVisible) return;

    // Print debug info below the frame
    std::cout << "\n=== DEBUG INFO ===\n";
    std::cout << "FPS: " << std::fixed << std::setprecision(1) << currentFps << "\n";
    std::cout << "Position: (" << (int)playerPosition.x << ", "
              << (int)playerPosition.y << ", " << (int)playerPosition.z << ")\n";

    // Chunk coordinates
    int chunkX = (int)(playerPosition.x / CHUNK_SIZE);
    int chunkY = (int)(playerPosition.y / CHUNK_SIZE);
    int chunkZ = (int)(playerPosition.z / CHUNK_SIZE);
    if (playerPosition.x < 0) chunkX--;
    if (playerPosition.y < 0) chunkY--;
    if (playerPosition.z < 0) chunkZ--;

    std::cout << "Chunk: (" << chunkX << ", " << chunkY << ", " << chunkZ << ")\n";
    std::cout << "Yaw: " << (int)playerYaw << " Pitch: " << (int)playerPitch << "\n";
    std::cout << "Editor: " << (editorModeActive ? "ON" : "OFF") << "\n";
    std::cout << "Selected Block: " << selectedBlockType << "\n";
    std::cout << "==================\n";
}

void SandboxMode::Render() {
    if (!initialized) return;

    // Render debug overlay after the frame
    RenderDebugOverlay();
}

} // namespace vge
