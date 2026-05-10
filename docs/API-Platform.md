# Platform Module API
**Files:** src/platform/file_system.h, src/platform/input.h, src/platform/threading.h, src/platform/timer.h, src/platform/window.h

## `platform/file_system.h`
```cpp
namespace aether {
```

### `class FileSystem`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `exists` | `static bool` | `const std::string& path` |
| `writeText` | `static bool` | `const std::string& path, const std::string& text` |

## `platform/input.h`
```cpp
namespace vge {
```

### `class Input`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Update` | `void` | `void* windowHandle` |
| `IsKeyPressed` | `bool` | `int key` |
| `IsKeyJustPressed` | `bool` | `int key` |
| `IsKeyReleased` | `bool` | `int key` |
| `IsMouseButtonPressed` | `bool` | `int button` |
| `GetMousePosition` | `void` | `double& x, double& y` |
| `EnableTerminalMode` | `void` | `` |
| `DisableTerminalMode` | `void` | `` |

## `platform/threading.h`
```cpp
namespace aether {
```

### `class ThreadPool`

## `platform/timer.h`
```cpp
namespace aether {
```

### `class Timer`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `start` | `void` | `` |
| `elapsedSeconds` | `float` | `` |

## `platform/window.h`
```cpp
namespace vge {
```

### `class Window`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `KeyCallback` | `static void` | `GLFWwindow* window, int key, int scancode, int action, int mods` |
| `MouseButtonCallback` | `static void` | `GLFWwindow* window, int button, int action, int mods` |
| `CursorCallback` | `static void` | `GLFWwindow* window, double x, double y` |
| `CloseCallback` | `static void` | `GLFWwindow* window` |
| `Initialize` | `bool` | `int width, int height, const std::string& title` |
| `Shutdown` | `void` | `` |
| `PollEvents` | `void` | `` |
| `SwapBuffers` | `void` | `` |
| `ShouldClose` | `bool` | `` |
| `GetHandle` | `void*` | `` |
| `IsKeyPressed` | `bool` | `int key` |
| `GetMousePosition` | `void` | `double& x, double& y` |
| `IsMouseButtonPressed` | `bool` | `int button` |
| `SetCursorMode` | `void` | `bool locked` |
