# Editor System

## Overview

The Aether Engine includes a comprehensive in-game editor with visual editing capabilities, multi-viewport support, and asset management.

## Core Components

### InGameEditor

Main editor class that manages all editing functionality:

```cpp
#include "editor/in_game_editor.h"

// Create editor
vge::InGameEditor editor(world, camera, input, renderer);
editor.Initialize();

// Toggle editor mode
editor.Activate();   // Enter edit mode
editor.Deactivate(); // Exit edit mode

// Selection
editor.SelectBlock(position, normal);
editor.SelectEntity(entityId);
editor.ClearSelection();

// Block operations
editor.PlaceBlockAt(position, normal);
editor.RemoveBlockAt(position);
editor.PaintBlockAt(position);

// Entity operations
editor.SpawnEntity("zombie", position);
editor.RemoveSelectedEntity();
editor.DuplicateSelectedEntity();

// Terrain tools
editor.RaiseTerrainAt(position);
editor.LowerTerrainAt(position);
editor.SmoothTerrainAt(position);

// Undo/Redo
editor.Undo();
editor.Redo();
```

### Editor Selection

```cpp
struct EditorSelection {
    enum class Type { None, Block, Entity };
    
    Type type = Type::None;
    Vec3 blockPosition;      // For block selection
    uint32_t entityId = 0;   // For entity selection
    Vec3 worldPosition;      // Hit point in world
    Vec3 normal;             // Surface normal
    bool active = false;
};
```

### Gizmo Types

```cpp
enum class GizmoType {
    None,
    Translate,  // XYZ arrows
    Rotate,     // XYZ circles
    Scale       // XYZ boxes
};

editor.SetGizmoType(GizmoType::Translate);
```

## ImGui Integration

### EditorUI Panels

The editor uses Dear ImGui for all UI panels:

```cpp
#include "ui/imgui_wrapper.h"

vge::EditorUI editorUI;

// Show panels
editorUI.ShowSceneViewport(camera, world);
editorUI.ShowHierarchyPanel(entityManager);
editorUI.ShowInspectorPanel(selectedEntity);
editorUI.ShowProjectBrowser(assetBrowser);
editorUI.ShowConsolePanel(console);
editorUI.ShowDebugPanel(debugSystem);
editorUI.ShowBlockPicker(blockRegistry, selectedBlock);
editorUI.ShowTerrainTools(voxelEditor);
editorUI.ShowEntitySpawner(archetypeRegistry);
editorUI.ShowGizmoToolbar(currentGizmoType);
editorUI.ShowEditorToolbar(editor);
```

### Panel Descriptions

| Panel | Purpose |
|-------|---------|
| Scene Viewport | Main 3D view with grid, gizmos, selection |
| Hierarchy | List of all entities in scene |
| Inspector | Properties of selected entity/block |
| Project Browser | File/asset browser |
| Console | Log output and commands |
| Debug Visualization | Toggle debug overlays |
| Block Picker | Visual block selection with colors |
| Terrain Tools | Brush settings for terrain editing |
| Entity Spawner | Spawn entities from archetypes |
| Gizmo Toolbar | Select translate/rotate/scale |
| Editor Toolbar | Play/pause/save buttons |

## Debug Visualization

### DebugRenderer

OpenGL-based debug drawing system:

```cpp
#include "debug/debug_renderer.h"

vge::DebugRenderer& debug = vge::GetDebugRenderer();
debug.Initialize();

// Basic shapes
debug.DrawLine(start, end, color);
debug.DrawBox(min, max, color);
debug.DrawSphere(center, radius, color);
debug.DrawCross(position, size, color);
debug.DrawArrow(start, end, color);
debug.DrawGrid(center, size, step, color);

// Persistent (timed) drawing
debug.DrawLinePersistent(start, end, color, duration);
debug.DrawBoxPersistent(min, max, color, duration);

// Specialized visualizations
debug.DrawNavMesh(navMesh, color);
debug.DrawPath(path, color);
debug.DrawCollider(collider, color);
debug.DrawRigidBody(body, color);
debug.DrawChunkBorder(chunkX, chunkY, chunkZ, color);
debug.DrawEntityBounds(min, max, color);
debug.DrawAIState(position, "Idle", color);
debug.DrawRaycast(origin, direction, distance, hit, hitPoint);
debug.DrawLightVolume(position, range, color);
debug.DrawSelectionBox(min, max, color);

// Gizmos
debug.DrawGizmoTranslate(position, size);
debug.DrawGizmoRotate(position, size);
debug.DrawGizmoScale(position, size);

// Render all debug commands
debug.Render(camera);
debug.Update(deltaTime);  // Update persistent commands
debug.Clear();            // Clear immediate commands
```

