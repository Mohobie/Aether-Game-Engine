#include "ui/imgui_wrapper.h"
#include "platform/window.h"
#include "core/logger.h"
#include "rendering/camera.h"
#include "entity/entity.h"
// #include "entity/entity_manager.h"  // Not yet implemented
#include "voxel/world.h"
#include "voxel/block_registry.h"
#include "voxel/voxel_editor.h"
#include "ai/entity_ai.h"
#include "debug/debug_system.h"
#include "physics/physics_world.h"
#include "ai/ai_system.h"
#include <GLFW/glfw3.h>

namespace vge {

// ============================================
// ImGuiWrapper Implementation
// ============================================

ImGuiWrapper::ImGuiWrapper() : initialized(false), window(nullptr) {}

ImGuiWrapper::~ImGuiWrapper() {
    if (initialized) Shutdown();
}

bool ImGuiWrapper::Initialize(GLFWwindow* glfwWindow) {
    if (initialized) return true;
    if (!glfwWindow) {
        Logger::Error("[ImGui] Invalid GLFW window");
        return false;
    }
    
    window = glfwWindow;
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    // Enable docking (if available in this ImGui version)
    #ifdef IMGUI_HAS_DOCK
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    #endif
    
    // Setup style
    SetupStyle();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    initialized = true;
    Logger::Info("[ImGui] Initialized successfully");
    return true;
}

void ImGuiWrapper::Shutdown() {
    if (!initialized) return;
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    initialized = false;
    window = nullptr;
    Logger::Info("[ImGui] Shutdown");
}

void ImGuiWrapper::BeginFrame() {
    if (!initialized) return;
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiWrapper::EndFrame() {
    if (!initialized) return;
    
    ImGui::Render();
}

void ImGuiWrapper::Render() {
    if (!initialized) return;
    
    // Render ImGui draw data
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Update and Render additional Platform Windows (if multi-viewport enabled)
    #ifdef IMGUI_HAS_DOCK
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
    #endif
}

void ImGuiWrapper::ProcessEvent(void* event) {
    // Handled automatically by ImGui_ImplGlfw_InstallCallbacks
    // This is for manual event forwarding if needed
}

void ImGuiWrapper::SetupStyle() {
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    
    // Colors - Dark professional theme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.40f, 0.80f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.45f, 0.85f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.40f, 0.80f, 0.5f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.45f, 0.85f, 0.7f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.50f, 0.90f, 1.0f);
}

// ============================================
// EditorUI Implementation
// ============================================

void EditorUI::ShowSceneViewport(World* world, Camera* camera) {
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_MenuBar);
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Wireframe", nullptr, nullptr);
            ImGui::MenuItem("Shaded", nullptr, nullptr);
            ImGui::MenuItem("Lighting Only", nullptr, nullptr);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Camera")) {
            ImGui::MenuItem("Perspective", nullptr, nullptr);
            ImGui::MenuItem("Top", nullptr, nullptr);
            ImGui::MenuItem("Front", nullptr, nullptr);
            ImGui::MenuItem("Side", nullptr, nullptr);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    // Viewport size
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    // TODO: Render world to framebuffer and display as image
    ImGui::Text("Viewport: %.0f x %.0f", viewportSize.x, viewportSize.y);
    ImGui::Text("Camera: %.1f, %.1f, %.1f", 
                camera->GetPosition().x, 
                camera->GetPosition().y, 
                camera->GetPosition().z);
    
    ImGui::End();
}

void EditorUI::ShowHierarchyPanel(EntityManager* entityManager) {
    ImGui::Begin("Hierarchy");
    
    if (entityManager) {
        // TODO: List all entities when EntityManager is implemented
        ImGui::Text("Entities: (EntityManager not yet implemented)");
        
        if (ImGui::TreeNode("World")) {
            ImGui::Selectable("Directional Light");
            ImGui::Selectable("Player");
            ImGui::TreePop();
        }
    }
    
    ImGui::End();
}

void EditorUI::ShowInspectorPanel(Entity* entity) {
    ImGui::Begin("Inspector");
    
    if (entity) {
        ImGui::Text("Entity ID: %u", entity->GetID());
        ImGui::Text("Name: %s", entity->GetName().c_str());
        ImGui::Text("Active: %s", entity->IsActive() ? "Yes" : "No");
        
        // Components
        if (ImGui::CollapsingHeader("Components")) {
            auto components = entity->GetAllComponents();
            for (auto* comp : components) {
                ImGui::BulletText("Component");
            }
            if (components.empty()) {
                ImGui::Text("No components");
            }
        }
    } else {
        ImGui::Text("No entity selected");
    }
    
    ImGui::End();
}

void EditorUI::ShowProjectPanel() {
    ImGui::Begin("Project");
    
    if (ImGui::TreeNode("Assets")) {
        ImGui::Selectable("Materials");
        ImGui::Selectable("Meshes");
        ImGui::Selectable("Textures");
        ImGui::Selectable("Scripts");
        ImGui::TreePop();
    }
    
    if (ImGui::TreeNode("Scenes")) {
        ImGui::Selectable("Main");
        ImGui::Selectable("Test");
        ImGui::TreePop();
    }
    
    ImGui::End();
}

void EditorUI::ShowConsolePanel(Console* console) {
    ImGui::Begin("Console");
    
    if (console) {
        // Console output
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        contentSize.y -= 30; // Reserve space for input
        
        ImGui::BeginChild("ConsoleOutput", contentSize, true);
        // TODO: Display console history
        ImGui::Text("Console output...");
        ImGui::EndChild();
        
        // Input
        static char inputBuffer[256] = "";
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##ConsoleInput", inputBuffer, 256, 
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            // Execute command
            inputBuffer[0] = '\0';
        }
    }
    
