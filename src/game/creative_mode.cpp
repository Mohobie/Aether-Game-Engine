#include "game/creative_mode.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "voxel/chunk_manager.h"
#include "voxel/block_registry.h"
#include "voxel/voxel_editor.h"
#include "core/raycast.h"
#include "rendering/camera.h"
#include "platform/input.h"
#include "rendering/renderer.h"
#include "rendering/sky/day_night_cycle.h"
#include "game/block_interaction.h"
#include "core/logger.h"
#include <cmath>
#include <iostream>

namespace vge {

CreativeMode::CreativeMode(World* w, WorldGenerator* gen, ChunkManager* chunks,
                           Camera* cam, Input* inp, Renderer* rend)
    : world(w), worldGenerator(gen), chunkManager(chunks),
      camera(cam), input(inp), renderer(rend),
      voxelEditor(nullptr), blockPicker(nullptr),
      flyCamera(nullptr), blockInteraction(nullptr),
      dayNightCycle(nullptr),
      initialized(false), debugOverlayVisible(false),
      flightModeEnabled(true), instantBreakEnabled(true),
      invincible(true), selectedBlockType(1),
      playerPosition(0, 20, 0), spawnPoint(0, 20, 0),
      playerYaw(0), playerPitch(-10.0f),
      flySpeed(10.0f), flySpeedFast(25.0f),
      mouseSensitivity(0.1f),
      selectedInventorySlot(0),
      frameCount(0), fpsTimer(0), currentFps(0),
      terrainChunkRadius(5), terrainHeight(4) {}

CreativeMode::~CreativeMode() {
    Shutdown();
}

bool CreativeMode::Initialize() {
    if (initialized) return true;
    Logger::Info("[Creative] Initializing creative mode...");
    voxelEditor = new VoxelEditor(world);
    blockPicker = new BlockPicker(world);
    flyCamera = new FlyCamera();
    blockInteraction = new BlockInteraction();
    dayNightCycle = new DayNightCycle();
    if (blockInteraction) {
        blockInteraction->Initialize(*world);
    }
    BlockRegistry& registry = BlockRegistry::GetInstance();
    selectedBlockType = registry.GetBlockId("grass");
    if (selectedBlockType == BLOCK_AIR) selectedBlockType = 1;
    if (voxelEditor) voxelEditor->SetSelectedBlock(selectedBlockType);
    flyCamera->SetSpeed(flySpeed);
    flyCamera->SetSensitivity(mouseSensitivity);
    flyCamera->SetPosition(playerPosition);
    flyCamera->Enable();
    UpdateCreativeInventory();
    GenerateFlatTerrain();
    SpawnPlayer(Vec3(0, 20, 0));
    initialized = true;
    Logger::Info("[Creative] Creative mode initialized");
    return true;
}

void CreativeMode::Shutdown() {
    if (!initialized) return;
    Logger::Info("[Creative] Shutting down creative mode...");
    delete blockInteraction;
    delete dayNightCycle;
    delete flyCamera;
    delete blockPicker;
    delete voxelEditor;
    blockInteraction = nullptr;
    dayNightCycle = nullptr;
    flyCamera = nullptr;
    blockPicker = nullptr;
    voxelEditor = nullptr;
    initialized = false;
    Logger::Info("[Creative] Creative mode shutdown complete");
}

void CreativeMode::UpdateCreativeInventory() {
    creativeInventory.clear();
    BlockRegistry& registry = BlockRegistry::GetInstance();
    std::vector<std::string> allBlocks = registry.GetAllBlockIds();
    for (const auto& blockId : allBlocks) {
        if (blockId != "air") creativeInventory.push_back(blockId);
    }
    if (creativeInventory.empty()) {
        creativeInventory = {"stone", "dirt", "grass", "wood", "planks", "glass", "sand", "snow", "glowstone"};
    }
    while (static_cast<int>(creativeInventory.size()) < INVENTORY_SIZE) {
        creativeInventory.push_back("stone");
    }
    Logger::Info("[Creative] Creative inventory loaded with " + std::to_string(creativeInventory.size()) + " blocks");
}

void CreativeMode::GenerateFlatTerrain() {
    if (!world || !worldGenerator) {
        Logger::Error("[Creative] Cannot generate terrain - missing world or generator");
        return;
    }
    Logger::Info("[Creative] Generating flat grass terrain...");
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID grassId = registry.GetBlockId("grass");
    BlockTypeID dirtId = registry.GetBlockId("dirt");
    BlockTypeID stoneId = registry.GetBlockId("stone");
    BlockTypeID bedrockId = registry.GetBlockId("bedrock");
    if (grassId == BLOCK_AIR) grassId = 1;
    if (dirtId == BLOCK_AIR) dirtId = 2;
    if (stoneId == BLOCK_AIR) stoneId = 3;
    if (bedrockId == BLOCK_AIR) bedrockId = stoneId;
    for (int cx = -terrainChunkRadius; cx < terrainChunkRadius; cx++) {
        for (int cz = -terrainChunkRadius; cz < terrainChunkRadius; cz++) {
            Chunk* chunk = world->GetOrCreateChunk(cx, 0, cz);
            if (!chunk) continue;
            for (int x = 0; x < CHUNK_SIZE; x++) {
                for (int y = 0; y < CHUNK_SIZE; y++) {
                    for (int z = 0; z < CHUNK_SIZE; z++) {
                        chunk->SetBlock(x, y, z, BLOCK_AIR);
                    }
                }
            }
            for (int x = 0; x < CHUNK_SIZE; x++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    int surfaceY = terrainHeight;
                    for (int y = 0; y < CHUNK_SIZE && y <= surfaceY + 2; y++) {
                        int worldY = y;
                        if (worldY == surfaceY) chunk->SetBlock(x, y, z, grassId);
                        else if (worldY > surfaceY - 4 && worldY < surfaceY) chunk->SetBlock(x, y, z, dirtId);
                        else if (worldY >= 0 && worldY <= surfaceY - 4) chunk->SetBlock(x, y, z, stoneId);
                        else if (worldY < 0) chunk->SetBlock(x, y, z, bedrockId);
                    }
                }
            }
            chunk->loaded = true;
            chunk->SetDirty(true);
        }
    }
    Logger::Info("[Creative] Flat terrain generation complete");
}

