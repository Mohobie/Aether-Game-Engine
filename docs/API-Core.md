# Core Module API
**Files:** src/core/achievements.h, src/core/application.h, src/core/config.h, src/core/crafting.h, src/core/engine.h, src/core/entity.h, src/core/event_system.h, src/core/inventory.h, src/core/job_system.h, src/core/logger.h, src/core/math.h, src/core/memory.h, src/core/player_controller.h, src/core/profiler.h, src/core/raycast.h, src/core/save_system.h, src/core/serializer.h, src/core/time_system.h, src/core/types.h, src/core/utils.h

## `core/achievements.h`
```cpp
namespace vge {
```

### `enum class AchievementType`
| Value | Description |
|-------|-------------|
| `MineBlock` | |
| `PlaceBlock` | |
| `Walk` | |
| `SurviveDays` | |
| `ReachDepth` | |
| `ReachHeight` | |
| `FindDiamond` | |
| `CraftItem` | |
| `KillEnemy` | |
| `Custom` | |

### `class AchievementType`

### `class Achievement`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `UpdateProgress` | `bool` | `int value` |
| `Increment` | `bool` | `int amount = 1` |

### `class AchievementManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `RegisterDefaultAchievements` | `void` | `` |
| `Initialize` | `void` | `` |
| `UpdateProgress` | `bool` | `AchievementType type, int value` |
| `Increment` | `bool` | `AchievementType type, int amount = 1` |
| `GetAchievement` | `const Achievement*` | `const std::string& id` |

## `core/application.h`
```cpp
namespace vge {
```

### `class Application`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `bool` | `` |
| `Run` | `void` | `` |
| `Shutdown` | `void` | `` |

## `core/config.h`
```cpp
namespace vge {
```

### `class Config`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Load` | `bool` | `const std::string& path` |
| `Save` | `bool` | `const std::string& path` |
| `SetDefaultKeybinds` | `void` | `` |

## `core/crafting.h`
```cpp
namespace vge {
```

### `struct CraftingRecipe`
| Member | Type |
|--------|------|
| `outputType` | `BlockType` |
| `outputCount` | `int` |

### `struct CraftingResult`
| Member | Type |
|--------|------|
| `success` | `bool` |
| `outputType` | `BlockType` |
| `outputCount` | `int` |

### `class CraftingGrid`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `SetItem` | `void` | `int x, int y, BlockType type` |
| `GetItem` | `BlockType` | `int x, int y` |
| `Clear` | `void` | `` |
| `IsEmpty` | `bool` | `` |

### `class CraftingSystem`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `InitializeRecipes` | `void` | `` |
| `TryCraft` | `CraftingResult` | `const CraftingGrid& grid` |
| `PrintRecipes` | `void` | `` |

## `core/engine.h`
```cpp
namespace vge {
```

### `class Engine`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Update` | `void` | `float dt` |
| `Render` | `void` | `` |
| `Initialize` | `bool` | `` |
| `Run` | `void` | `` |
| `Shutdown` | `void` | `` |

## `core/entity.h`
```cpp
namespace vge {
```

### `class Component`

### `class Entity`

## `core/event_system.h`
```cpp
namespace aether {
```

### `class EventSystem`

## `core/inventory.h`
```cpp
namespace vge {
```

### `struct InventorySlot`
| Member | Type |
|--------|------|
| `type` | `BlockType` |
| `count` | `int` |

### `class Inventory`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `AddItem` | `bool` | `BlockType type, int amount = 1` |
| `RemoveItem` | `bool` | `int slotIndex, int amount = 1` |
| `HasItem` | `bool` | `BlockType type, int amount = 1` |
| `GetItemCount` | `int` | `BlockType type` |
| `SelectSlot` | `void` | `int index` |

## `core/job_system.h`
```cpp
namespace aether {
```

### `class JobSystem`

## `core/logger.h`
```cpp
namespace vge {
```

### `enum class LogLevel`
| Value | Description |
|-------|-------------|
| `Debug` | |
| `Info` | |
| `Warning` | |
| `Error` | |
| `Fatal` | |

### `class LogLevel`

