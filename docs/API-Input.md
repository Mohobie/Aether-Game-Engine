# Input Module API

**Canonical input path:** `src/platform/input_manager.h`  
**Legacy / inactive path:** `src/input/input_manager.h`, `src/input/key_bindings.h`  
**Compatibility-only path outside the active library target:** `src/platform/input.h`

This page reflects the 2026-05-14 architecture decision: the supported input API in the current build is `vge::Input` from `platform/input_manager.h`. The older `src/input/*` family is retained only as legacy source material.

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

## Legacy note

If you are auditing older docs or examples:

- `src/input/input_manager.h` is not the supported runtime input API.
- `src/platform/input.h` is an older compatibility path used by inactive windowless/engine code, but it is intentionally excluded from the active `voxel_engine_lib` build path.
