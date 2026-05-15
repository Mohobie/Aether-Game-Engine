# Platform Module API

This page was corrected during the 2026-05-14 architecture audit.

**Canonical platform files for this audit:** `platform/window.h`, `platform/platform_common.h`, `platform/file_system.h`, `platform/threading.h`, `platform/timer.h`, `platform/input_manager.h`  
**Legacy / inactive input path:** `platform/input.h`

The important architecture decision here is that `platform/input_manager.h` is the supported input abstraction in the current build. `platform/input.h` remains a legacy parallel input implementation and is intentionally outside the canonical build path.

## `platform/platform_common.h`

```cpp
namespace vge {
```

### Cross-platform detection

- `PLATFORM_WINDOWS`
- `PLATFORM_LINUX`
- `PLATFORM_MACOS`

## `platform/window.h`

```cpp
namespace vge {
```

### `class Window`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Window` | ctor | `` |
| `~Window` | dtor | `` |
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
| `GetWidth` | `int` | `` |
| `GetHeight` | `int` | `` |
| `SetInputCallback` | `void` | `std::function<void(int, int)> cb` |
| `SetMouseCallback` | `void` | `std::function<void(int, int, double, double)> cb` |
| `SetCursorCallback` | `void` | `std::function<void(double, double)> cb` |

## `platform/file_system.h`

```cpp
namespace vge {
```

### `class FileSystem`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `exists` | `static bool` | `const std::string& path` |
| `readText` | `static std::string` | `const std::string& path` |
| `writeText` | `static bool` | `const std::string& path, const std::string& text` |
| `createDirectory` | `static bool` | `const std::string& path` |
| `deleteFile` | `static bool` | `const std::string& path` |
| `listDirectory` | `static std::vector<std::string>` | `const std::string& path` |

## `platform/input_manager.h`

```cpp
namespace vge {
```

### `enum class KeyCode`
| Value |
|-------|
| `W`, `A`, `S`, `D` |
| `Space`, `Escape`, `Enter` |
| `Up`, `Down`, `Left`, `Right` |
| `E`, `Q`, `F`, `Shift`, `Ctrl` |
| `Key0` through `Key9` |
| `Count` |

### `class Input`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Input` | ctor | `` |
| `~Input` | dtor | `` |
| `Update` | `void` | `void* windowHandle = nullptr` |
| `IsKeyPressed` | `bool` | `KeyCode key` |
| `IsKeyJustPressed` | `bool` | `KeyCode key` |
| `IsKeyReleased` | `bool` | `KeyCode key` |
| `SetMouseDelta` | `void` | `float dx, float dy` |
| `GetMouseDelta` | `void` | `float& dx, float& dy` |
| `SetScrollDelta` | `void` | `float delta` |
| `GetScrollDelta` | `float` | `` |
| `ResetMouseDelta` | `void` | `` |
| `IsKeyPressed` | `bool` | `int key` |
| `IsKeyJustPressed` | `bool` | `int key` |
| `IsKeyReleased` | `bool` | `int key` |

## Build note

The current audited Windows pass was revalidated on 2026-05-15 using `C:/Program Files/CMake/bin/cmake.exe`. In this shell, `voxel_engine_lib`, `save_test`, `world_test`, and `voxel_test` are verified; `voxel_engine` remains dependency-gated because `GLFW` is unavailable.

## Legacy note

If an older page or example references `platform/input.h`, treat it as a legacy input path rather than the supported API.
