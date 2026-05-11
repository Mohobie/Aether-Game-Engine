#pragma once
#include "math/vec3.h"
#include "voxel/block_types.h"
#include <cstring>
#include <memory>

namespace vge {

// Forward declarations
class World;
class WorldGenerator;
class ChunkManager;
class Camera;
class Input;
class Renderer;
class VoxelEditor;
class InGameEditor;
class BlockPicker;
class FlyCamera;

// ============================================
// Sandbox Game Mode - Creative Building
// ============================================
// Ties together world generation, fly camera,
// and block editing for a creative mode experience.
//
// Controls:
//   WASD         - Fly movement
//   Arrow Keys   - Look around
//   Space/Shift  - Fly up/down
//   E            - Place block
//   Q            - Remove block
//   F1           - Toggle editor mode
//   F3           - Toggle debug overlay
//   ESC          - Quit
//
class SandboxMode {
private:
    // Core systems (owned by Engine, not here)
    World* world;
    WorldGenerator* worldGenerator;
    ChunkManager* chunkManager;
    Camera* camera;
    Input* input;
    Renderer* renderer;

    // Subsystems (owned by SandboxMode)
    VoxelEditor* voxelEditor;
    InGameEditor* inGameEditor;
    BlockPicker* blockPicker;
    FlyCamera* flyCamera;

    // State
    bool initialized;
    bool editorModeActive;
    bool debugOverlayVisible;
    BlockTypeID selectedBlockType;

    // Player state
    Vec3 playerPosition;
    float playerYaw;
    float playerPitch;
    float flySpeed;
    float mouseSensitivity;

    // Debug info
    int frameCount;
    float fpsTimer;
    float currentFps;

    // Terrain config
    int terrainChunkRadius;
    int terrainHeight;

    // Internal methods
    void GenerateFlatTerrain();
    void UpdateFlyCamera(float deltaTime);
    void HandleBlockEditing();
    void RenderDebugOverlay();
    void UpdateDebugInfo(float deltaTime);

public:
    SandboxMode(World* world, WorldGenerator* generator, ChunkManager* chunks,
                Camera* camera, Input* input, Renderer* renderer);
    ~SandboxMode();

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

    // Block editing
    void SetSelectedBlock(BlockTypeID type);
    BlockTypeID GetSelectedBlock() const { return selectedBlockType; }
    bool PlaceBlockAt(const Vec3& position, const Vec3& normal);
    bool RemoveBlockAt(const Vec3& position);

    // Editor mode
    void ToggleEditorMode();
    bool IsEditorModeActive() const { return editorModeActive; }
    void SetEditorModeActive(bool active);

    // Debug overlay
    void ToggleDebugOverlay();
    bool IsDebugOverlayVisible() const { return debugOverlayVisible; }
    void SetDebugOverlayVisible(bool visible);

    // Fly camera
    void SetFlySpeed(float speed) { flySpeed = speed; }
    float GetFlySpeed() const { return flySpeed; }

    // Getters for subsystems
    VoxelEditor* GetVoxelEditor() const { return voxelEditor; }
    InGameEditor* GetInGameEditor() const { return inGameEditor; }
    BlockPicker* GetBlockPicker() const { return blockPicker; }
    FlyCamera* GetFlyCamera() const { return flyCamera; }

    bool IsInitialized() const { return initialized; }
};

} // namespace vge
