#include "in_game_editor.h"
#include "voxel/world.h"
#include "voxel/block_registry.h"
#include "render/camera.h"
#include "platform/input.h"
#include "render/renderer.h"
#include "entity/entity_manager.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace vge {

// ============================================
// FlyCamera Implementation
// ============================================

FlyCamera::FlyCamera()
    : position(0, 40, 0), forward(0, 0, -1), up(0, 1, 0), right(1, 0, 0),
      yaw(-90.0f), pitch(0.0f), speed(10.0f), sensitivity(0.1f), enabled(false) {
    UpdateVectors();
}

FlyCamera::~FlyCamera() {}

void FlyCamera::UpdateVectors() {
    Vec3 newForward;
    newForward.x = std::cos(yaw * 3.14159f / 180.0f) * std::cos(pitch * 3.14159f / 180.0f);
    newForward.y = std::sin(pitch * 3.14159f / 180.0f);
    newForward.z = std::sin(yaw * 3.14159f / 180.0f) * std::cos(pitch * 3.14159f / 180.0f);
    forward = newForward.normalize();
    
    right = forward.cross(Vec3(0, 1, 0)).normalize();
    up = right.cross(forward).normalize();
}

void FlyCamera::Update(float deltaTime, Input& input) {
    if (!enabled) return;
    
    // Mouse look (using arrow keys for now - would use mouse delta in real implementation)
    if (input.IsKeyPressed(GLFW_KEY_UP)) pitch += sensitivity * 50.0f * deltaTime;
    if (input.IsKeyPressed(GLFW_KEY_DOWN)) pitch -= sensitivity * 50.0f * deltaTime;
    if (input.IsKeyPressed(GLFW_KEY_LEFT)) yaw -= sensitivity * 50.0f * deltaTime;
    if (input.IsKeyPressed(GLFW_KEY_RIGHT)) yaw += sensitivity * 50.0f * deltaTime;
    
    // Clamp pitch
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    UpdateVectors();
    
    // Movement
    float moveSpeed = speed;
    if (input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) moveSpeed *= 3.0f; // Sprint
    if (input.IsKeyPressed(GLFW_KEY_LEFT_CONTROL)) moveSpeed *= 0.3f; // Slow
    
    Vec3 moveDir(0, 0, 0);
    if (input.IsKeyPressed(GLFW_KEY_W)) moveDir = moveDir + forward;
    if (input.IsKeyPressed(GLFW_KEY_S)) moveDir = moveDir - forward;
    if (input.IsKeyPressed(GLFW_KEY_A)) moveDir = moveDir - right;
    if (input.IsKeyPressed(GLFW_KEY_D)) moveDir = moveDir + right;
    if (input.IsKeyPressed(GLFW_KEY_SPACE)) moveDir = moveDir + up;
    if (input.IsKeyPressed(GLFW_KEY_LEFT_ALT)) moveDir = moveDir - up;
    
    if (moveDir.length() > 0) {
        moveDir = moveDir.normalize() * moveSpeed * deltaTime;
        position = position + moveDir;
    }
}

void FlyCamera::SetPosition(const Vec3& pos) {
    position = pos;
}

void FlyCamera::SetRotation(float yawDeg, float pitchDeg) {
    yaw = yawDeg;
    pitch = pitchDeg;
    UpdateVectors();
}

Vec3 FlyCamera::ScreenToWorldRay(float screenX, float screenY, float screenWidth, float screenHeight) const {
    float x = (2.0f * screenX / screenWidth) - 1.0f;
    float y = 1.0f - (2.0f * screenY / screenHeight);
    
    Vec3 ray;
    ray.x = x;
    ray.y = y;
    ray.z = -1.0f;
    
    // Transform to world space
    Vec3 worldRay = forward + right * x + up * y;
    return worldRay.normalize();
}

void FlyCamera::LookAt(const Vec3& target) {
    Vec3 direction = (target - position).normalize();
    pitch = std::asin(direction.y) * 180.0f / 3.14159f;
    yaw = std::atan2(direction.z, direction.x) * 180.0f / 3.14159f;
    UpdateVectors();
}

// ============================================
// BlockPicker Implementation
// ============================================

BlockPicker::BlockPicker(World* world)
    : world(world), reachDistance(50.0f) {}

BlockPicker::~BlockPicker() {}

bool BlockPicker::PickBlock(const Vec3& origin, const Vec3& direction, Vec3& outPosition, Vec3& outNormal) {
    return PickBlock(origin, direction, outPosition, outNormal, reachDistance);
}