    ImGui::End();
}

void EditorUI::ShowDebugPanel(DebugSystem* debug) {
    ImGui::Begin("Debug");
    
    if (debug) {
        auto& visualizer = debug->GetVisualizer();
        
        bool showNavMesh = visualizer.IsModeEnabled(DebugDrawMode::NavMesh);
        if (ImGui::Checkbox("Show NavMesh", &showNavMesh)) {
            visualizer.ToggleMode(DebugDrawMode::NavMesh);
        }
        
        bool showPaths = visualizer.IsModeEnabled(DebugDrawMode::Wireframe);
        if (ImGui::Checkbox("Show Paths", &showPaths)) {
            visualizer.ToggleMode(DebugDrawMode::Wireframe);
        }
        
        bool showPhysics = visualizer.IsModeEnabled(DebugDrawMode::CollisionShapes);
        if (ImGui::Checkbox("Show Physics", &showPhysics)) {
            visualizer.ToggleMode(DebugDrawMode::CollisionShapes);
        }
        
        bool showChunkBorders = visualizer.IsModeEnabled(DebugDrawMode::ChunkBorders);
        if (ImGui::Checkbox("Show Chunk Borders", &showChunkBorders)) {
            visualizer.ToggleMode(DebugDrawMode::ChunkBorders);
        }
        
        bool showLightVolumes = visualizer.IsModeEnabled(DebugDrawMode::LightVolumes);
        if (ImGui::Checkbox("Show Light Volumes", &showLightVolumes)) {
            visualizer.ToggleMode(DebugDrawMode::LightVolumes);
        }
        
        bool showGrid = visualizer.ShowGrid();
        if (ImGui::Checkbox("Show Grid", &showGrid)) {
            visualizer.ToggleGrid();
        }
        
        bool showAxes = visualizer.ShowAxes();
        if (ImGui::Checkbox("Show Axes", &showAxes)) {
            visualizer.ToggleAxes();
        }
        
        bool showStats = visualizer.ShowStats();
        if (ImGui::Checkbox("Show Stats", &showStats)) {
            visualizer.ToggleStats();
        }
    }
    
    ImGui::End();
}

