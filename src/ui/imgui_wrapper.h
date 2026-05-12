#pragma once

// Disable obsolete ImGui functions
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "math/vec3.h"
#include "voxel/block_types.h"

struct GLFWwindow;

// Forward declarations
namespace vge {
    class Camera;
    class Entity;
    class EntityManager;
    class World;
    class BlockRegistry;
    class VoxelEditor;
    class EntityArchetypeRegistry;
    class Console;
    class DebugSystem;
    class PhysicsWorld;
    class AISystem;
}

// Gizmo type (match editor definition)
enum class GizmoType {
    None,
    Translate,
    Rotate,
    Scale
};

namespace vge {

// ============================================
// ImGui Wrapper - Real Dear ImGui Integration
// ============================================
class ImGuiWrapper {
private:
    bool initialized;
    GLFWwindow* window;
    
    // Style
    void SetupStyle();
    void SetupDocking();
    
public:
    ImGuiWrapper();
    ~ImGuiWrapper();
    
    // Initialize with GLFW window
    bool Initialize(GLFWwindow* window);
    void Shutdown();
    
    // Frame management
    void BeginFrame();
    void EndFrame();
    void Render();  // Calls ImGui::Render + backend render
    
    // Utilities
    bool IsInitialized() const { return initialized; }
    
    // Input handling - call before glfwPollEvents if needed
    static void ProcessEvent(void* event);  // GLFWevent*
};

// ============================================
// Editor UI Panels
// ============================================
class EditorUI {
public:
    // Main editor windows
    static void ShowSceneViewport(class World* world, class Camera* camera);
    static void ShowHierarchyPanel(class EntityManager* entityManager);
    static void ShowInspectorPanel(class Entity* entity);
    static void ShowProjectPanel();
    static void ShowConsolePanel(class Console* console);
    static void ShowDebugPanel(class DebugSystem* debug);
    
    // Voxel-specific
    static void ShowBlockPicker(class BlockRegistry* registry, BlockTypeID& selected);
    static void ShowTerrainTools(class VoxelEditor* editor);
    static void ShowEntitySpawner(class EntityArchetypeRegistry* registry);
    
    // Gizmo controls
    static void ShowGizmoToolbar(GizmoType& currentGizmo);
    static void ShowEditorToolbar(bool& playMode, bool& pauseMode);
};

// ============================================
// Debug Visualization Panel
// ============================================
struct DebugVisualizationFlags {
    bool showNavMesh = false;
    bool showPaths = false;
    bool showPhysics = false;
    bool showColliders = false;
    bool showEntityBounds = false;
    bool showAIStates = false;
    bool showChunkBorders = false;
    bool showLightVolumes = false;
    bool showFrustum = false;
    bool showRaycasts = false;
};

class DebugVisualizationPanel {
public:
    static void Show(DebugVisualizationFlags& flags);
    static void RenderVisualizations(const DebugVisualizationFlags& flags,
                                     class World* world,
                                     class PhysicsWorld* physics,
                                     class AISystem* ai,
                                     class Camera* camera);
};

} // namespace vge