bool BlockPicker::PickBlock(const Vec3& origin, const Vec3& direction, Vec3& outPosition, Vec3& outNormal, float maxDistance) {
    if (!world) return false;
    
    // Simple raycast through grid
    Vec3 rayDir = direction.normalize();
    float step = 0.1f;
    float distance = 0.0f;
    
    Vec3 lastPos = origin;
    
    while (distance < maxDistance) {
        Vec3 currentPos = origin + rayDir * distance;
        int x = (int)std::floor(currentPos.x);
        int y = (int)std::floor(currentPos.y);
        int z = (int)std::floor(currentPos.z);
        
        BlockTypeID block = world->GetBlock(x, y, z);
        if (block != BLOCK_AIR && BlockRegistry::GetInstance().GetBlock(block).IsSolid()) {
            outPosition = Vec3((float)x, (float)y, (float)z);
            // Calculate normal based on which face was hit
            Vec3 diff = currentPos - lastPos;
            if (std::abs(diff.x) >= std::abs(diff.y) && std::abs(diff.x) >= std::abs(diff.z)) {
                outNormal = Vec3(diff.x > 0 ? -1.0f : 1.0f, 0, 0);
            } else if (std::abs(diff.y) >= std::abs(diff.x) && std::abs(diff.y) >= std::abs(diff.z)) {
                outNormal = Vec3(0, diff.y > 0 ? -1.0f : 1.0f, 0);
            } else {
                outNormal = Vec3(0, 0, diff.z > 0 ? -1.0f : 1.0f);
            }
            return true;
        }
        
        lastPos = currentPos;
        distance += step;
    }
    
    return false;
}

bool BlockPicker::PickEntity(const Vec3& origin, const Vec3& direction, EntitySpawner* spawner, uint32_t& outEntityId) {
    if (!spawner) return false;
    
    Vec3 rayDir = direction.normalize();
    float closestDist = reachDistance;
    bool found = false;
    
    for (const auto& entity : spawner->GetAllEntities()) {
        if (!entity.active) continue;
        
        // Simple distance check from ray to entity position
        Vec3 toEntity = entity.position - origin;
        float proj = toEntity.dot(rayDir);
        if (proj < 0 || proj > closestDist) continue;
        
        Vec3 closestPoint = origin + rayDir * proj;
        float dist = (closestPoint - entity.position).length();
        
        if (dist < 2.0f) { // Within 2 units
            closestDist = proj;
            outEntityId = entity.id;
            found = true;
        }
    }
    
    return found;
}

// ============================================
// InGameEditor Implementation
// ============================================

InGameEditor::InGameEditor(World* world, Camera* camera, Input* input, Renderer* renderer)
    : world(world), camera(camera), input(input), renderer(renderer), entityManager(nullptr),
      voxelEditor(nullptr), entitySpawner(nullptr), flyCamera(nullptr), blockPicker(nullptr),
      active(false), currentGizmoType(GizmoType::Translate),
      showBlockPicker(false), showEntitySpawner(false), showTerrainTools(false),
      showGizmos(true), showSelectionHighlight(true),
      selectedBlockType(BLOCK_AIR) {}

InGameEditor::~InGameEditor() {
    Shutdown();
}

bool InGameEditor::Initialize() {
    if (!world || !camera || !input || !renderer) {
        std::cerr << "[Editor] Missing required components" << std::endl;
        return false;
    }
    
    voxelEditor = new VoxelEditor(world);
    entitySpawner = new EntitySpawner();
    flyCamera = new FlyCamera();
    blockPicker = new BlockPicker(world);
    
    // Set default selected block
    selectedBlockType = BlockRegistry::GetInstance().GetBlockId("stone");
    if (selectedBlockType == BLOCK_AIR) {
        selectedBlockType = 1; // Fallback
    }
    voxelEditor->SetSelectedBlock(selectedBlockType);
    
    std::cout << "[Editor] Initialized" << std::endl;
    return true;
}

void InGameEditor::Shutdown() {
    delete blockPicker;
    delete flyCamera;
    delete entitySpawner;
    delete voxelEditor;
    blockPicker = nullptr;
    flyCamera = nullptr;
    entitySpawner = nullptr;
    voxelEditor = nullptr;
}

void InGameEditor::Update(float deltaTime) {
    if (!active) return;
    
    HandleInput(deltaTime);
    
    // Update fly camera
    flyCamera->Update(deltaTime, *input);
    
    // Update camera position from fly camera
    if (camera) {
        // Note: In real implementation, would update camera through renderer
    }
}

void InGameEditor::Render() {
    if (!active) return;
    
    if (showGizmos && gizmo.visible) {
        RenderGizmo();
    }
    
    if (showSelectionHighlight && selection.IsValid()) {
        RenderSelectionHighlight();
    }
    
    // Render UI panels
    if (showBlockPicker) RenderBlockPickerUI();
    if (showEntitySpawner) RenderEntitySpawnerUI();
    if (showTerrainTools) RenderTerrainToolsUI();
    
    RenderEditorUI();
}