void EditorUI::ShowBlockPicker(BlockRegistry* registry, BlockTypeID& selected) {
    ImGui::Begin("Block Picker");
    
    // Use OldBlockRegistry for now since BlockRegistry isn't fully implemented
    OldBlockRegistry& oldRegistry = OldBlockRegistry::GetInstance();
    
    // Grid of blocks
    int blockCount = 25; // Known block types
    int columns = 4;
    
    for (int i = 0; i < blockCount; ++i) {
        if (i > 0 && i % columns != 0) {
            ImGui::SameLine();
        }
        
        const BlockInfo& info = oldRegistry.GetBlock(static_cast<BlockTypeID>(i));
        
        // Color button
        ImVec4 color(info.color.x, info.color.y, info.color.z, 1.0f);
        
        bool isSelected = (selected == i);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
        }
        
        if (ImGui::ColorButton(info.GetName(), color, ImGuiColorEditFlags_NoTooltip, ImVec2(40, 40))) {
            selected = i;
        }
        
        if (isSelected) {
            ImGui::PopStyleColor();
        }
        
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", info.GetName());
        }
    }
    
    ImGui::Separator();
    
    // Selected block info
    if (selected < blockCount) {
        const BlockInfo& info = oldRegistry.GetBlock(static_cast<BlockTypeID>(selected));
        ImGui::Text("Selected: %s", info.GetName());
        ImGui::Text("Solid: %s", info.IsSolid() ? "Yes" : "No");
        ImGui::Text("Opaque: %s", info.IsOpaque() ? "Yes" : "No");
        ImGui::Text("Hardness: %.1f", info.GetHardness());
    }
    
    ImGui::End();
}

void EditorUI::ShowTerrainTools(VoxelEditor* editor) {
    ImGui::Begin("Terrain Tools");
    
    if (editor) {
        ImGui::Text("Brush Tools");
        ImGui::Separator();
        
        if (ImGui::Button("Place", ImVec2(80, 30))) {
            editor->SetTool(VoxelToolType::Place);
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove", ImVec2(80, 30))) {
            editor->SetTool(VoxelToolType::Remove);
        }
        
        if (ImGui::Button("Paint", ImVec2(80, 30))) {
            editor->SetTool(VoxelToolType::Paint);
        }
        ImGui::SameLine();
        if (ImGui::Button("Smooth", ImVec2(80, 30))) {
            editor->SetTool(VoxelToolType::Smooth);
        }
        
        if (ImGui::Button("Raise", ImVec2(80, 30))) {
            editor->SetTool(VoxelToolType::Raise);
        }
        ImGui::SameLine();
        if (ImGui::Button("Lower", ImVec2(80, 30))) {
            editor->SetTool(VoxelToolType::Lower);
        }
        
        ImGui::Separator();
        
        // Brush radius
        float brushRadius = editor->GetBrushRadius();
        if (ImGui::SliderFloat("Brush Radius", &brushRadius, 0.5f, 10.0f)) {
            editor->SetBrushRadius(brushRadius);
        }
        
        // Brush strength
        float brushStrength = editor->GetBrushStrength();
        if (ImGui::SliderFloat("Brush Strength", &brushStrength, 0.1f, 1.0f)) {
            editor->SetBrushStrength(brushStrength);
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Undo", ImVec2(80, 30))) {
            editor->Undo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Redo", ImVec2(80, 30))) {
            editor->Redo();
        }
    }
    
    ImGui::End();
}

void EditorUI::ShowEntitySpawner(EntityArchetypeRegistry* registry) {
    ImGui::Begin("Entity Spawner");
    
    if (registry) {
        auto archetypeIds = registry->GetAllArchetypeIds();
        
        ImGui::Text("Archetypes: %zu", archetypeIds.size());
        ImGui::Separator();
        
        for (const auto& id : archetypeIds) {
            const EntityArchetype* archetype = registry->GetArchetype(id);
            if (!archetype) continue;
            
            if (ImGui::Button(archetype->displayName.c_str(), ImVec2(-1, 25))) {
                // Spawn entity at cursor or camera position
            }
            
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("ID: %s\nHealth: %.0f\nSpeed: %.1f",
                                  archetype->id.c_str(),
                                  archetype->health,
                                  archetype->speed);
            }
        }
    }
    
    ImGui::End();
}

