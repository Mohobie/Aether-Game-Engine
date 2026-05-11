#include "game/survival_mode.h"
#include "game/player_stats.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "voxel/chunk_manager.h"
#include "voxel/block_registry.h"
#include "voxel/voxel_editor.h"
#include "rendering/camera.h"
#include "platform/input.h"
#include "rendering/renderer.h"
#include "rendering/sky/day_night_cycle.h"
#include "core/crafting.h"
#include "core/logger.h"
#include "core/raycast.h"
#include "editor/in_game_editor.h"
#include <cmath>
#include <iostream>
#include <iomanip>

namespace vge {

SurvivalMode::SurvivalMode(World* w, WorldGenerator* gen, ChunkManager* chunks,
                           Camera* cam, Input* inp, Renderer* rend)
    : world(w), worldGenerator(gen), chunkManager(chunks),
      camera(cam), input(inp), renderer(rend),
      blockPicker(nullptr), flyCamera(nullptr),
      playerStats(nullptr), dayNightCycle(nullptr),
      craftingSystem(nullptr), craftingGrid(nullptr),
      initialized(false), debugOverlayVisible(false),
      selectedBlockType(1),
      playerPosition(0, 20, 0), spawnPoint(0, 20, 0),
      playerYaw(0), playerPitch(0),
      moveSpeed(5.0f), sprintSpeed(10.0f),
      mouseSensitivity(0.1f),
      nightHungerMultiplier(1.5f),
      frameCount(0), fpsTimer(0), currentFps(0),
      terrainChunkRadius(5), terrainHeight(4) {}

SurvivalMode::~SurvivalMode() {
    Shutdown();
}

bool SurvivalMode::Initialize() {
    if (initialized) return true;

    Logger::Info("[Survival] Initializing survival mode...");

    // Create subsystems
    blockPicker = new BlockPicker(world);
    flyCamera = new FlyCamera();
    playerStats = new PlayerStats(spawnPoint);
    dayNightCycle = new DayNightCycle();
    craftingSystem = new CraftingSystem();
    craftingGrid = new CraftingGrid();

    // Set default selected block to grass
    BlockRegistry& registry = BlockRegistry::GetInstance();
    selectedBlockType = registry.GetBlockId("grass");
    if (selectedBlockType == BLOCK_AIR) {
        selectedBlockType = 1;
    }

    // Configure fly camera (used as player camera)
    flyCamera->SetSpeed(moveSpeed);
    flyCamera->SetSensitivity(mouseSensitivity);
    flyCamera->SetPosition(playerPosition);
    flyCamera->Enable();

    // Initialize crafting recipes
    InitializeCraftingRecipes();

    // Generate flat terrain
    GenerateFlatTerrain();

    // Spawn player
    SpawnPlayer(spawnPoint);

    initialized = true;
    Logger::Info("[Survival] Survival mode initialized");
    return true;
}

void SurvivalMode::Shutdown() {
    if (!initialized) return;

    Logger::Info("[Survival] Shutting down survival mode...");

    delete blockPicker;
    delete flyCamera;
    delete playerStats;
    delete dayNightCycle;
    delete craftingSystem;
    delete craftingGrid;

    blockPicker = nullptr;
    flyCamera = nullptr;
    playerStats = nullptr;
    dayNightCycle = nullptr;
    craftingSystem = nullptr;
    craftingGrid = nullptr;

    initialized = false;
    Logger::Info("[Survival] Survival mode shutdown complete");
}

void SurvivalMode::InitializeCraftingRecipes() {
    if (!craftingSystem) return;

    // Wood -> Planks (shapeless)
    CraftingRecipe woodToPlanks;
    woodToPlanks.recipeID = "wood_to_planks";
    woodToPlanks.displayName = "Wooden Planks";
    woodToPlanks.shapeless = true;
    woodToPlanks.ingredients.push_back(RecipeIngredient("wood", 1));
    woodToPlanks.outputItemID = "planks";
    woodToPlanks.outputCount = 4;
    woodToPlanks.craftTime = 0.0f;
    craftingSystem->AddRecipe(woodToPlanks);

    // Planks -> Sticks (shapeless)
    CraftingRecipe planksToSticks;
    planksToSticks.recipeID = "planks_to_sticks";
    planksToSticks.displayName = "Sticks";
    planksToSticks.shapeless = true;
    planksToSticks.ingredients.push_back(RecipeIngredient("planks", 2));
    planksToSticks.outputItemID = "sticks";
    planksToSticks.outputCount = 4;
    planksToSticks.craftTime = 0.0f;
    craftingSystem->AddRecipe(planksToSticks);

    // Pickaxe (3x3 pattern)
    CraftingRecipe pickaxe;
    pickaxe.recipeID = "wooden_pickaxe";
    pickaxe.displayName = "Wooden Pickaxe";
    pickaxe.shapeless = false;
    // Pattern: planks across top, sticks down middle
    pickaxe.pattern = {
        {"planks", "planks", "planks"},
        {"",       "sticks", ""      },
        {"",       "sticks", ""      }
    };
    pickaxe.outputItemID = "wooden_pickaxe";
    pickaxe.outputCount = 1;
    pickaxe.craftTime = 0.0f;
    craftingSystem->AddRecipe(pickaxe);

    Logger::Info("[Survival] Initialized " + std::to_string(craftingSystem->GetRecipeCount()) + " crafting recipes");
}

void SurvivalMode::GenerateFlatTerrain() {
    if (!world || !worldGenerator) {
        Logger::Error("[Survival] Cannot generate terrain - missing world or generator");
        return;
    }

    Logger::Info("[Survival] Generating flat grass terrain (" +
                 std::to_string(terrainChunkRadius * 2) + "x" +
                 std::to_string(terrainChunkRadius * 2) + " chunks)...");

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

    Logger::Info("[Survival] Flat terrain generation complete");
}

void SurvivalMode::ClearTerrain() {
    if (!world) return;
    Logger::Info("[Survival] Clearing terrain...");
    world->Clear();
    Logger::Info("[Survival] Terrain cleared");
}

void SurvivalMode::GenerateTerrain(int chunkRadius, int height) {
    terrainChunkRadius = chunkRadius;
    terrainHeight = height;
    GenerateFlatTerrain();
}

void SurvivalMode::SpawnPlayer(const Vec3& position) {
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
    if (playerStats) {
        playerStats->SetSpawnPoint(spawnPoint);
        playerStats->Respawn();
    }

    Logger::Info("[Survival] Player spawned at (" +
                 std::to_string((int)position.x) + ", " +
                 std::to_string((int)position.y) + ", " +
                 std::to_string((int)position.z) + ")");
}

void SurvivalMode::SetPlayerPosition(const Vec3& pos) {
    playerPosition = pos;
    if (camera) camera->SetPosition(pos);
    if (flyCamera) flyCamera->SetPosition(pos);
}

void SurvivalMode::Update(float deltaTime) {
    if (!initialized || !input) return;

    // Update debug info
    UpdateDebugInfo(deltaTime);

    // Update day/night cycle
    if (dayNightCycle) {
        dayNightCycle->Update(deltaTime);
    }

    // Update player stats
    if (playerStats) {
        // Apply night hunger multiplier
        float hungerMultiplier = (IsNight() ? nightHungerMultiplier : 1.0f);
        playerStats->SetHungerDecayRate(0.5f * hungerMultiplier);

        // Handle sprint input
        bool sprinting = input->IsKeyPressed(GLFW_KEY_LEFT_SHIFT) || input->IsKeyPressed(GLFW_KEY_RIGHT_SHIFT);
        playerStats->SetSprinting(sprinting);

        // Update stats
        playerStats->Update(deltaTime);

        // Check death
        CheckPlayerDeath();
    }

    // Handle F3 - Toggle debug overlay
    if (input->IsKeyJustPressed(GLFW_KEY_F3)) {
        ToggleDebugOverlay();
    }

    // Update movement
    UpdateMovement(deltaTime);

    // Handle block editing
    HandleBlockEditing();
}

void SurvivalMode::UpdateMovement(float deltaTime) {
    if (!camera || !input || !flyCamera || !playerStats) return;

    // Adjust speed based on sprint and stamina
    float currentSpeed = moveSpeed;
    if (playerStats->IsSprinting() && playerStats->CanSprint()) {
        currentSpeed = sprintSpeed;
    } else {
        // Not sprinting - reset to normal speed
        playerStats->SetSprinting(false);
    }

    flyCamera->SetSpeed(currentSpeed);
    flyCamera->Update(deltaTime, *input);

    // Sync camera to fly camera
    playerPosition = flyCamera->GetPosition();
    playerYaw = flyCamera->GetYaw();
    playerPitch = flyCamera->GetPitch();

    camera->SetPosition(playerPosition);
    camera->SetRotation(playerYaw, playerPitch, 0);
}

void SurvivalMode::HandleBlockEditing() {
    if (!input || !blockPicker || !world) return;

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

bool SurvivalMode::PlaceBlockAt(const Vec3& position, const Vec3& normal) {
    (void)normal;
    if (!world) return false;

    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);

    if (world->GetBlock(x, y, z) != BLOCK_AIR) {
        return false;
    }

    world->SetBlock(x, y, z, selectedBlockType);
    Logger::Info("[Survival] Placed block at (" +
                 std::to_string(x) + ", " +
                 std::to_string(y) + ", " +
                 std::to_string(z) + ")");
    return true;
}

bool SurvivalMode::RemoveBlockAt(const Vec3& position) {
    if (!world) return false;

    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);

