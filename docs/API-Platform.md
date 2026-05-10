# Platform Module API

**Files:** `platform/window.h`, `platform/platform.h`, `platform/file_system.h`, `platform/thread.h`, `platform/timer.h`

---

## `platform/window.h`

```cpp
namespace vge {
```

### `struct WindowProperties`
| Member | Type | Description |
|--------|------|-------------|
| `title` | `std::string` | Window title |
| `width` | `int` | Window width |
| `height` | `int` | Window height |
| `fullscreen` | `bool` | Fullscreen mode |
| `resizable` | `bool` | Resizable |
| `vsync` | `bool` | VSync enabled |
| `msaaSamples` | `int` | MSAA sample count |
| `majorVersion` | `int` | OpenGL major version |
| `minorVersion` | `int` | OpenGL minor version |

### `class Window`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Window` | (ctor) | `void` | Constructor |
| `Create` | `bool` | `const WindowProperties& props` | Create window |
| `Destroy` | `void` | `void` | Destroy window |
| `Show` | `void` | `void` | Show window |
| `Hide` | `void` | `void` | Hide window |
| `Minimize` | `void` | `void` | Minimize |
| `Maximize` | `void` | `void` | Maximize |
| `Restore` | `void` | `void` | Restore |
| `Focus` | `void` | `void` | Focus window |
| `IsFocused` | `bool` | `void` | Check focused |
| `IsMinimized` | `bool` | `void` | Check minimized |
| `IsMaximized` | `bool` | `void` | Check maximized |
| `ShouldClose` | `bool` | `void` | Check should close |
| `SetShouldClose` | `void` | `bool close` | Set should close |
| `SwapBuffers` | `void` | `void` | Swap buffers |
| `PollEvents` | `void` | `void` | Poll events |
| `WaitEvents` | `void` | `void` | Wait for events |
| `SetTitle` | `void` | `const std::string& title` | Set title |
| `GetTitle` | `std::string` | `void` | Get title |
| `SetSize` | `void` | `int width, int height` | Set size |
| `GetSize` | `std::pair<int,int>` | `void` | Get size |
| `SetPosition` | `void` | `int x, int y` | Set position |
| `GetPosition` | `std::pair<int,int>` | `void` | Get position |
| `SetFullscreen` | `void` | `bool fullscreen` | Toggle fullscreen |
| `IsFullscreen` | `bool` | `void` | Check fullscreen |
| `SetResizable` | `void` | `bool resizable` | Set resizable |
| `IsResizable` | `bool` | `void` | Check resizable |
| `SetVSync` | `void` | `bool vsync` | Toggle VSync |
| `IsVSyncEnabled` | `bool` | `void` | Check VSync |
| `SetIcon` | `void` | `const std::string& path` | Set window icon |
| `GetFramebufferSize` | `std::pair<int,int>` | `void` | Get framebuffer size |
| `GetContentScale` | `std::pair<float,float>` | `void` | Get DPI scale |
| `GetNativeHandle` | `void*` | `void` | Get native window handle |
| `SetEventCallback` | `void` | `std::function<void(Event&)> callback` | Set event callback |

---

## `platform/platform.h`

### `enum class PlatformType`
| Value | Description |
|-------|-------------|
| `Windows` | Windows OS |
| `Linux` | Linux OS |
| `MacOS` | macOS |
| `Android` | Android |
| `iOS` | iOS |
| `Web` | Web/Emscripten |
| `Unknown` | Unknown platform |

