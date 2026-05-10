# Platform Module API

**Files:** `platform/window.h`, `platform/platform_common.h`, `platform/file_system.h`, `platform/threading.h`, `platform/timer.h`, `platform/input_manager.h`

---

## `platform/platform_common.h`

```cpp
namespace vge {
```

### Cross-Platform Detection
| Define | Platform |
|--------|----------|
| `PLATFORM_WINDOWS` | Windows |
| `PLATFORM_LINUX` | Linux |
| `PLATFORM_MACOS` | macOS |
| `PLATFORM_UNIX` | Generic Unix |

### `class Path`
Cross-platform path utilities.

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Join` | `std::string` | `const std::string& a, const std::string& b` | Join two paths |
| `GetDirectory` | `std::string` | `const std::string& path` | Get parent directory |
| `GetFilename` | `std::string` | `const std::string& path` | Get filename |
| `GetExtension` | `std::string` | `const std::string& path` | Get file extension |
| `Normalize` | `std::string` | `const std::string& path` | Normalize separators |
| `IsAbsolute` | `bool` | `const std::string& path` | Check if absolute |

### `class File`
Cross-platform file operations.

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Exists` | `bool` | `const std::string& path` | Check if file exists |
| `IsDirectory` | `bool` | `const std::string& path` | Check if directory |
| `CreateDirectory` | `bool` | `const std::string& path` | Create directory |
| `Delete` | `bool` | `const std::string& path` | Delete file/directory |
| `GetSize` | `size_t` | `const std::string& path` | Get file size |
| `ReadText` | `std::string` | `const std::string& path` | Read text file |
| `WriteText` | `bool` | `const std::string& path, const std::string& content` | Write text file |

### Global Functions
| Function | Return Type | Parameters | Description |
|----------|-------------|------------|-------------|
| `SleepMs` | `void` | `uint32_t milliseconds` | Sleep for milliseconds |
| `GetCurrentThreadId` | `uint64_t` | `void` | Get current thread ID |

---

## `platform/window.h`

```cpp
namespace vge {
```

### `class Window`
GLFW-based window (cross-platform).

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Window` | (ctor) | `void` | Constructor |
| `~Window` | (dtor) | `void` | Destructor |
| `Initialize` | `bool` | `int width, int height, const std::string& title` | Create window |
| `Shutdown` | `void` | `void` | Destroy window |
| `PollEvents` | `void` | `void` | Poll input events |
| `SwapBuffers` | `void` | `void` | Swap buffers |
| `ShouldClose` | `bool` | `void` | Check if should close |
| `GetHandle` | `void*` | `void` | Get native handle |
| `IsKeyPressed` | `bool` | `int key` | Check key pressed |
| `GetMousePosition` | `void` | `double& x, double& y` | Get mouse position |
| `IsMouseButtonPressed` | `bool` | `int button` | Check mouse button |
| `SetCursorMode` | `void` | `bool locked` | Lock/unlock cursor |
| `GetWidth` | `int` | `void` | Get window width |
| `GetHeight` | `int` | `void` | Get window height |

---

## `platform/file_system.h`

```cpp
namespace aether {
```

### `class FileSystem`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `exists` | `bool` | `const std::string& path` | Check exists |
| `readText` | `std::string` | `const std::string& path` | Read text |
| `writeText` | `bool` | `const std::string& path, const std::string& text` | Write text |
| `createDirectory` | `bool` | `const std::string& path` | Create directory |
| `deleteFile` | `bool` | `const std::string& path` | Delete file |
| `listDirectory` | `std::vector<std::string>` | `const std::string& path` | List contents |

---

## `platform/input_manager.h`

```cpp
namespace vge {
```

### `enum class KeyCode`
| Value | Description |
|-------|-------------|
| `W` | W key |
| `A` | A key |
| `S` | S key |
| `D` | D key |
| `Space` | Spacebar |
| `Escape` | Escape |
| `Enter` | Enter |
| `Up` | Up arrow |
| `Down` | Down arrow |
| `Left` | Left arrow |
| `Right` | Right arrow |
| `E` | E key |
| `Q` | Q key |
| `Shift` | Shift |
| `Ctrl` | Control |
| `Count` | Total key count |

### `class Input`
Cross-platform input manager.

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Input` | (ctor) | `void` | Constructor |
| `~Input` | (dtor) | `void` | Destructor |
| `Update` | `void` | `void` | Update input state |
| `IsKeyPressed` | `bool` | `KeyCode key` | Check key held |
| `IsKeyJustPressed` | `bool` | `KeyCode key` | Check key just pressed |
| `IsKeyReleased` | `bool` | `KeyCode key` | Check key released |
| `IsKeyPressed` | `bool` | `int key` | Legacy: check by int |
| `IsKeyJustPressed` | `bool` | `int key` | Legacy: check by int |
| `IsKeyReleased` | `bool` | `int key` | Legacy: check by int |

**Platform Notes:**
- **Windows**: Uses `GetAsyncKeyState()` for real-time key detection
- **Linux**: Uses terminal raw mode for keyboard input

---

## `platform/threading.h`

```cpp
namespace aether {
```

### `class ThreadPool`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `ThreadPool` | (ctor) | `size_t numThreads = 4` | Constructor |
| `~ThreadPool` | (dtor) | `void` | Destructor |
| `enqueue` | `void` | `std::function<void()> task` | Add task |

---

## `platform/timer.h`

```cpp
namespace aether {
```

### `class Timer`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Timer` | (ctor) | `void` | Constructor |
| `start` | `void` | `void` | Start timer |
| `elapsed` | `float` | `void` | Get elapsed seconds |
| `elapsed_ms` | `float` | `void` | Get elapsed milliseconds |

---

## Cross-Platform Build Instructions

### Linux
```bash
mkdir build && cd build
cmake ..
make
```

### Windows
```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Dependencies
- **GLFW3**: Window and input (optional, for graphical mode)
- **OpenGL**: Graphics rendering (optional)
- **Lua 5.3+**: Scripting support (optional)

**Windows Setup:**
1. Install GLFW to `C:\glfw` or set `GLFW_DIR` environment variable
2. Install Lua to `C:\lua` or set `LUA_DIR` environment variable
3. Or place libraries in `third_party/` directory

---

*Cross-platform support added 2026-05-10*