void CreativeMode::ClearTerrain() {
    if (!world) return;
    Logger::Info("[Creative] Clearing terrain...");
    world->Clear();
    Logger::Info("[Creative] Terrain cleared");
}

void CreativeMode::GenerateTerrain(int chunkRadius, int height) {
    terrainChunkRadius = chunkRadius;
    terrainHeight = height;
    GenerateFlatTerrain();
}

void CreativeMode::SpawnPlayer(const Vec3& position) {
    playerPosition = position;
    spawnPoint = position;
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
    Logger::Info("[Creative] Player spawned at (" + std::to_string((int)position.x) + ", " + std::to_string((int)position.y) + ", " + std::to_string((int)position.z) + ")");
}

void CreativeMode::SetPlayerPosition(const Vec3& pos) {
    playerPosition = pos;
    if (camera) camera->SetPosition(pos);
    if (flyCamera) flyCamera->SetPosition(pos);
}

void CreativeMode::SyncCameraToPlayer() {
    if (!camera) return;
    camera->SetPosition(playerPosition);
    camera->SetRotation(playerYaw, playerPitch, 0);
}

void CreativeMode::Update(float deltaTime) {
    if (!initialized || !input) return;
    UpdateDebugInfo(deltaTime);
    if (dayNightCycle) dayNightCycle->Update(deltaTime);
    if (input->IsKeyJustPressed(GLFW_KEY_F3)) ToggleDebugOverlay();
    if (input->IsKeyJustPressed(GLFW_KEY_F5)) ToggleFlightMode();
    if (flightModeEnabled) UpdateFlight(deltaTime);
    HandleBlockEditing();
    if (blockInteraction && camera) blockInteraction->Update(*camera, *input, *world);
}

