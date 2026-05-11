#pragma once
#include "math/vec3.h"
#include "voxel/block_types.h"
#include <memory>
#include <string>
#include <vector>

namespace vge {

// Forward declarations
class World;
class WorldGenerator;
class ChunkManager;
class Camera;
class Input;
class Renderer;
class BlockPicker;
class FlyCamera;
class PlayerStats;
class DayNightCycle;
class CraftingSystem;
class CraftingGrid;

// ============================================
// Survival Game Mode
// ============================================
// Survival gameplay with health, hunger, stamina systems,
// day/night cycle integration, and basic crafting.
//
// Controls:
//   WASD         - Movement
//   Shift        - Sprint (consumes stamina)
//   Space        - Jump
//   E            - Place block
//   Q            - Remove block
//   F3           - Toggle debug overlay
//   ESC          - Quit
//
class SurvivalMode {
private:
    // Core systems (owned by Engine, not here)
    World* world;
    WorldGenerator* worldGenerator;
    ChunkManager* chunkManager;
    Camera* camera;
    Input* input;
    Renderer* renderer;

    // Subsystems (owned by SurvivalMode)
    BlockPicker* blockPicker;
    FlyCamera* flyCamera;
    PlayerStats* playerStats;
    DayNightCycle* dayNightCycle;
    CraftingSystem* craftingSystem;
    CraftingGrid* craftingGrid;

    // State
    bool initialized;
    bool debugOverlayVisible;
    BlockTypeID selectedBlockType;

    // Player state
    Vec3 playerPosition;
    Vec3 spawnPoint;
    float playerYaw;
    float playerPitch;
    float moveSpeed;
    float sprintSpeed;
    float mouseSensitivity;

    // Day/night hunger modifier
    float nightHungerMultiplier;

    // Debug info
    int frameCount;
    float fpsTimer;
    float currentFps;

    // Terrain config
    int terrainChunkRadius;
    int terrainHeight;

    // Internal methods
    void GenerateFlatTerrain();
    void UpdateMovement(float deltaTime);
    void HandleBlockEditing();
    void RenderDebugOverlay();
    void UpdateDebugInfo(float deltaTime);
    void InitializeCraftingRecipes();
    void CheckPlayerDeath();
    void RespawnPlayer();

public:
    SurvivalMode(World* world, WorldGenerator* generator, ChunkManager* chunks,
                 Camera* camera, Input* input, Renderer* renderer);
    ~SurvivalMode();

    bool Initialize();
    void Shutdown();

    // Main lifecycle
    void Update(float deltaTime);
    void Render();

    // Terrain generation
    void GenerateTerrain(int chunkRadius, int height);
    void ClearTerrain();

    // Player
    void SpawnPlayer(const Vec3& position);
    Vec3 GetPlayerPosition() const { return playerPosition; }
    void SetPlayerPosition(const Vec3& pos);

    // Stats access
    PlayerStats* GetPlayerStats() const { return playerStats; }

    // Day/Night
    DayNightCycle* GetDayNightCycle() const { return dayNightCycle; }
    bool IsNight() const;

    // Crafting
    CraftingSystem* GetCraftingSystem() const { return craftingSystem; }
    bool CraftItem(const std::string& recipeID);
    std::vector<std::string> GetAvailableRecipes() const;

    // Block editing
    void SetSelectedBlock(BlockTypeID type);
    BlockTypeID GetSelectedBlock() const { return selectedBlockType; }
    bool PlaceBlockAt(const Vec3& position, const Vec3& normal);
    bool RemoveBlockAt(const Vec3& position);

    // Debug overlay
    void ToggleDebugOverlay();
    bool IsDebugOverlayVisible() const { return debugOverlayVisible; }
    void SetDebugOverlayVisible(bool visible);

    bool IsInitialized() const { return initialized; }
};

} // namespace vge
