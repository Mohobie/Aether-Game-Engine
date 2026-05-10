# Game Module API
**Files:** src/game/application.h, src/game/game_state.h, src/game/scene.h, src/game/serializer.h

## `game/application.h`
```cpp
namespace aether {
```

### `class Application`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `initialize` | `bool` | `int width, int height, const std::string& title` |
| `run` | `void` | `` |
| `shutdown` | `void` | `` |
| `isRunning` | `bool` | `` |
| `getDeltaTime` | `float` | `` |

## `game/game_state.h`
```cpp
namespace aether {
```

### `enum class GameStateType`
| Value | Description |
|-------|-------------|
| `Menu` | |
| `Playing` | |
| `Paused` | |
| `Settings` | |
| `Loading` | |

### `class GameStateType`

### `class GameState`

## `game/scene.h`
```cpp
namespace aether {
```

### `class Scene`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `initialize` | `void` | `` |
| `update` | `void` | `float deltaTime` |
| `render` | `void` | `` |
| `setActiveCamera` | `void` | `EntityID cameraEntity` |
| `getActiveCamera` | `EntityID` | `` |

## `game/serializer.h`
```cpp
namespace aether {
```

### `class Serializer`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `saveWorld` | `static bool` | `const World& world, const std::string& path` |
| `loadWorld` | `static bool` | `World& world, const std::string& path` |
| `saveEntities` | `static bool` | `const EntityManager& manager, const std::string& path` |
| `loadEntities` | `static bool` | `EntityManager& manager, const std::string& path` |
| `writeInt32` | `static void` | `std::vector<uint8_t>& data, int32_t value` |
| `writeString` | `static void` | `std::vector<uint8_t>& data, const std::string& value` |
| `readInt32` | `static int32_t` | `const std::vector<uint8_t>& data, size_t& offset` |