void InGameEditor::Activate() {
    if (active) return;
    active = true;
    
    // Store current camera position
    if (camera) {
        flyCamera->SetPosition(camera->position);
    }
    
    if (onEditorActivated) onEditorActivated();
    std::cout << "[Editor] Activated" << std::endl;
}

void InGameEditor::Deactivate() {
    if (!active) return;
    active = false;
    
    if (onEditorDeactivated) onEditorDeactivated();
    std::cout << "[Editor] Deactivated" << std::endl;
}

void InGameEditor::SelectBlock(const Vec3& position, const Vec3& normal) {
    selection.type = EditorSelection::Type::Block;
    selection.blockPosition = position;
    selection.worldPosition = position;
    selection.normal = normal;
    selection.active = true;
    selection.entityId = 0;
    
    // Update highlight
    highlight.min = position;
    highlight.max = position + Vec3(1, 1, 1);
    highlight.visible = true;
    
    // Update gizmo position
    gizmo.position = position + Vec3(0.5f, 0.5f, 0.5f);
    gizmo.visible = true;
    
    if (onSelectionChanged) onSelectionChanged(selection);
}

void InGameEditor::SelectEntity(uint32_t entityId) {
    SpawnedEntity* entity = entitySpawner ? entitySpawner->GetEntity(entityId) : nullptr;
    if (!entity) return;
    
    selection.type = EditorSelection::Type::Entity;
    selection.entityId = entityId;
    selection.worldPosition = entity->position;
    selection.normal = Vec3(0, 1, 0);
    selection.active = true;
    
    // Update highlight
    highlight.min = entity->position - entity->scale * 0.5f;
    highlight.max = entity->position + entity->scale * 0.5f;
    highlight.visible = true;
    
    // Update gizmo position
    gizmo.position = entity->position;
    gizmo.visible = true;
    
    if (onSelectionChanged) onSelectionChanged(selection);
}

void InGameEditor::ClearSelection() {
    selection.Clear();
    highlight.visible = false;
    gizmo.visible = false;
}

void InGameEditor::SetGizmoType(GizmoType type) {
    currentGizmoType = type;
    gizmo.type = type;
}

void InGameEditor::SetSelectedBlockType(BlockTypeID type) {
    selectedBlockType = type;
    if (voxelEditor) {
        voxelEditor->SetSelectedBlock(type);
    }
}

void InGameEditor::PlaceBlockAt(const Vec3& position, const Vec3& normal) {
    if (!voxelEditor) return;
    voxelEditor->PlaceBlock(position, normal);
}

void InGameEditor::RemoveBlockAt(const Vec3& position) {
    if (!voxelEditor) return;
    voxelEditor->RemoveBlock(position);
}

void InGameEditor::PaintBlockAt(const Vec3& position) {
    if (!voxelEditor) return;
    voxelEditor->PaintBlock(position);
}

uint32_t InGameEditor::SpawnEntity(const std::string& type, const Vec3& position) {
    if (!entitySpawner) return 0;
    return entitySpawner->SpawnEntity(type, position);
}

bool InGameEditor::RemoveSelectedEntity() {
    if (selection.type != EditorSelection::Type::Entity || !entitySpawner) {
        return false;
    }
    bool result = entitySpawner->RemoveEntity(selection.entityId);
    if (result) {
        ClearSelection();
    }
    return result;
}