### Debug Draw Modes

```cpp
enum class DebugDrawMode {
    None = 0,
    Wireframe = 1,
    Normals = 2,
    BoundingBoxes = 4,
    ChunkBorders = 8,
    LightVolumes = 16,
    CollisionShapes = 32,
    NavMesh = 64,
    All = -1
};

// Toggle modes
vge::DebugVisualizer& visualizer = debugSystem.GetVisualizer();
visualizer.ToggleMode(DebugDrawMode::NavMesh);
visualizer.ToggleMode(DebugDrawMode::CollisionShapes);
bool showNavMesh = visualizer.IsModeEnabled(DebugDrawMode::NavMesh);
```

## Scene Viewport System

### Viewport Types

```cpp
enum class ViewportType {
    Perspective,  // Orbit camera
    Top,          // Orthographic Y-up
    Front,        // Orthographic Z-forward
    Side,         // Orthographic X-right
    Isometric     // Fixed angle
};
```

### ViewportManager

```cpp
#include "editor/scene_viewport.h"

vge::ViewportManager viewportManager;
viewportManager.Initialize(screenWidth, screenHeight);

// Layouts
viewportManager.SetLayoutSingle();           // Fullscreen perspective
viewportManager.SetLayoutQuad();             // 4 views (persp, top, front, side)
viewportManager.SetLayoutTriple();           // Perspective + 2 orthographic
viewportManager.SetLayoutSplitHorizontal();  // Two side-by-side
viewportManager.SetLayoutSplitVertical();    // Two stacked

// Access viewports
vge::SceneViewport* active = viewportManager.GetActiveViewport();
vge::SceneViewport* viewport = viewportManager.GetViewport(index);

// Camera control
active->SetCameraPosition(position);
active->SetCameraTarget(target);
active->Orbit(deltaYaw, deltaPitch);
active->Pan(deltaX, deltaY);
active->Zoom(delta);
active->FocusOnPoint(point);
active->FocusOnBounds(min, max);

// View settings
active->SetShowGrid(true);
active->SetShowGizmos(true);
active->SetShowSelection(true);

// Update and render
viewportManager.Update(deltaTime);
viewportManager.Render();
```

### Screen/World Conversion

```cpp
// Screen to world ray
Vec3 ray = viewport.ScreenToWorldRay(screenX, screenY);

// Screen to world point at depth
Vec3 worldPos = viewport.ScreenToWorldPoint(screenX, screenY, depth);

// World to screen
Vec2 screenPos = viewport.WorldToScreen(worldPos);
bool visible = viewport.IsWorldPointVisible(worldPos);
```

## Asset Browser

### Asset Types

```cpp
enum class AssetType {
    Unknown,
    Texture,        // .png, .jpg, .tga
    Model,          // .obj, .fbx, .gltf
    Material,
    Shader,         // .vert, .frag, .glsl
    Audio,          // .wav, .mp3, .ogg
    Script,         // .lua, .js, .py
    Prefab,         // .json
    Scene,          // .scene
    BlockType,
    EntityArchetype,
    Font,
    Animation,
    ParticleSystem
};
```

### AssetBrowser Usage

```cpp
#include "editor/asset_browser.h"

vge::AssetBrowser assetBrowser;
assetBrowser.Initialize("assets/");

// Navigation
assetBrowser.SetPath("assets/models/");
assetBrowser.GoUp();
assetBrowser.GoToRoot();

// Filtering
assetBrowser.SetSearchQuery("tree");
assetBrowser.SetTypeFilter(AssetType::Model);
assetBrowser.SetShowFavoritesOnly(true);
assetBrowser.ClearFilter();

// Selection
assetBrowser.SelectAsset(index);
assetBrowser.MultiSelect(index, true);  // Add to selection
vge::AssetInfo* selected = assetBrowser.GetSelectedAsset();

// Asset operations
assetBrowser.ImportAsset("/path/to/model.obj");
assetBrowser.DeleteAsset(assetId);
assetBrowser.RenameAsset(assetId, "new_name");
assetBrowser.DuplicateAsset(assetId);
assetBrowser.ToggleFavorite(assetId);

// Callbacks
assetBrowser.SetOnAssetSelected([](const AssetInfo& asset) {
    // Handle selection
});
assetBrowser.SetOnAssetDoubleClicked([](const AssetInfo& asset) {
    // Handle double click
});
```