void EditorUI::ShowGizmoToolbar(GizmoType& currentGizmo) {
    ImGui::Begin("Gizmos");
    
    if (ImGui::RadioButton("Translate", currentGizmo == GizmoType::Translate)) {
        currentGizmo = GizmoType::Translate;
    }
    if (ImGui::RadioButton("Rotate", currentGizmo == GizmoType::Rotate)) {
        currentGizmo = GizmoType::Rotate;
    }
    if (ImGui::RadioButton("Scale", currentGizmo == GizmoType::Scale)) {
        currentGizmo = GizmoType::Scale;
    }
    
    ImGui::End();
}

void EditorUI::ShowEditorToolbar(bool& playMode, bool& pauseMode) {
    ImGui::Begin("Toolbar", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    // Play/Stop
    if (!playMode) {
        if (ImGui::Button("Play", ImVec2(60, 30))) {
            playMode = true;
            pauseMode = false;
        }
    } else {
        if (ImGui::Button("Stop", ImVec2(60, 30))) {
            playMode = false;
            pauseMode = false;
        }
    }
    
    ImGui::SameLine();
    
    // Pause
    if (playMode) {
        if (ImGui::Button(pauseMode ? "Resume" : "Pause", ImVec2(60, 30))) {
            pauseMode = !pauseMode;
        }
    }
    
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(20, 0));
    ImGui::SameLine();
    
    // Save/Load
    if (ImGui::Button("Save", ImVec2(60, 30))) {
        // Save scene
    }
    ImGui::SameLine();
    if (ImGui::Button("Load", ImVec2(60, 30))) {
        // Load scene
    }
    
    ImGui::End();
}

// ============================================
// DebugVisualizationPanel Implementation
// ============================================

void DebugVisualizationPanel::Show(DebugVisualizationFlags& flags) {
    ImGui::Begin("Debug Visualization");
    
    ImGui::Checkbox("Navigation Mesh", &flags.showNavMesh);
    ImGui::Checkbox("AI Paths", &flags.showPaths);
    ImGui::Checkbox("Physics Bodies", &flags.showPhysics);
    ImGui::Checkbox("Colliders", &flags.showColliders);
    ImGui::Checkbox("Entity Bounds", &flags.showEntityBounds);
    ImGui::Checkbox("AI States", &flags.showAIStates);
    ImGui::Checkbox("Chunk Borders", &flags.showChunkBorders);
    ImGui::Checkbox("Light Volumes", &flags.showLightVolumes);
    ImGui::Checkbox("Camera Frustum", &flags.showFrustum);
    ImGui::Checkbox("Raycasts", &flags.showRaycasts);
    
    ImGui::Separator();
    
    if (ImGui::Button("Enable All")) {
        flags.showNavMesh = true;
        flags.showPaths = true;
        flags.showPhysics = true;
        flags.showColliders = true;
        flags.showEntityBounds = true;
        flags.showAIStates = true;
        flags.showChunkBorders = true;
        flags.showLightVolumes = true;
        flags.showFrustum = true;
        flags.showRaycasts = true;
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Disable All")) {
        flags.showNavMesh = false;
        flags.showPaths = false;
        flags.showPhysics = false;
        flags.showColliders = false;
        flags.showEntityBounds = false;
        flags.showAIStates = false;
        flags.showChunkBorders = false;
        flags.showLightVolumes = false;
        flags.showFrustum = false;
        flags.showRaycasts = false;
    }
    
    ImGui::End();
}

void DebugVisualizationPanel::RenderVisualizations(const DebugVisualizationFlags& flags,
                                                    World* world,
                                                    PhysicsWorld* physics,
                                                    AISystem* ai,
                                                    Camera* camera) {
    // This would call DebugSystem draw commands based on flags
    // Implementation depends on DebugSystem capabilities
    
    if (flags.showNavMesh && ai && ai->GetNavMesh()) {
        // Draw navmesh nodes and connections
    }
    
    if (flags.showPaths && ai) {
        // Draw active AI paths
    }
    
    if (flags.showPhysics && physics) {
        // Draw physics bodies
    }
    
    if (flags.showChunkBorders && world) {
        // Draw chunk boundaries
    }
    
    if (flags.showFrustum && camera) {
        // Draw camera frustum
    }
}

} // namespace vge