bool InGameEditor::DuplicateSelectedEntity() {
    if (selection.type != EditorSelection::Type::Entity || !entitySpawner) {
        return false;
    }
    
    SpawnedEntity* entity = entitySpawner->GetEntity(selection.entityId);
    if (!entity) return false;
    
    Vec3 newPos = entity->position + Vec3(1, 0, 0);
    uint32_t newId = entitySpawner->SpawnEntity(entity->type, newPos, entity->rotation, entity->scale);
    SelectEntity(newId);
    return true;
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
    voxelEditor->FlattenTerrain(position, targetHeight);
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

void InGameEditor::HandleInput(float deltaTime) {
    if (!input) return;
    
    HandleKeyboardInput(deltaTime);
    HandleMouseInput(deltaTime);
}

void InGameEditor::HandleKeyboardInput(float deltaTime) {
    (void)deltaTime;
    
    // Toggle editor
    if (input->IsKeyJustPressed(GLFW_KEY_F3)) {
        Toggle();
        return;
    }
    
    if (!active) return;
    
    // Undo/Redo
    if (input->IsKeyPressed(GLFW_KEY_LEFT_CONTROL) && input->IsKeyJustPressed(GLFW_KEY_Z)) {
        Undo();
    }
    if (input->IsKeyPressed(GLFW_KEY_LEFT_CONTROL) && input->IsKeyJustPressed(GLFW_KEY_Y)) {
        Redo();
    }
    
    // Gizmo type switching
    if (input->IsKeyJustPressed(GLFW_KEY_1)) SetGizmoType(GizmoType::Translate);
    if (input->IsKeyJustPressed(GLFW_KEY_2)) SetGizmoType(GizmoType::Rotate);
    if (input->IsKeyJustPressed(GLFW_KEY_3)) SetGizmoType(GizmoType::Scale);
    
    // Tool switching
    if (input->IsKeyJustPressed(GLFW_KEY_4)) {
        showBlockPicker = !showBlockPicker;
        showEntitySpawner = false;
        showTerrainTools = false;
    }
    if (input->IsKeyJustPressed(GLFW_KEY_5)) {
        showEntitySpawner = !showEntitySpawner;
        showBlockPicker = false;
        showTerrainTools = false;
    }
    if (input->IsKeyJustPressed(GLFW_KEY_6)) {
        showTerrainTools = !showTerrainTools;
        showBlockPicker = false;
        showEntitySpawner = false;
    }
    
    // Delete selected
    if (input->IsKeyJustPressed(GLFW_KEY_DELETE)) {
        if (selection.type == EditorSelection::Type::Block) {
            RemoveBlockAt(selection.blockPosition);
            ClearSelection();
        } else if (selection.type == EditorSelection::Type::Entity) {
            RemoveSelectedEntity();
        }
    }
    
    // Duplicate
    if (input->IsKeyPressed(GLFW_KEY_LEFT_CONTROL) && input->IsKeyJustPressed(GLFW_KEY_D)) {
        DuplicateSelectedEntity();
    }
    
    // Toggle gizmos
    if (input->IsKeyJustPressed(GLFW_KEY_G)) {
        showGizmos = !showGizmos;
    }
    
    // Clear selection
    if (input->IsKeyJustPressed(GLFW_KEY_ESCAPE)) {
        ClearSelection();
    }
}

void InGameEditor::HandleMouseInput(float deltaTime) {
    (void)deltaTime;
    
    if (!active || !input) return;
    
    // Left click - select or place
    if (input->IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        Vec3 position, normal;
        
        if (RaycastWorld(position, normal)) {
            // Check if we hit an entity first
            uint32_t entityId;
            Vec3 ray = GetMouseRay();
            Vec3 camPos = flyCamera->GetPosition();
            
            if (blockPicker && blockPicker->PickEntity(camPos, ray, entitySpawner, entityId)) {
                SelectEntity(entityId);
            } else {
                // Hit block
                if (input->IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
                    // Shift+click = remove block
                    RemoveBlockAt(position);
                } else if (input->IsKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
                    // Ctrl+click = paint block
                    PaintBlockAt(position);
                } else {
                    // Normal click = select block
                    SelectBlock(position, normal);
                }
            }
        } else {
            ClearSelection();
        }
    }
    
    // Right click - place block
    if (input->IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        Vec3 position, normal;
        
        if (RaycastWorld(position, normal)) {
            PlaceBlockAt(position, normal);
        }
    }
    
    // Middle click - pick block type
    if (input->IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        Vec3 position, normal;
        
        if (RaycastWorld(position, normal)) {
            BlockTypeID block = world->GetBlock(
                (int)std::floor(position.x),
                (int)std::floor(position.y),
                (int)std::floor(position.z)
            );
            if (block != BLOCK_AIR) {
                SetSelectedBlockType(block);
            }
        }
    }
}

Vec3 InGameEditor::GetMouseRay() const {
    if (!flyCamera) return Vec3(0, 0, -1);
    return flyCamera->GetForward();
}

bool InGameEditor::RaycastWorld(Vec3& outPosition, Vec3& outNormal) {
    if (!blockPicker || !flyCamera) return false;
    
    Vec3 ray = GetMouseRay();
    Vec3 origin = flyCamera->GetPosition();
    
    return blockPicker->PickBlock(origin, ray, outPosition, outNormal);
}

void InGameEditor::RenderGizmo() {
    // Stub - would render translation/rotation/scale gizmo
    // This requires actual rendering implementation with lines/meshes
}

void InGameEditor::RenderSelectionHighlight() {
    // Stub - would render wireframe box around selection
    // This requires actual rendering implementation with lines
}

void InGameEditor::RenderBlockPickerUI() {
    // Stub - would render ImGui panel with block grid
}

void InGameEditor::RenderEntitySpawnerUI() {
    // Stub - would render ImGui panel with entity types
}

void InGameEditor::RenderTerrainToolsUI() {
    // Stub - would render ImGui panel with terrain brush settings
}

void InGameEditor::RenderEditorUI() {
    // Stub - would render main editor toolbar and info
}

} // namespace vge