void CreativeMode::UpdateFlight(float deltaTime) {
    if (!camera || !input || !flyCamera) return;
    float currentSpeed = flySpeed;
    if (input->IsKeyPressed(GLFW_KEY_LEFT_CONTROL) || input->IsKeyPressed(GLFW_KEY_RIGHT_CONTROL)) {
        currentSpeed = flySpeedFast;
    }
    flyCamera->SetSpeed(currentSpeed);
    flyCamera->Update(deltaTime, *input);
    playerPosition = flyCamera->GetPosition();
    playerYaw = flyCamera->GetYaw();
    playerPitch = flyCamera->GetPitch();
    camera->SetPosition(playerPosition);
    camera->SetRotation(playerYaw, playerPitch, 0);
}

void CreativeMode::HandleBlockEditing() {
    if (!input || !blockPicker || !world) return;
    Vec3 origin = camera->GetPosition();
    Vec3 direction = camera->GetForward();
    Vec3 hitPos, hitNormal;
    bool hit = blockPicker->PickBlock(origin, direction, hitPos, hitNormal, 15.0f);
    if (!hit) return;
    if (input->IsKeyJustPressed(GLFW_KEY_E)) {
        Vec3 placePos = hitPos + hitNormal;
        PlaceBlockAt(placePos, hitNormal);
    }
    if (input->IsKeyJustPressed(GLFW_KEY_Q)) {
        RemoveBlockAt(hitPos);
    }
    if (input->IsKeyJustPressed(GLFW_KEY_1)) SelectSlot(0);
    if (input->IsKeyJustPressed(GLFW_KEY_2)) SelectSlot(1);
    if (input->IsKeyJustPressed(GLFW_KEY_3)) SelectSlot(2);
    if (input->IsKeyJustPressed(GLFW_KEY_4)) SelectSlot(3);
    if (input->IsKeyJustPressed(GLFW_KEY_5)) SelectSlot(4);
    if (input->IsKeyJustPressed(GLFW_KEY_6)) SelectSlot(5);
    if (input->IsKeyJustPressed(GLFW_KEY_7)) SelectSlot(6);
    if (input->IsKeyJustPressed(GLFW_KEY_8)) SelectSlot(7);
    if (input->IsKeyJustPressed(GLFW_KEY_9)) SelectSlot(8);
}

bool CreativeMode::PlaceBlockAt(const Vec3& position, const Vec3& normal) {
    (void)normal;
    if (!world) return false;
    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);
    if (world->GetBlock(x, y, z) != BLOCK_AIR) return false;
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID blockToPlace = selectedBlockType;
    if (selectedInventorySlot >= 0 && selectedInventorySlot < static_cast<int>(creativeInventory.size())) {
        BlockTypeID inventoryBlock = registry.GetBlockId(creativeInventory[selectedInventorySlot]);
        if (inventoryBlock != BLOCK_AIR) blockToPlace = inventoryBlock;
    }
    world->SetBlock(x, y, z, blockToPlace);
    Logger::Info("[Creative] Placed block at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
    return true;
}

bool CreativeMode::RemoveBlockAt(const Vec3& position) {
    if (!world) return false;
    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);
    if (world->GetBlock(x, y, z) == BLOCK_AIR) return false;
    world->SetBlock(x, y, z, BLOCK_AIR);
    Logger::Info("[Creative] Removed block at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
    return true;
}

void CreativeMode::SetSelectedBlock(BlockTypeID type) {
    selectedBlockType = type;
    if (voxelEditor) voxelEditor->SetSelectedBlock(type);
}

void CreativeMode::SetInventorySlot(int slot, const std::string& blockId) {
    if (slot >= 0 && slot < INVENTORY_SIZE) {
        if (slot >= static_cast<int>(creativeInventory.size())) creativeInventory.resize(slot + 1, "stone");
        creativeInventory[slot] = blockId;
    }
}

std::string CreativeMode::GetInventorySlot(int slot) const {
    if (slot >= 0 && slot < static_cast<int>(creativeInventory.size())) return creativeInventory[slot];
    return "stone";
}

void CreativeMode::SelectSlot(int slot) {
    if (slot >= 0 && slot < INVENTORY_SIZE) {
        selectedInventorySlot = slot;
        if (slot < static_cast<int>(creativeInventory.size())) {
            BlockRegistry& registry = BlockRegistry::GetInstance();
            BlockTypeID type = registry.GetBlockId(creativeInventory[slot]);
            if (type != BLOCK_AIR) SetSelectedBlock(type);
        }
        Logger::Info("[Creative] Selected slot " + std::to_string(slot + 1) + ": " + creativeInventory[slot]);
    }
}

