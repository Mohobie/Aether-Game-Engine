# UI Module API
**Files:** `src/ui/button.h`, `src/ui/console.h`, `src/ui/imgui_wrapper.h`, `src/ui/label.h`, `src/ui/menu_system.h`, `src/ui/panel.h`, `src/ui/ui_element.h`, `src/ui/ui_manager.h`

All UI types documented here live in the `vge` namespace unless noted otherwise.

## `ui/ui_element.h`
### `class UIElement`
Base class for lightweight custom UI elements.

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `UIElement` | constructor | `` |
| `setPosition` | `void` | `const Vec2& pos` |
| `setSize` | `void` | `const Vec2& size` |
| `getPosition` | `Vec2` | `` |
| `getSize` | `Vec2` | `` |
| `setVisible` | `void` | `bool visible` |
| `isVisible` | `bool` | `` |
| `addChild` | `void` | `std::shared_ptr<UIElement> child` |
| `render` | `virtual void` | `` |
| `onClick` | `virtual bool` | `const Vec2& pos` |

### Protected members
| Member | Type |
|--------|------|
| `position` | `Vec2` |
| `size` | `Vec2` |
| `visible` | `bool` |
| `children` | `std::vector<std::shared_ptr<UIElement>>` |

## `ui/ui_manager.h`
### `class UIManager`
Container for top-level `UIElement` instances.

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `UIManager` | constructor | `` |
| `addElement` | `void` | `std::shared_ptr<UIElement> element` |
| `removeElement` | `void` | `UIElement* element` |
| `render` | `void` | `` |
| `onClick` | `bool` | `const Vec2& pos` |
| `clear` | `void` | `` |

## `ui/button.h`
### `class Button : public UIElement`

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Button` | constructor | `` |
| `setText` | `void` | `const std::string& text` |
| `getText` | `std::string` | `` |
| `setCallback` | `void` | `std::function<void()> callback` |
| `render` | `void` | `` |
| `onClick` | `bool` | `const Vec2& pos` |

## `ui/label.h`
### `class Label : public UIElement`

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Label` | constructor | `` |
| `setText` | `void` | `const std::string& text` |
| `getText` | `std::string` | `` |
| `setColor` | `void` | `uint32_t color` |
| `render` | `void` | `` |

## `ui/panel.h`
### `class Panel : public UIElement`

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Panel` | constructor | `` |
| `setBackgroundColor` | `void` | `uint32_t color` |
| `render` | `void` | `` |

## `ui/console.h`
### `class Console`
In-engine console with command integration hooks.

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Console` | constructor | `` |
| `~Console` | destructor | `` |
| `Initialize` | `void` | `` |
| `Shutdown` | `void` | `` |
| `AddLine` | `void` | `const std::string& line` |
| `Clear` | `void` | `` |
| `Render` | `void` | `` |
| `Toggle` | `void` | `` |
| `ExecuteCommand` | `void` | `const std::string& cmd` |
| `IsVisible` | `bool` | `` |
| `SetupCommands` | `void` | `const CommandContext& ctx` |
| `GetHistoryText` | `std::string` | `` |
| `SetCheatsEnabled` | `void` | `bool enabled` |
| `SetDebugEnabled` | `void` | `bool enabled` |

### Forward-declared integration types
| Type |
|------|
| `CommandExecutor` |
| `CommandContext` |

## `ui/imgui_wrapper.h`
### `enum class GizmoType`
| Value |
|-------|
| `None` |
| `Translate` |
| `Rotate` |
| `Scale` |

### `class ImGuiWrapper`
Dear ImGui integration layer for GLFW/OpenGL.

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `ImGuiWrapper` | constructor | `` |
| `~ImGuiWrapper` | destructor | `` |
| `Initialize` | `bool` | `GLFWwindow* window` |
| `Shutdown` | `void` | `` |
| `BeginFrame` | `void` | `` |
| `EndFrame` | `void` | `` |
| `Render` | `void` | `` |
| `IsInitialized` | `bool` | `` |
| `ProcessEvent` | `static void` | `void* event` |

### `class EditorUI`
Static helpers for editor-facing ImGui panels.

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `ShowSceneViewport` | `static void` | `World* world, Camera* camera` |
| `ShowHierarchyPanel` | `static void` | `EntityManager* entityManager` |
| `ShowInspectorPanel` | `static void` | `Entity* entity` |
| `ShowProjectPanel` | `static void` | `` |
| `ShowConsolePanel` | `static void` | `Console* console` |
| `ShowDebugPanel` | `static void` | `DebugSystem* debug` |
| `ShowBlockPicker` | `static void` | `BlockRegistry* registry, BlockTypeID& selected` |
| `ShowTerrainTools` | `static void` | `VoxelEditor* editor` |
| `ShowEntitySpawner` | `static void` | `EntityArchetypeRegistry* registry` |
| `ShowGizmoToolbar` | `static void` | `GizmoType& currentGizmo` |
| `ShowEditorToolbar` | `static void` | `bool& playMode, bool& pauseMode` |

### `struct DebugVisualizationFlags`
| Member | Type |
|--------|------|
| `showNavMesh` | `bool` |
| `showPaths` | `bool` |
| `showPhysics` | `bool` |
| `showColliders` | `bool` |
| `showEntityBounds` | `bool` |
| `showAIStates` | `bool` |
| `showChunkBorders` | `bool` |
| `showLightVolumes` | `bool` |
| `showFrustum` | `bool` |
| `showRaycasts` | `bool` |

### `class DebugVisualizationPanel`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Show` | `static void` | `DebugVisualizationFlags& flags` |
| `RenderVisualizations` | `static void` | `const DebugVisualizationFlags& flags, World* world, PhysicsWorld* physics, AISystem* ai, Camera* camera` |

## `ui/menu_system.h`
### `enum class MenuScreen`
| Value |
|-------|
| `None` |
| `MainMenu` |
| `Settings` |
| `Pause` |
| `Inventory` |
| `Crafting` |

### `class MenuSystem`
ImGui-driven menu flow for the engine runtime.

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `MenuSystem` | constructor | `` |
| `~MenuSystem` | destructor | `` |
| `Initialize` | `bool` | `Renderer* renderer, Window* window` |
| `Shutdown` | `void` | `` |
| `Update` | `void` | `float deltaTime` |
| `ShowScreen` | `void` | `MenuScreen screen` |
| `ShowPauseMenu` | `void` | `` |
| `HideMenu` | `void` | `` |
| `IsMenuOpen` | `bool` | `` |
| `GetCurrentScreen` | `MenuScreen` | `` |

### Public callbacks
| Member | Type |
|--------|------|
| `onStartSinglePlayer` | `std::function<void()>` |
| `onStartMultiplayer` | `std::function<void()>` |
| `onQuit` | `std::function<void()>` |
| `onSaveAndQuit` | `std::function<void()>` |
