# Ui Module API
**Files:** src/ui/button.h, src/ui/console.h, src/ui/imgui_wrapper.h, src/ui/label.h, src/ui/menu_system.h, src/ui/panel.h, src/ui/ui_element.h, src/ui/ui_manager.h

## `ui/button.h`
```cpp
namespace aether {
```

## `ui/console.h`
```cpp
namespace vge {
```

### `class Console`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `void` | `` |
| `Shutdown` | `void` | `` |
| `AddLine` | `void` | `const std::string& line` |
| `Clear` | `void` | `` |
| `Render` | `void` | `` |
| `Toggle` | `void` | `` |
| `ExecuteCommand` | `void` | `const std::string& cmd` |

## `ui/imgui_wrapper.h`
```cpp
namespace vge {
```

### `class ImGuiWrapper`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `bool` | `void* windowHandle` |
| `Shutdown` | `void` | `` |
| `BeginFrame` | `void` | `` |
| `EndFrame` | `void` | `` |
| `Render` | `void` | `` |

## `ui/label.h`
```cpp
namespace aether {
```

## `ui/menu_system.h`
```cpp
namespace vge {
```

### `enum class MenuScreen`
| Value | Description |
|-------|-------------|
| `None` | |
| `MainMenu` | |
| `Settings` | |
| `Pause` | |
| `Inventory` | |
| `Crafting` | |

### `class MenuScreen`

### `class MenuSystem`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `RenderMainMenu` | `void` | `` |
| `RenderSettings` | `void` | `` |
| `RenderPauseMenu` | `void` | `` |
| `RenderInventory` | `void` | `` |
| `RenderCrafting` | `void` | `` |
| `Initialize` | `bool` | `Renderer* renderer, Window* window` |
| `Shutdown` | `void` | `` |
| `Update` | `void` | `float deltaTime` |
| `ShowScreen` | `void` | `MenuScreen screen` |
| `ShowPauseMenu` | `void` | `` |
| `HideMenu` | `void` | `` |
| `IsMenuOpen` | `bool` | `` |

## `ui/panel.h`
```cpp
namespace aether {
```

## `ui/ui_element.h`
```cpp
namespace aether {
```

### `class UIElement`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `setPosition` | `void` | `const Vec2& pos` |
| `setSize` | `void` | `const Vec2& size` |
| `getPosition` | `Vec2` | `` |
| `getSize` | `Vec2` | `` |
| `setVisible` | `void` | `bool visible` |
| `isVisible` | `bool` | `` |
| `addChild` | `void` | `std::shared_ptr<UIElement> child` |
| `render` | `virtual void` | `` |
| `onClick` | `virtual bool` | `const Vec2& pos` |

## `ui/ui_manager.h`
```cpp
namespace aether {
```

### `class UIManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `addElement` | `void` | `std::shared_ptr<UIElement> element` |
| `removeElement` | `void` | `UIElement* element` |
| `render` | `void` | `` |
| `onClick` | `bool` | `const Vec2& pos` |
| `clear` | `void` | `` |