### `class Platform`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetType` | `PlatformType` | `void` | Get platform type |
| `GetName` | `std::string` | `void` | Get platform name |
| `GetCPUCount` | `int` | `void` | Get CPU core count |
| `GetTotalRAM` | `size_t` | `void` | Get total RAM |
| `GetAvailableRAM` | `size_t` | `void` | Get available RAM |
| `GetPageSize` | `size_t` | `void` | Get memory page size |
| `GetCacheLineSize` | `size_t` | `void` | Get cache line size |
| `GetExecutablePath` | `std::string` | `void` | Get exe path |
| `GetWorkingDirectory` | `std::string` | `void` | Get working dir |
| `SetWorkingDirectory` | `void` | `const std::string& path` | Set working dir |
| `GetUserDataDirectory` | `std::string` | `void` | Get user data dir |
| `GetTemporaryDirectory` | `std::string` | `void` | Get temp dir |
| `GetDocumentsDirectory` | `std::string` | `void` | Get documents dir |
| `Sleep` | `void` | `uint32_t milliseconds` | Sleep thread |
| `Yield` | `void` | `void` | Yield thread |
| `GetTime` | `double` | `void` | Get high-res time |
| `GetTimeMillis` | `uint64_t` | `void` | Get time in ms |
| `GetTimeMicros` | `uint64_t` | `void` | Get time in μs |
| `GetTimeNanos` | `uint64_t` | `void` | Get time in ns |
| `OpenURL` | `bool` | `const std::string& url` | Open URL in browser |
| `ShowMessageBox` | `void` | `const std::string& title, const std::string& message` | Show message box |
| `IsDebuggerAttached` | `bool` | `void` | Check debugger |
| `DebugBreak` | `void` | `void` | Trigger breakpoint |
| `SetEnvironmentVariable` | `void` | `const std::string& name, const std::string& value` | Set env var |
| `GetEnvironmentVariable` | `std::string` | `const std::string& name` | Get env var |

---

## `platform/file_system.h`

### `struct FileInfo`
| Member | Type | Description |
|--------|------|-------------|
| `name` | `std::string` | File name |
| `path` | `std::string` | Full path |
| `size` | `size_t` | File size |
| `isDirectory` | `bool` | Is directory |
| `isHidden` | `bool` | Is hidden |
| `isReadOnly` | `bool` | Is read-only |
| `createdTime` | `uint64_t` | Creation timestamp |
| `modifiedTime` | `uint64_t` | Last modified timestamp |
| `accessedTime` | `uint64_t` | Last accessed timestamp |

### `class FileSystem`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Exists` | `bool` | `const std::string& path` | Check exists |
| `IsFile` | `bool` | `const std::string& path` | Check is file |
| `IsDirectory` | `bool` | `const std::string& path` | Check is directory |
| `CreateDirectory` | `bool` | `const std::string& path` | Create directory |
| `CreateDirectories` | `bool` | `const std::string& path` | Create path recursively |
| `Delete` | `bool` | `const std::string& path` | Delete file |
| `DeleteDirectory` | `bool` | `const std::string& path` | Delete directory |
| `Move` | `bool` | `const std::string& from, const std::string& to` | Move file |
| `Copy` | `bool` | `const std::string& from, const std::string& to` | Copy file |
| `Rename` | `bool` | `const std::string& from, const std::string& to` | Rename file |
| `GetFileSize` | `size_t` | `const std::string& path` | Get file size |
| `GetFileInfo` | `FileInfo` | `const std::string& path` | Get file info |
| `ListDirectory` | `std::vector<FileInfo>` | `const std::string& path` | List directory |
| `ReadFile` | `std::string` | `const std::string& path` | Read text file |
| `ReadBinaryFile` | `std::vector<uint8_t>` | `const std::string& path` | Read binary file |
| `WriteFile` | `bool` | `const std::string& path, const std::string& content` | Write text file |
| `WriteBinaryFile` | `bool` | `const std::string& path, const std::vector<uint8_t>& content` | Write binary file |
| `AppendFile` | `bool` | `const std::string& path, const std::string& content` | Append to file |
| `GetCurrentDirectory` | `std::string` | `void` | Get current dir |
| `SetCurrentDirectory` | `void` | `const std::string& path` | Set current dir |
| `GetAbsolutePath` | `std::string` | `const std::string& path` | Get absolute path |
| `GetRelativePath` | `std::string` | `const std::string& path, const std::string& base` | Get relative path |
| `GetParentDirectory` | `std::string` | `const std::string& path` | Get parent dir |
| `GetFileName` | `std::string` | `const std::string& path` | Get file name |
| `GetFileExtension` | `std::string` | `const std::string& path` | Get extension |
| `GetFileNameWithoutExtension` | `std::string` | `const std::string& path` | Get name without ext |
| `JoinPath` | `std::string` | `const std::string& a, const std::string& b` | Join paths |
| `NormalizePath` | `std::string` | `const std::string& path` | Normalize path |
| `WatchDirectory` | `void` | `const std::string& path, std::function<void(const std::string&, bool)> callback` | Watch for changes |
| `StopWatching` | `void` | `const std::string& path` | Stop watching |

