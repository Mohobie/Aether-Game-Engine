# Input Module API
**Files:** src/input/input_manager.h, src/input/key_bindings.h

## `input/input_manager.h`
```cpp
namespace aether {
```

### `enum class Key`
| Value | Description |
|-------|-------------|
| `Unknown` | |
| `N` | |
| `Escape` | |
| `Right` | |
| `Shift` | |

### `enum class MouseButton`
| Value | Description |
|-------|-------------|
| `Left` | |

### `class Key`

### `class MouseButton`

### `class InputManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `update` | `void` | `` |
| `isKeyDown` | `bool` | `Key key` |
| `isKeyPressed` | `bool` | `Key key` |
| `isKeyReleased` | `bool` | `Key key` |
| `isMouseButtonDown` | `bool` | `MouseButton button` |
| `getMousePosition` | `void` | `float& x, float& y` |
| `getMouseDelta` | `void` | `float& dx, float& dy` |

## `input/key_bindings.h`
```cpp
namespace aether {
```

### `struct KeyBinding`
| Member | Type |
|--------|------|
| `key` | `Key` |

### `class KeyBindings`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `loadDefaults` | `void` | `` |
| `bind` | `void` | `const std::string& action, Key key` |
| `getBinding` | `Key` | `const std::string& action` |
| `resetToDefaults` | `void` | `` |