### Asset Info

```cpp
struct AssetInfo {
    std::string id;
    std::string name;
    std::string path;
    AssetType type;
    size_t fileSize;
    std::string lastModified;
    std::string thumbnailPath;
    bool isDirectory;
    bool isFavorite;
    std::vector<std::string> tags;
};
```

## Fly Camera

```cpp
vge::FlyCamera flyCamera;

// Update in game loop
flyCamera.Update(deltaTime, input);

// Set position/rotation
flyCamera.SetPosition(Vec3(10, 5, 10));
flyCamera.SetRotation(yaw, pitch);
flyCamera.LookAt(target);

// Settings
flyCamera.SetSpeed(10.0f);
flyCamera.SetSensitivity(0.1f);

// Get camera vectors
Vec3 pos = flyCamera.GetPosition();
Vec3 forward = flyCamera.GetForward();
Vec3 right = flyCamera.GetRight();
Vec3 up = flyCamera.GetUp();

// Screen ray
Vec3 ray = flyCamera.ScreenToWorldRay(mouseX, mouseY, screenW, screenH);
```

## Block Picker

```cpp
vge::BlockPicker picker(world);

// Raycast to find block
Vec3 position, normal;
if (picker.PickBlock(origin, direction, position, normal)) {
    // Block found at position
}

// Set reach distance
picker.SetReachDistance(10.0f);
```

## Editor Callbacks

```cpp
editor.SetOnEditorActivated([]() {
    // Editor activated
});

editor.SetOnEditorDeactivated([]() {
    // Editor deactivated
});

editor.SetOnSelectionChanged([](const EditorSelection& selection) {
    // Selection changed
    if (selection.type == EditorSelection::Type::Block) {
        // Block selected
    } else if (selection.type == EditorSelection::Type::Entity) {
        // Entity selected
    }
});
```

## File Structure

```
src/editor/
├── in_game_editor.h      # Main editor class
├── in_game_editor.cpp    # Editor implementation
├── scene_viewport.h      # Viewport system
├── scene_viewport.cpp    # Viewport implementation
├── asset_browser.h       # Asset browser
├── asset_browser.cpp     # Asset browser implementation
├── entity_spawner.h      # Entity spawning
└── entity_spawner.cpp    # Spawner implementation

src/ui/
├── imgui_wrapper.h       # ImGui integration
└── imgui_wrapper.cpp     # EditorUI panels

src/debug/
├── debug_system.h        # Debug system
├── debug_system.cpp      # Profiler, metrics
├── debug_renderer.h      # Debug visualization
└── debug_renderer.cpp    # OpenGL debug rendering
```

## Integration Example

```cpp
// Initialize systems
vge::InGameEditor editor(world, camera, input, renderer);
editor.Initialize();

vge::ViewportManager viewports;
viewports.Initialize(1920, 1080);
viewports.SetLayoutQuad();

vge::AssetBrowser assetBrowser;
assetBrowser.Initialize("assets/");

vge::DebugRenderer& debug = vge::GetDebugRenderer();
debug.Initialize();

// Main loop
while (running) {
    // Update
    editor.Update(deltaTime, input);
    viewports.Update(deltaTime);
    debug.Update(deltaTime);
    
    // Render
    viewports.Render();
    editor.Render();
    debug.Render(*viewports.GetActiveViewport()->GetCamera());
    
    // ImGui panels
    editorUI.ShowSceneViewport(camera, world);
    editorUI.ShowInspectorPanel(selectedEntity);
    editorUI.ShowBlockPicker(blockRegistry, selectedBlock);
    editorUI.ShowDebugPanel(debugSystem);
    
    // Present
    renderer->EndFrame();
}
```

## Key Features

- **Visual Editing**: Gizmos, selection highlights, debug overlays
- **Multi-Viewport**: Single, quad, triple, split layouts
- **Asset Management**: Browse, import, organize game assets
- **Undo/Redo**: Full undo/redo for all editing operations
- **Debug Visualization**: NavMesh, paths, colliders, AI states
- **ImGui Integration**: Dockable panels, professional UI