---

## `platform/thread.h`

### `class Thread`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Thread` | (ctor) | `std::function<void()> func` | Constructor |
| `Start` | `void` | `void` | Start thread |
| `Join` | `void` | `void` | Wait for completion |
| `Detach` | `void` | `void` | Detach thread |
| `IsRunning` | `bool` | `void` | Check running |
| `GetID` | `uint64_t` | `void` | Get thread ID |
| `SetName` | `void` | `const std::string& name` | Set thread name |
| `GetName` | `std::string` | `void` | Get thread name |
| `SetPriority` | `void` | `int priority` | Set priority |
| `GetPriority` | `int` | `void` | Get priority |
| `SetAffinity` | `void` | `uint64_t mask` | Set CPU affinity |
| `GetAffinity` | `uint64_t` | `void` | Get affinity |

### `class Mutex`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Lock` | `void` | `void` | Lock mutex |
| `Unlock` | `void` | `void` | Unlock mutex |
| `TryLock` | `bool` | `void` | Try lock |
| `IsLocked` | `bool` | `void` | Check locked |

### `class ScopedLock`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `ScopedLock` | (ctor) | `Mutex& mutex` | Lock on construct |
| `~ScopedLock` | (dtor) | `void` | Unlock on destroy |

### `class ConditionVariable`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Wait` | `void` | `Mutex& mutex` | Wait for signal |
| `WaitFor` | `bool` | `Mutex& mutex, uint32_t milliseconds` | Wait with timeout |
| `NotifyOne` | `void` | `void` | Wake one waiter |
| `NotifyAll` | `void` | `void` | Wake all waiters |

---

## `platform/timer.h`

### `class Timer`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Timer` | (ctor) | `void` | Constructor |
| `Start` | `void` | `void` | Start timer |
| `Stop` | `void` | `void` | Stop timer |
| `Reset` | `void` | `void` | Reset timer |
| `IsRunning` | `bool` | `void` | Check running |
| `GetElapsed` | `float` | `void` | Get elapsed seconds |
| `GetElapsedMillis` | `float` | `void` | Get elapsed milliseconds |
| `GetElapsedMicros` | `float` | `void` | Get elapsed microseconds |
| `SetCallback` | `void` | `std::function<void()> callback` | Set callback |
| `SetInterval` | `void` | `float seconds` | Set interval |
| `GetInterval` | `float` | `void` | Get interval |
| `SetRepeating` | `void` | `bool repeat` | Set repeating |
| `IsRepeating` | `bool` | `void` | Check repeating |
| `SetOneShot` | `void` | `void` | Set one-shot |
| `Pause` | `void` | `void` | Pause |
| `Resume` | `void` | `void` | Resume |
| `IsPaused` | `bool` | `void` | Check paused |

### `class Stopwatch`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Start` | `void` | `void` | Start |
| `Stop` | `void` | `void` | Stop |
| `Reset` | `void` | `void` | Reset |
| `IsRunning` | `bool` | `void` | Check running |
| `GetElapsed` | `float` | `void` | Get elapsed seconds |
| `GetElapsedMillis` | `float` | `void` | Get elapsed ms |
| `GetElapsedMicros` | `float` | `void` | Get elapsed μs |
| `Lap` | `float` | `void` | Record lap, return lap time |
| `GetLapCount` | `size_t` | `void` | Get lap count |
| `GetLapTime` | `float` | `size_t index` | Get specific lap |
| `GetAverageLap` | `float` | `void` | Get average lap |
| `GetMinLap` | `float` | `void` | Get fastest lap |
| `GetMaxLap` | `float` | `void` | Get slowest lap |
