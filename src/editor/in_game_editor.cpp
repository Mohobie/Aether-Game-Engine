#include "in_game_editor.h"
#include "voxel/voxel_editor.h"
#include "rendering/camera.h"
#include "platform/input_manager.h"
#include "entity/entity.h"
#include "core/logger.h"
#include "debug/debug_renderer.h"
#include <cmath>

namespace vge {

// ============================================
// InGameEditor Implementation
// ============================================

// Simple log stub for compilation
static void LogInfo(const char* msg) { (void)msg; }

InGameEditor::InGameEditor(World* world, Camera* camera, Input* input, Renderer* renderer)
    : world(world), camera(camera), input(input), renderer(renderer),
      entityManager(nullptr), voxelEditor(nullptr), entitySpawner(nullptr),
      flyCamera(nullptr), blockPicker(nullptr), active(false),
      currentGizmoType(GizmoType::Translate), showBlockPicker(false),
      showEntitySpawner(false), showTerrainTools(false), showGizmos(true),
      showSelectionHighlight(true), selectedBlockType(BLOCK_AIR) {}

InGameEditor::~InGameEditor() {
    Shutdown();
}

bool InGameEditor::Initialize() {
    voxelEditor = new VoxelEditor(world);
    entitySpawner = new EntitySpawner();
    flyCamera = new FlyCamera();
    blockPicker = new BlockPicker(world);
    
    LogInfo("[Editor] Initialized");
    return true;
}

void InGameEditor::Shutdown() {
    delete voxelEditor;
    delete entitySpawner;
    delete flyCamera;
    delete blockPicker;
    
    voxelEditor = nullptr;
    entitySpawner = nullptr;
    flyCamera = nullptr;
    blockPicker = nullptr;
}

void InGameEditor::Update(float deltaTime, Input& editorInput) {
    if (!active) return;
    
    if (flyCamera) {
        flyCamera->Update(deltaTime, editorInput);
    }
    
    HandleInput(deltaTime);
}

void InGameEditor::Render() {
    if (!active) return;
    
    if (showGizmos) {
        RenderGizmo();
    }
    
    if (showSelectionHighlight) {
        RenderSelectionHighlight();
    }
    
    RenderEditorUI();
}

void InGameEditor::Activate() {
    active = true;
    
    if (onEditorActivated) {
        onEditorActivated();
    }
    
    LogInfo("[Editor] Activated");
}

void InGameEditor::Deactivate() {
    active = false;
    
    if (onEditorDeactivated) {
        onEditorDeactivated();
    }
    
    LogInfo("[Editor] Deactivated");
}

void InGameEditor::SelectBlock(const Vec3& position, const Vec3& normal) {
    selection.type = EditorSelection::Type::Block;
    selection.blockPosition = position;
    selection.normal = normal;
    selection.worldPosition = position;
    selection.active = true;
    
    if (onSelectionChanged) {
        onSelectionChanged(selection);
    }
}

void InGameEditor::SelectEntity(uint32_t entityId) {
    selection.type = EditorSelection::Type::Entity;
    selection.entityId = entityId;
    selection.active = true;
    
    if (onSelectionChanged) {
        onSelectionChanged(selection);
    }
}

void InGameEditor::ClearSelection() {
    selection.Clear();
    
    if (onSelectionChanged) {
        onSelectionChanged(selection);
    }
}

void InGameEditor::SetGizmoType(GizmoType type) {
    currentGizmoType = type;
}

void InGameEditor::SetSelectedBlockType(BlockTypeID type) {
    selectedBlockType = type;
}

void InGameEditor::PlaceBlockAt(const Vec3& position, const Vec3& normal) {
    if (!voxelEditor) return;
    
    Vec3 placePos = position + normal * 0.5f;
    voxelEditor->ApplyToolAt(placePos);
}

void InGameEditor::RemoveBlockAt(const Vec3& position) {
    if (!voxelEditor) return;
    
    voxelEditor->SetTool(VoxelToolType::Remove);
    voxelEditor->ApplyToolAt(position);
    voxelEditor->SetTool(VoxelToolType::Place);
}

void InGameEditor::PaintBlockAt(const Vec3& position) {
    if (!voxelEditor) return;
    
    voxelEditor->SetTool(VoxelToolType::Paint);
    voxelEditor->ApplyToolAt(position);
    voxelEditor->SetTool(VoxelToolType::Place);
}

uint32_t InGameEditor::SpawnEntity(const std::string& type, const Vec3& position) {
    if (!entitySpawner) return 0;
    
    return entitySpawner->SpawnEntity(type, position);
}

bool InGameEditor::RemoveSelectedEntity() {
    if (!selection.IsValid() || selection.type != EditorSelection::Type::Entity) {
        return false;
    }
    
    if (entitySpawner) {
        entitySpawner->RemoveEntity(selection.entityId);
        ClearSelection();
        return true;
    }
    
    return false;
}

bool InGameEditor::DuplicateSelectedEntity() {
    if (!selection.IsValid() || selection.type != EditorSelection::Type::Entity) {
        return false;
    }
    
    if (entitySpawner) {
        Vec3 offset(1.0f, 0.0f, 0.0f);
        // Get current entity position and spawn new one
        SpawnedEntity* entity = entitySpawner->GetEntity(selection.entityId);
        if (entity) {
            return entitySpawner->SpawnEntity(entity->type, entity->position + offset) != 0;
        }
    }
    
    return false;
}

void InGameEditor::SmoothTerrainAt(const Vec3& position) {
    if (!voxelEditor) return;
    
    voxelEditor->SetTool(VoxelToolType::Smooth);
    voxelEditor->ApplyToolAt(position);
}

void InGameEditor::RaiseTerrainAt(const Vec3& position) {
    if (!voxelEditor) return;
    
    voxelEditor->SetTool(VoxelToolType::Raise);
    voxelEditor->ApplyToolAt(position);
}

void InGameEditor::LowerTerrainAt(const Vec3& position) {
    if (!voxelEditor) return;
    
    voxelEditor->SetTool(VoxelToolType::Lower);
    voxelEditor->ApplyToolAt(position);
}

void InGameEditor::FlattenTerrainAt(const Vec3& position, float targetHeight) {
    if (!voxelEditor) return;
    
    voxelEditor->SetTool(VoxelToolType::Flatten);
    voxelEditor->ApplyToolAt(position);
}

bool InGameEditor::Undo() {
    if (!voxelEditor) return false;
    return voxelEditor->Undo();
}

bool InGameEditor::Redo() {
    if (!voxelEditor) return false;
    return voxelEditor->Redo();
}

bool InGameEditor::CanUndo() const {
    if (!voxelEditor) return false;
    return voxelEditor->CanUndo();
}

bool InGameEditor::CanRedo() const {
    if (!voxelEditor) return false;
    return voxelEditor->CanRedo();
}

void InGameEditor::SetOnEditorActivated(std::function<void()> callback) {
    onEditorActivated = callback;
}

void InGameEditor::SetOnEditorDeactivated(std::function<void()> callback) {
    onEditorDeactivated = callback;
}

void InGameEditor::SetOnSelectionChanged(std::function<void(const EditorSelection&)> callback) {
    onSelectionChanged = callback;
}

// ============================================
// Input Handling
// ============================================

void InGameEditor::HandleInput(float deltaTime) {
    HandleMouseInput(deltaTime);
    HandleKeyboardInput(deltaTime);
}

void InGameEditor::HandleMouseInput(float deltaTime) {
    (void)deltaTime;
    
    if (!input) return;
    
    // Note: Mouse button support would need to be added to Input class
    // For now, use keyboard alternatives
    
    // Place block with Enter key
    if (input->IsKeyJustPressed(KeyCode::Enter)) {
        Vec3 position, normal;
        
        if (RaycastWorld(position, normal)) {
            if (currentGizmoType == GizmoType::Translate) {
                SelectBlock(position, normal);
            } else {
                PlaceBlockAt(position, normal);
            }
        }
    }
    
    // Remove block with Escape
    if (input->IsKeyJustPressed(KeyCode::Escape)) {
        Vec3 position, normal;
        
        if (RaycastWorld(position, normal)) {
            RemoveBlockAt(position);
        }
    }
}

void InGameEditor::HandleKeyboardInput(float deltaTime) {
    (void)deltaTime;
    
    if (!input) return;
    
    // Undo - using Q for now
    if (input->IsKeyJustPressed(KeyCode::Q)) {
        Undo();
    }
    
    // Redo - using E for now
    if (input->IsKeyJustPressed(KeyCode::E)) {
        Redo();
    }
    
    // Toggle gizmos
    if (input->IsKeyJustPressed(KeyCode::Shift)) {
        showGizmos = !showGizmos;
    }
}

// ============================================
// Raycasting
// ============================================

Vec3 InGameEditor::GetMouseRay() const {
    if (!camera) return Vec3(0, 0, 1);
    
    return camera->GetForward();
}

bool InGameEditor::RaycastWorld(Vec3& outPosition, Vec3& outNormal) {
    if (!camera || !blockPicker) return false;
    
    Vec3 origin = camera->GetPosition();
    Vec3 direction = GetMouseRay();
    
    return blockPicker->PickBlock(origin, direction, outPosition, outNormal);
}

// ============================================
// Rendering
// ============================================

void InGameEditor::RenderGizmo() {
    if (!selection.IsValid()) return;
    
    DebugRenderer& debug = GetDebugRenderer();
    
    Vec3 gizmoPos = selection.worldPosition;
    float gizmoSize = 1.0f;
    
    switch (currentGizmoType) {
        case GizmoType::Translate:
            debug.DrawGizmoTranslate(gizmoPos, gizmoSize);
            break;
        case GizmoType::Rotate:
            debug.DrawGizmoRotate(gizmoPos, gizmoSize);
            break;
        case GizmoType::Scale:
            debug.DrawGizmoScale(gizmoPos, gizmoSize);
            break;
        default:
            break;
    }
}

void InGameEditor::RenderSelectionHighlight() {
    if (!selection.IsValid()) return;
    
    DebugRenderer& debug = GetDebugRenderer();
    Vec3 color(1.0f, 0.8f, 0.0f); // Orange selection color
    
    switch (selection.type) {
        case EditorSelection::Type::Block: {
            Vec3 min = selection.blockPosition;
            Vec3 max = selection.blockPosition + Vec3(1, 1, 1);
            debug.DrawSelectionBox(min, max, color);
            break;
        }
        case EditorSelection::Type::Entity: {
            // Draw cross at entity position
            debug.DrawCross(selection.worldPosition, 0.5f, color);
            break;
        }
        default:
            break;
    }
}

void InGameEditor::RenderBlockPickerUI() {
    // Block picker is now handled by ImGui in imgui_wrapper.cpp
    // This stub remains for non-ImGui fallback
}

void InGameEditor::RenderEntitySpawnerUI() {
    // Entity spawner is now handled by ImGui in imgui_wrapper.cpp
    // This stub remains for non-ImGui fallback
}

void InGameEditor::RenderTerrainToolsUI() {
    // Terrain tools are now handled by ImGui in imgui_wrapper.cpp
    // This stub remains for non-ImGui fallback
}

void InGameEditor::RenderEditorUI() {
    // Main editor UI is now handled by ImGui in imgui_wrapper.cpp
    // This stub remains for non-ImGui fallback
}

// ============================================
// FlyCamera Implementation
// ============================================

FlyCamera::FlyCamera()
    : position(0, 0, 0), forward(0, 0, 1), up(0, 1, 0), right(1, 0, 0),
      yaw(-90.0f), pitch(0.0f), speed(5.0f), sensitivity(0.1f), enabled(true) {}

FlyCamera::~FlyCamera() {}

void FlyCamera::UpdateVectors() {
    float yawRad = yaw * 3.14159f / 180.0f;
    float pitchRad = pitch * 3.14159f / 180.0f;

    forward.x = std::cos(pitchRad) * std::cos(yawRad);
    forward.y = std::sin(pitchRad);
    forward.z = std::cos(pitchRad) * std::sin(yawRad);
    forward = forward.normalize();

    right = forward.cross(Vec3(0, 1, 0)).normalize();
    up = right.cross(forward).normalize();
}

void FlyCamera::Update(float deltaTime, Input& input) {
    if (!enabled) return;

    // --- Mouse look (yaw/pitch) ---
    float dx, dy;
    input.GetMouseDelta(dx, dy);
    yaw   += dx * sensitivity;
    pitch += dy * sensitivity;

    // Clamp pitch to avoid gimbal lock
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    UpdateVectors();

    // --- Scroll wheel adjusts speed ---
    float scroll = input.GetScrollDelta();
    if (scroll != 0.0f) {
        speed += scroll * 2.0f;
        if (speed < 0.5f)  speed = 0.5f;
        if (speed > 100.0f) speed = 100.0f;
    }

    // --- WASD movement relative to look direction ---
    Vec3 moveDir(0, 0, 0);

    if (input.IsKeyPressed(static_cast<int>(KeyCode::W))) {
        moveDir = moveDir + forward;
    }
    if (input.IsKeyPressed(static_cast<int>(KeyCode::S))) {
        moveDir = moveDir - forward;
    }
    if (input.IsKeyPressed(static_cast<int>(KeyCode::A))) {
        moveDir = moveDir - right;
    }
    if (input.IsKeyPressed(static_cast<int>(KeyCode::D))) {
        moveDir = moveDir + right;
    }

    // --- Fly mode: Space = up, Shift = down ---
    if (input.IsKeyPressed(static_cast<int>(KeyCode::Space))) {
        moveDir = moveDir + Vec3(0, 1, 0);
    }
    if (input.IsKeyPressed(static_cast<int>(KeyCode::Shift))) {
        moveDir = moveDir - Vec3(0, 1, 0);
    }

    // Normalize horizontal movement so diagonal isn't faster
    if (moveDir.length() > 0.001f) {
        moveDir = moveDir.normalize();
        position = position + moveDir * (speed * deltaTime);
    }
}

void FlyCamera::SetPosition(const Vec3& pos) { position = pos; }
void FlyCamera::SetRotation(float yawDeg, float pitchDeg) {
    yaw = yawDeg;
    pitch = pitchDeg;
    UpdateVectors();
}
void FlyCamera::LookAt(const Vec3& target) {
    Vec3 direction = target - position;
    yaw = atan2f(direction.z, direction.x) * 180.0f / 3.14159f;
    pitch = asinf(direction.y / direction.length()) * 180.0f / 3.14159f;
    UpdateVectors();
}

// ============================================
// BlockPicker Implementation
// ============================================

BlockPicker::BlockPicker(World* w) : world(w), reachDistance(5.0f) {}
BlockPicker::~BlockPicker() {}
bool BlockPicker::PickBlock(const Vec3& origin, const Vec3& direction, Vec3& outPosition, Vec3& outNormal) {
    return PickBlock(origin, direction, outPosition, outNormal, reachDistance);
}
bool BlockPicker::PickBlock(const Vec3& origin, const Vec3& direction, Vec3& outPosition, Vec3& outNormal, float maxDistance) {
    Vec3 dir = direction;
    if (dir.length() > 0.001f) {
        dir = dir.normalize();
    }
    Vec3 pos = origin;
    
    for (float dist = 0.0f; dist < maxDistance; dist += 0.1f) {
        int x = (int)std::floor(pos.x);
        int y = (int)std::floor(pos.y);
        int z = (int)std::floor(pos.z);
        
        BlockTypeID block = BLOCK_AIR;
        if (world) {
            block = world->GetBlock(x, y, z);
        }
        
        if (block != BLOCK_AIR) {
            outPosition = Vec3((float)x, (float)y, (float)z);
            outNormal = dir * -1.0f;
            return true;
        }
        
        pos = pos + dir * 0.1f;
    }
    
    return false;
}
bool BlockPicker::PickEntity(const Vec3& origin, const Vec3& direction, EntitySpawner* spawner, uint32_t& outEntityId) {
    (void)origin; (void)direction; (void)spawner; (void)outEntityId;
    return false;
}

void LogInfo(const char* fmt, ...) {
    (void)fmt;
}

} // namespace vge
