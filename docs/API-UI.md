# UI Module API

**Canonical files for this audit:** `src/ui/ui_system.h`, `src/ui/console.h`, `src/ui/imgui_wrapper.h`, `src/ui/menu_system.h`  
**Legacy / inactive for this audit:** `src/ui/ui_manager.h`

This page reflects the current canonical UI architecture used by `game/application.h` and the active audited module map. The supported retained-mode UI path is `ui/ui_system.h`, while the smaller `ui/ui_manager.h` API remains in-tree only as legacy source material and is not part of the active library target.

All UI types documented here live in the `vge` namespace unless noted otherwise.

## `ui/ui_system.h`

```cpp
namespace vge {
```

### `enum class UIEventType`

| Value |
|-------|
| `Click` |
| `Hover` |
| `Press` |
| `Release` |
| `Drag` |
| `Scroll` |
| `TextInput` |

### `struct UIEvent`

| Member | Type |
|--------|------|
| `type` | `UIEventType` |
| `position` | `Vec2` |
| `delta` | `Vec2` |
| `keyCode` | `int` |
| `text` | `std::string` |

### `class UIElement`

Base class for retained-mode UI elements.

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `UIElement` | constructor | `const std::string& name = "Element"` |
| `SetPosition` | `void` | `const Vec2& pos` |
| `SetSize` | `void` | `const Vec2& sz` |
| `GetPosition` | `Vec2` | `` |
| `GetSize` | `Vec2` | `` |
| `GetAbsolutePosition` | `Vec2` | `` |
| `SetVisible` | `void` | `bool vis` |
| `IsVisible` | `bool` | `` |
| `SetEnabled` | `void` | `bool en` |
| `IsEnabled` | `bool` | `` |
| `SetHovered` | `void` | `bool h` |
| `IsHovered` | `bool` | `` |
| `IsPressed` | `bool` | `` |
| `AddChild` | `void` | `std::unique_ptr<UIElement> child` |
| `RemoveChild` | `void` | `UIElement* child` |
| `GetParent` | `UIElement*` | `` |
| `GetChildren` | `const std::vector<std::unique_ptr<UIElement>>&` | `` |
| `FindChild` | `UIElement*` | `const std::string& childName` |
| `GetName` | `std::string` | `` |
| `HandleEvent` | `virtual bool` | `const UIEvent& event` |
| `OnClick` | `virtual void` | `` |
| `OnHover` | `virtual void` | `` |
| `OnPress` | `virtual void` | `` |
| `OnRelease` | `virtual void` | `` |
| `SetOnClick` | `void` | `std::function<void()> callback` |
| `SetOnHover` | `void` | `std::function<void()> callback` |
| `SetOnPress` | `void` | `std::function<void()> callback` |
| `SetOnRelease` | `void` | `std::function<void()> callback` |
| `SetOnEvent` | `void` | `std::function<void(const UIEvent&)> callback` |
| `SetBackgroundColor` | `void` | `const Vec3& color` |
| `SetForegroundColor` | `void` | `const Vec3& color` |
| `SetHoverColor` | `void` | `const Vec3& color` |
| `SetPressedColor` | `void` | `const Vec3& color` |
| `SetAlpha` | `void` | `float a` |
| `SetBorderRadius` | `void` | `float radius` |
| `SetBorderWidth` | `void` | `float width` |
| `SetBorderColor` | `void` | `const Vec3& color` |
| `Render` | `virtual void` | `` |
| `Update` | `virtual void` | `float deltaTime` |
| `Layout` | `virtual void` | `` |
| `ContainsPoint` | `bool` | `const Vec2& point` |
| `PrintHierarchy` | `void` | `int indent = 0` |

### Canonical concrete UI types

- `UIButton`
- `UILabel`
- `UISlider`
- `UITextInput`
- `UIPanel`
- `UIImage`
- `UICheckbox`
- `UIDropdown`

### `class UIManager`

Retained-mode root UI manager from `ui/ui_system.h`.

| Method | Return Type | Parameters |
|--------|-------------|------------|
| `UIManager` | constructor | `` |
| `~UIManager` | destructor | `` |
| `CreateElement` | `UIElement*` | `const std::string& type, const std::string& name` |
| `CreateButton` | `UIButton*` | `const std::string& name, const std::string& text` |
| `CreateLabel` | `UILabel*` | `const std::string& name, const std::string& text` |
| `CreateSlider` | `UISlider*` | `const std::string& name, float min, float max` |
| `CreateTextInput` | `UITextInput*` | `const std::string& name, const std::string& placeholder` |
| `CreatePanel` | `UIPanel*` | `const std::string& name` |
| `CreateImage` | `UIImage*` | `const std::string& name, const std::string& texture` |
| `CreateCheckbox` | `UICheckbox*` | `const std::string& name, const std::string& label` |
| `CreateDropdown` | `UIDropdown*` | `const std::string& name` |
| `AddRootElement` | `void` | `std::unique_ptr<UIElement> element` |
| `RemoveElement` | `void` | `UIElement* element` |
| `FindElement` | `UIElement*` | `const std::string& name` |
| `HandleMouseMove` | `void` | `const Vec2& position` |
| `HandleMouseClick` | `void` | `const Vec2& position, bool pressed` |
| `HandleTextInput` | `void` | `const std::string& text` |
| `HandleKeyInput` | `void` | `int keyCode, bool pressed` |
| `Update` | `void` | `float deltaTime` |
| `Render` | `void` | `` |
| `Layout` | `void` | `` |
| `SetScreenSize` | `void` | `const Vec2& size` |
| `GetScreenSize` | `Vec2` | `` |
| `SetScale` | `void` | `float s` |
| `GetScale` | `float` | `` |
| `SetFocus` | `void` | `UIElement* element` |
| `GetFocusedElement` | `UIElement*` | `` |
| `Clear` | `void` | `` |

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

## Legacy note

If an older page or example references `ui/ui_manager.h`, treat it as a legacy parallel UI path. The canonical UI manager for this audit is the richer `UIManager` declared in `ui/ui_system.h`.