void CreativeMode::NextSlot() { SelectSlot((selectedInventorySlot + 1) % INVENTORY_SIZE); }
void CreativeMode::PrevSlot() { SelectSlot((selectedInventorySlot - 1 + INVENTORY_SIZE) % INVENTORY_SIZE); }
void CreativeMode::ToggleFlightMode() {
    flightModeEnabled = !flightModeEnabled;
    Logger::Info(std::string("[Creative] Flight mode ") + (flightModeEnabled ? "enabled" : "disabled"));
}
void CreativeMode::SetFlightModeEnabled(bool enabled) { flightModeEnabled = enabled; }
void CreativeMode::ToggleInstantBreak() {
    instantBreakEnabled = !instantBreakEnabled;
    Logger::Info(std::string("[Creative] Instant break ") + (instantBreakEnabled ? "enabled" : "disabled"));
}
void CreativeMode::SetInstantBreakEnabled(bool enabled) { instantBreakEnabled = enabled; }
void CreativeMode::ToggleInvincible() {
    invincible = !invincible;
    Logger::Info(std::string("[Creative] Invincibility ") + (invincible ? "enabled" : "disabled"));
}
void CreativeMode::SetInvincible(bool enabled) { invincible = enabled; }
void CreativeMode::ToggleDebugOverlay() {
    debugOverlayVisible = !debugOverlayVisible;
    Logger::Info(std::string("[Creative] Debug overlay ") + (debugOverlayVisible ? "enabled" : "disabled"));
}
void CreativeMode::SetDebugOverlayVisible(bool visible) { debugOverlayVisible = visible; }
void CreativeMode::UpdateDebugInfo(float deltaTime) {
    frameCount++;
    fpsTimer += deltaTime;
    if (fpsTimer >= 1.0f) {
        currentFps = frameCount / fpsTimer;
        frameCount = 0;
        fpsTimer = 0;
    }
}
void CreativeMode::RenderDebugOverlay() {
    if (!debugOverlayVisible) return;
    std::cout << "\n=== CREATIVE MODE DEBUG ===\n";
    std::cout << "FPS: " << std::fixed << std::setprecision(1) << currentFps << "\n";
    std::cout << "Position: (" << (int)playerPosition.x << ", " << (int)playerPosition.y << ", " << (int)playerPosition.z << ")\n";
    int chunkX = (int)(playerPosition.x / CHUNK_SIZE);
    int chunkY = (int)(playerPosition.y / CHUNK_SIZE);
    int chunkZ = (int)(playerPosition.z / CHUNK_SIZE);
    if (playerPosition.x < 0) chunkX--;
    if (playerPosition.y < 0) chunkY--;
    if (playerPosition.z < 0) chunkZ--;
    std::cout << "Chunk: (" << chunkX << ", " << chunkY << ", " << chunkZ << ")\n";
    std::cout << "Yaw: " << (int)playerYaw << " Pitch: " << (int)playerPitch << "\n";
    std::cout << "Flight: " << (flightModeEnabled ? "ON" : "OFF") << "\n";
    std::cout << "Instant Break: " << (instantBreakEnabled ? "ON" : "OFF") << "\n";
    std::cout << "Invincible: " << (invincible ? "ON" : "OFF") << "\n";
    std::cout << "Fly Speed: " << flySpeed << " (Fast: " << flySpeedFast << ")\n";
    std::cout << "Selected Slot: " << (selectedInventorySlot + 1) << "/" << INVENTORY_SIZE << "\n";
    if (selectedInventorySlot < static_cast<int>(creativeInventory.size())) {
        std::cout << "Selected Block: " << creativeInventory[selectedInventorySlot] << "\n";
    }
    if (dayNightCycle) {
        std::cout << "Time: " << dayNightCycle->GetTimeString() << "\n";
        std::cout << "IsNight: " << (dayNightCycle->IsNight() ? "YES" : "NO") << "\n";
    }
    std::cout << "===========================\n";
}
void CreativeMode::Render() {
    if (!initialized) return;
    RenderDebugOverlay();
}

} // namespace vge
