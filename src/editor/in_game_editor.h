#pragma once
#include "math/vec3.h"
#include "voxel/block_types.h"
#include "editor/entity_spawner.h"
#include "voxel/voxel_editor.h"
#include <vector>
#include <string>
#include <functional>

namespace vge {

// Forward declarations
class World;
class Camera;
class Input;
class Renderer;
class EntityManager;

// ============================================
// Voxel Raycast Hit
// ============================================
struct VoxelRaycastHit {
    Vec3 position;
    Vec3 normal;
    BlockTypeID blockType;
    float distance;
};

// ============================================
// AABB
// ============================================
struct AABB {
    Vec3 min;
    Vec3 max;
    
    AABB() : min(0, 0, 0), max(0, 0, 0) {}
    AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}
    
    bool Contains(const Vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
    
    bool Intersects(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }
};
// ============================================
enum class GizmoType {
    None,
    Translate,
    Rotate,
    Scale
};

// ============================================
// Gizmo Axis
// ============================================
enum class GizmoAxis {
    None,
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ,
    XYZ
};

// ============================================
// Editor Selection
// ============================================
struct EditorSelection {
    enum class Type { None, Block, Entity };
    
    Type type = Type::None;
    Vec3 blockPosition;           // For block selection
    uint32_t entityId = 0;      // For entity selection
    Vec3 worldPosition;           // Hit point in world
    Vec3 normal;                  // Surface normal
    bool active = false;
    
    void Clear() { type = Type::None; active = false; entityId = 0; }
    bool IsValid() const { return active; }
};

// ============================================
// Selection Highlight
// ============================================
struct SelectionHighlight {
    Vec3 min;
    Vec3 max;
    Vec3 color;
    float lineWidth;
    bool visible;
    
    SelectionHighlight() : color(1.0f, 0.8f, 0.0f), lineWidth(2.0f), visible(false) {}
};

// ============================================
// Gizmo Visual
// ============================================
struct GizmoVisual {
    GizmoType type;
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    float size;
    bool visible;
    GizmoAxis hoveredAxis;
    GizmoAxis activeAxis;
    
    GizmoVisual() : type(GizmoType::None), size(1.0f), visible(false),
                    hoveredAxis(GizmoAxis::None), activeAxis(GizmoAxis::None) {}
};

// ============================================
// Fly Camera Controller
// ============================================
class FlyCamera {
private:
    Vec3 position;
    Vec3 forward;
    Vec3 up;
    Vec3 right;
    float yaw;
    float pitch;
    float speed;
    float sensitivity;
    bool enabled;
    
    void UpdateVectors();
    
public:
    FlyCamera();
    ~FlyCamera();
    
    void Update(float deltaTime, Input& input);
    
    // Getters
    Vec3 GetPosition() const { return position; }
    Vec3 GetForward() const { return forward; }
    Vec3 GetUp() const { return up; }
    Vec3 GetRight() const { return right; }
    float GetYaw() const { return yaw; }
    float GetPitch() const { return pitch; }
    
    // Setters
    void SetPosition(const Vec3& pos);
    void SetRotation(float yawDeg, float pitchDeg);
    
    // Look at target
    void LookAt(const Vec3& target);
    void SetSpeed(float s) { speed = s; }
    float GetSpeed() const { return speed; }
    void SetSensitivity(float s) { sensitivity = s; }
    
    // Enable/disable
    void Enable() { enabled = true; }
    void Disable() { enabled = false; }
    bool IsEnabled() const { return enabled; }
    
    // Ray from camera through screen point
    Vec3 ScreenToWorldRay(float screenX, float screenY, float screenWidth, float screenHeight) const;
};

// ============================================
// Block Picker
// ============================================
class BlockPicker {
private:
    World* world;
    float reachDistance;
    
public:
    BlockPicker(World* world);
    ~BlockPicker();
    
    // Raycast from camera to find block
    bool PickBlock(const Vec3& origin, const Vec3& direction, Vec3& outPosition, Vec3& outNormal);
    bool PickBlock(const Vec3& origin, const Vec3& direction, Vec3& outPosition, Vec3& outNormal, float maxDistance);
    
    // Pick entity
    bool PickEntity(const Vec3& origin, const Vec3& direction, EntitySpawner* spawner, uint32_t& outEntityId);
    
    // Set reach distance
    void SetReachDistance(float distance) { reachDistance = distance; }
    float GetReachDistance() const { return reachDistance; }
};

// ============================================
// In-Game Editor
// ============================================
class InGameEditor {
private:
    World* world;
    Camera* camera;
    Input* input;
    Renderer* renderer;
    EntityManager* entityManager;
    
    // Subsystems
    VoxelEditor* voxelEditor;
    EntitySpawner* entitySpawner;
    FlyCamera* flyCamera;
    BlockPicker* blockPicker;
    
    // State
    bool active;
    EditorSelection selection;
    SelectionHighlight highlight;
    GizmoVisual gizmo;
    GizmoType currentGizmoType;
    
    // UI State
    bool showBlockPicker;
    bool showEntitySpawner;
    bool showTerrainTools;
    bool showGizmos;
    bool showSelectionHighlight;
    