### `class Logger`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `SetLogLevel` | `void` | `LogLevel level` |
| `Log` | `void` | `LogLevel level, const std::string& message` |
| `Info` | `static void` | `const std::string& msg` |
| `Error` | `static void` | `const std::string& msg` |
| `Debug` | `static void` | `const std::string& msg` |

## `core/math.h`
```cpp
namespace aether {
```

### `struct Vec2`

### `struct Vec3`

### `struct Mat4`

## `core/memory.h`
```cpp
namespace aether {
```

### `struct FreeNode`
| Member | Type |
|--------|------|
| `next` | `FreeNode*` |

### `class PoolAllocator`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `allocate` | `void*` | `` |
| `deallocate` | `void` | `void* ptr` |

## `core/player_controller.h`
```cpp
namespace vge {
```

### `class PlayerController`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `GetLookDirection` | `Vec3` | `` |
| `Update` | `void` | `float deltaTime, Input& input, World& world` |
| `CheckCollision` | `bool` | `const Vec3& pos, World& world` |
| `PlaceBlock` | `bool` | `World& world, BlockType type` |
| `BreakBlock` | `bool` | `World& world` |
| `GetPosition` | `Vec3` | `` |
| `SetPosition` | `void` | `const Vec3& pos` |
| `GetYaw` | `float` | `` |
| `GetPitch` | `float` | `` |

## `core/profiler.h`
```cpp
namespace aether {
```

### `struct ProfileResult`
| Member | Type |
|--------|------|
| `threadID` | `size_t` |

### `class Profiler`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `beginSession` | `void` | `const std::string& name` |
| `endSession` | `void` | `` |
| `writeProfile` | `void` | `const ProfileResult& result` |

### `class Timer`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `stop` | `void` | `` |

## `core/raycast.h`
```cpp
namespace vge {
```

### `struct RaycastHit`
| Member | Type |
|--------|------|
| `hit` | `bool` |
| `position` | `Vec3` |
| `blockPosition` | `Vec3` |
| `normal` | `Vec3` |
| `distance` | `float` |
| `blockType` | `BlockType` |

### `class Raycast`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Cast` | `RaycastHit` | `const Vec3& origin, const Vec3& direction, World& world` |
| `PlaceBlock` | `bool` | `const Vec3& origin, const Vec3& direction, World& world, BlockType type` |
| `RemoveBlock` | `bool` | `const Vec3& origin, const Vec3& direction, World& world` |

## `core/save_system.h`
```cpp
namespace vge {
```

### `struct SaveHeader`
| Member | Type |
|--------|------|
| `magic` | `uint32_t` |
| `version` | `uint32_t` |
| `chunkCount` | `uint32_t` |

### `struct ChunkData`

### `class SaveSystem`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `SaveWorld` | `static bool` | `const World& world, const std::string& path` |
| `LoadWorld` | `static bool` | `World& world, const std::string& path` |
| `SaveChunk` | `static bool` | `const Chunk& chunk, std::ofstream& file` |
| `LoadChunk` | `static bool` | `Chunk& chunk, std::ifstream& file` |

## `core/serializer.h`
```cpp
namespace aether {
```

### `class Serializer`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `writeInt32` | `void` | `int32_t value` |
| `writeInt64` | `void` | `int64_t value` |
| `writeFloat` | `void` | `float value` |
| `writeDouble` | `void` | `double value` |
| `writeString` | `void` | `const std::string& value` |
| `writeBytes` | `void` | `const std::vector<uint8_t>& data` |
| `readInt32` | `int32_t` | `` |
| `readInt64` | `int64_t` | `` |
| `readFloat` | `float` | `` |
| `readDouble` | `double` | `` |

## `core/time_system.h`
```cpp
namespace vge {
```

### `enum class WeatherType`
| Value | Description |
|-------|-------------|
| `Clear` | |
| `Rain` | |
| `Storm` | |
| `Snow` | |

### `class WeatherType`

### `class TimeSystem`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `ChangeWeather` | `void` | `` |
| `Update` | `void` | `float deltaTime` |
| `GetSunIntensity` | `float` | `` |
| `GetSunColor` | `Vec3` | `` |

## `core/types.h`
```cpp
namespace vge {
```

## `core/utils.h`
```cpp
namespace aether {
```

### `class Logger`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `info` | `static void` | `const std::string& msg` |
| `error` | `static void` | `const std::string& msg` |