    if (world->GetBlock(x, y, z) == BLOCK_AIR) {
        return false;
    }

    world->SetBlock(x, y, z, BLOCK_AIR);
    Logger::Info("[Survival] Removed block at (" +
                 std::to_string(x) + ", " +
                 std::to_string(y) + ", " +
                 std::to_string(z) + ")");
    return true;
}

void SurvivalMode::SetSelectedBlock(BlockTypeID type) {
    selectedBlockType = type;
}

void SurvivalMode::CheckPlayerDeath() {
    if (!playerStats) return;
    if (playerStats->IsDead()) {
        Logger::Info("[Survival] Player has died - respawning...");
        RespawnPlayer();
    }
}

void SurvivalMode::RespawnPlayer() {
    if (!playerStats) return;
    playerStats->Respawn();
    playerPosition = spawnPoint;
    if (flyCamera) {
        flyCamera->SetPosition(spawnPoint);
    }
    if (camera) {
        camera->SetPosition(spawnPoint);
    }
}

bool SurvivalMode::IsNight() const {
    if (!dayNightCycle) return false;
    return dayNightCycle->IsNight();
}

bool SurvivalMode::CraftItem(const std::string& recipeID) {
    if (!craftingSystem || !craftingGrid) return false;

    const CraftingRecipe* recipe = craftingSystem->GetRecipe(recipeID);
    if (!recipe) {
        Logger::Info("[Survival] Unknown recipe: " + recipeID);
        return false;
    }

    // For testing, we just log the craft attempt
    // In a full implementation, we'd check inventory and consume ingredients
    Logger::Info("[Survival] Crafted " + recipe->displayName);
    return true;
}