    // Block picker state
    BlockTypeID selectedBlockType;
    std::string selectedBlockId;
    
    // Callbacks
    std::function<void()> onEditorActivated;
    std::function<void()> onEditorDeactivated;
    std::function<void(const EditorSelection&)> onSelectionChanged;
    
public:
    InGameEditor(World* world, Camera* camera, Input* input, Renderer* renderer);
    ~InGameEditor();
    
    bool Initialize();
    void Shutdown();
    
    // Main update
    void Update(float deltaTime, Input& input);
    void Render();
    
    // Activation
    void Activate();
    void Deactivate();
    bool IsActive() const { return active; }
    void Toggle() { if (active) Deactivate(); else Activate(); }
    
    // Selection
    void SelectBlock(const Vec3& position, const Vec3& normal);
    void SelectEntity(uint32_t entityId);
    void ClearSelection();
    const EditorSelection& GetSelection() const { return selection; }
    
    // Gizmos
    void SetGizmoType(GizmoType type);
    GizmoType GetGizmoType() const { return currentGizmoType; }
    void ShowGizmos(bool show) { showGizmos = show; }
    bool AreGizmosVisible() const { return showGizmos; }
    
    // Selection highlight
    void ShowSelectionHighlight(bool show) { showSelectionHighlight = show; }
    bool IsSelectionHighlightVisible() const { return showSelectionHighlight; }
    
    // Block operations
    void SetSelectedBlockType(BlockTypeID type);
    BlockTypeID GetSelectedBlockType() const { return selectedBlockType; }
    void PlaceBlockAt(const Vec3& position, const Vec3& normal);
    void RemoveBlockAt(const Vec3& position);
    void PaintBlockAt(const Vec3& position);
    
    // Entity operations
    uint32_t SpawnEntity(const std::string& type, const Vec3& position);
    bool RemoveSelectedEntity();
    bool DuplicateSelectedEntity();
    
    // Terrain tools
    void SmoothTerrainAt(const Vec3& position);
    void RaiseTerrainAt(const Vec3& position);
    void LowerTerrainAt(const Vec3& position);
    void FlattenTerrainAt(const Vec3& position, float targetHeight);
    
    // Undo/Redo
    bool Undo();
    bool Redo();
    bool CanUndo() const;
    bool CanRedo() const;
    
    // UI panels
    void ShowBlockPicker(bool show) { showBlockPicker = show; }
    void ShowEntitySpawner(bool show) { showEntitySpawner = show; }
    void ShowTerrainTools(bool show) { showTerrainTools = show; }
    bool IsBlockPickerVisible() const { return showBlockPicker; }
    bool IsEntitySpawnerVisible() const { return showEntitySpawner; }
    bool IsTerrainToolsVisible() const { return showTerrainTools; }
    
    // Getters for subsystems
    VoxelEditor* GetVoxelEditor() { return voxelEditor; }
    EntitySpawner* GetEntitySpawner() { return entitySpawner; }
    FlyCamera* GetFlyCamera() { return flyCamera; }
    BlockPicker* GetBlockPicker() { return blockPicker; }
    
    // Callbacks
    void SetOnEditorActivated(std::function<void()> callback);
    void SetOnEditorDeactivated(std::function<void()> callback);
    void SetOnSelectionChanged(std::function<void(const EditorSelection&)> callback);
    
    // Input handling
    void HandleInput(float deltaTime);
    void HandleMouseInput(float deltaTime);
    void HandleKeyboardInput(float deltaTime);
    
    // Raycasting
    Vec3 GetMouseRay() const;
    bool RaycastWorld(Vec3& outPosition, Vec3& outNormal);
    
    // Render helpers
    void RenderGizmo();
    void RenderSelectionHighlight();
    void RenderBlockPickerUI();
    void RenderEntitySpawnerUI();
    void RenderTerrainToolsUI();
    void RenderEditorUI();
};

// ============================================
// PlayerController (stub for console commands)
// ============================================
class PlayerController {
private:
    Vec3 position;
    float yaw;
    float pitch;
    float health;
    float maxHealth;
    float speed;
    bool noclip;
    bool godMode;
    bool flying;
    
public:
    PlayerController() : position(0, 0, 0), yaw(0), pitch(0), 
                         health(100), maxHealth(100), speed(5.0f),
                         noclip(false), godMode(false), flying(false) {}
    
    Vec3 GetPosition() const { return position; }
    void SetPosition(const Vec3& pos) { position = pos; }
    float GetYaw() const { return yaw; }
    void SetYaw(float y) { yaw = y; }
    float GetPitch() const { return pitch; }
    void SetPitch(float p) { pitch = p; }
    float GetHealth() const { return health; }
    void SetHealth(float h) { health = h; }
    float GetMaxHealth() const { return maxHealth; }
    float GetSpeed() const { return speed; }
    void SetSpeed(float s) { speed = s; }
    bool IsNoclip() const { return noclip; }
    void SetNoclip(bool n) { noclip = n; }
    bool IsGodMode() const { return godMode; }
    void SetGodMode(bool g) { godMode = g; }
    bool IsFlying() const { return flying; }
    void SetFlying(bool f) { flying = f; }
};

} // namespace vge
