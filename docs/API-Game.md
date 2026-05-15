# Game Module API

**Canonical files:** `src/game/application.h`, `src/game/scene.h`, `src/game/game_state.h`  
**Legacy / inactive for this audit:** `src/game/save_system.h`, `src/game/serializer.h`

This page was corrected during the 2026-05-15 architecture audit. The supported game-facing application facade is `game/application.h` in namespace `vge`.

The current `Application` facade composes the canonical `ui/ui_system.h` stack. The older `ui/ui_manager.h` API remains in-tree only as legacy source material and is not part of the active library target for this session.

## `game/application.h`

```cpp
namespace vge {
```

### `class Application`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Application` | ctor | `` |
| `initialize` | `bool` | `int width, int height, const std::string& title` |
| `run` | `void` | `` |
| `shutdown` | `void` | `` |
| `isRunning` | `bool` | `` |
| `getDeltaTime` | `float` | `` |
| `getWindow` | `Window&` | `` |
| `getRenderer` | `Renderer&` | `` |
| `getCamera` | `Camera&` | `` |
| `getWorld` | `World&` | `` |
| `getAudio` | `AudioEngine&` | `` |
| `getInput` | `Input&` | `` |
| `getEntities` | `EntityManager&` | `` |
| `getUI` | `UIManager&` | `` |
| `getResources` | `ResourceManager&` | `` |

## `game/scene.h`

```cpp
namespace vge {
```

### `class Scene`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Scene` | ctor | `const std::string& name` |
| `initialize` | `void` | `` |
| `update` | `void` | `float deltaTime` |
| `render` | `void` | `` |
| `getEntityManager` | `EntityManager&` | `` |
| `getWorld` | `World&` | `` |
| `getName` | `const std::string&` | `` |
| `setActiveCamera` | `void` | `EntityID cameraEntity` |
| `getActiveCamera` | `EntityID` | `` |

## `game/game_state.h`

```cpp
namespace vge {
```

`game_state.h` remains part of the game-facing layer, but this audit did not expand its full API table.

## Legacy note

`game/save_system.h` and `game/serializer.h` are not part of the canonical architecture for this session. The active audited save path is `core/save_system.*` for direct world/chunk files, `voxel/world_serializer.*` for richer world snapshots, and `core/save_game.*` as the current higher-level wrapper.