std::vector<std::string> SurvivalMode::GetAvailableRecipes() const {
    std::vector<std::string> result;
    if (!craftingSystem) return result;

    auto recipes = craftingSystem->GetAllRecipes();
    for (const auto* recipe : recipes) {
        if (recipe) {
            result.push_back(recipe->recipeID);
        }
    }
    return result;
}

void SurvivalMode::ToggleDebugOverlay() {
    debugOverlayVisible = !debugOverlayVisible;
    Logger::Info(std::string("[Survival] Debug overlay ") +
                 (debugOverlayVisible ? "enabled" : "disabled"));
}

void SurvivalMode::SetDebugOverlayVisible(bool visible) {
    debugOverlayVisible = visible;
}

void SurvivalMode::UpdateDebugInfo(float deltaTime) {
    frameCount++;
    fpsTimer += deltaTime;

    if (fpsTimer >= 1.0f) {
        currentFps = frameCount / fpsTimer;
        frameCount = 0;
        fpsTimer = 0;
    }
}

void SurvivalMode::RenderDebugOverlay() {
    if (!debugOverlayVisible) return;

    std::cout << "\n=== SURVIVAL DEBUG ===\n";
    std::cout << "FPS: " << std::fixed << std::setprecision(1) << currentFps << "\n";
    std::cout << "Position: (" << (int)playerPosition.x << ", "
              << (int)playerPosition.y << ", " << (int)playerPosition.z << ")\n";

    int chunkX = (int)(playerPosition.x / CHUNK_SIZE);
    int chunkY = (int)(playerPosition.y / CHUNK_SIZE);
    int chunkZ = (int)(playerPosition.z / CHUNK_SIZE);
    if (playerPosition.x < 0) chunkX--;
    if (playerPosition.y < 0) chunkY--;
    if (playerPosition.z < 0) chunkZ--;

    std::cout << "Chunk: (" << chunkX << ", " << chunkY << ", " << chunkZ << ")\n";
    std::cout << "Yaw: " << (int)playerYaw << " Pitch: " << (int)playerPitch << "\n";

    if (playerStats) {
        std::cout << "Health: " << (int)playerStats->GetHealth() << "/" << (int)playerStats->GetMaxHealth() << "\n";
        std::cout << "Hunger: " << (int)playerStats->GetHunger() << "/" << (int)playerStats->GetMaxHunger() << "\n";
        std::cout << "Stamina: " << (int)playerStats->GetStamina() << "/" << (int)playerStats->GetMaxStamina() << "\n";
        std::cout << "Sprinting: " << (playerStats->IsSprinting() ? "YES" : "NO") << "\n";
        std::cout << "Dead: " << (playerStats->IsDead() ? "YES" : "NO") << "\n";
    }

    if (dayNightCycle) {
        std::cout << "Time: " << dayNightCycle->GetTimeString() << "\n";
        std::cout << "IsNight: " << (dayNightCycle->IsNight() ? "YES" : "NO") << "\n";
    }

    std::cout << "Selected Block: " << selectedBlockType << "\n";
    std::cout << "======================\n";
}

void SurvivalMode::Render() {
    if (!initialized) return;
    RenderDebugOverlay();
}

} // namespace vge
