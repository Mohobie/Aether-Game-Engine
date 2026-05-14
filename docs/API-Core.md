# Core Module API

This page was corrected during the 2026-05-14 architecture audit.

**Canonical core-facing files for the audited architecture split:** `src/core/save_system.h`, `src/core/save_game.h`, `src/core/serializer.h`, `src/core/player_controller.h`, `src/core/logger.h`, `src/core/time_system.h`, `src/core/config.h`, `src/core/types.h`  
**Legacy / inactive for this audit:** `src/core/application.h`, `src/core/entity.h`

The key point for this module is that not every header under `src/core/` is part of the canonical public architecture. `core/application.h` and `core/entity.h` are legacy parallel stacks; the supported application facade is `game/application.h`, and the supported entity layer is `entity/entity.h`.

## Canonical notes

- `core/save_system.h` is the active direct world/chunk binary save API in the current build.
- `core/save_game.h` is also active in the build and wraps `voxel/world_serializer.h` for named save files and quick-save behavior.
- `core/serializer.h` is a generic byte-buffer helper, not the top-level save pipeline.
- `core/player_controller.h` is built against the canonical `platform/input_manager.h` input path.
- `core/application.h` is not the supported application facade for this session.
- `core/entity.h` is not the supported entity layer for this session.

## `core/save_system.h`

```cpp
namespace vge {
```

### Constants

- `SAVE_MAGIC`
- `SAVE_VERSION`

### `struct SaveHeader`
| Member | Type |
|--------|------|
| `magic` | `uint32_t` |
| `version` | `uint32_t` |
| `chunkCount` | `uint32_t` |
| `blockPaletteSize` | `uint32_t` |
| `reserved` | `uint32_t[4]` |

### `struct BlockPaletteEntry`
| Member | Type |
|--------|------|
| `blockId` | `char[64]` |
| `typeId` | `uint16_t` |

### `struct ChunkData`
| Member | Type |
|--------|------|
| `chunkX` | `int` |
| `chunkY` | `int` |
| `chunkZ` | `int` |
| `blocks` | `uint16_t[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]` |

### `class SaveSystem`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `SaveWorld` | `static bool` | `const World& world, const std::string& path` |
| `LoadWorld` | `static bool` | `World& world, const std::string& path` |
| `SaveChunk` | `static bool` | `const Chunk& chunk, std::ofstream& file` |
| `LoadChunk` | `static bool` | `Chunk& chunk, std::ifstream& file` |

## `core/save_game.h`

```cpp
namespace vge {
```

### `struct SaveGameInfo`
| Member | Type |
|--------|------|
| `name` | `std::string` |
| `filePath` | `std::string` |
| `createdDate` | `std::string` |
| `lastPlayedDate` | `std::string` |
| `playTimeMinutes` | `int` |
| `playerPosition` | `Vec3` |
| `worldSeed` | `int` |
| `fileSize` | `int64_t` |

### `class SaveGameManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `void` | `const std::string& saveDir` |
| `SaveGame` | `bool` | `const std::string& saveName, const World& world, const PlayerController& player, float dayTime, int dayCount` |
| `LoadGame` | `bool` | `const std::string& saveName, World& world, PlayerController& player, float& outDayTime, int& outDayCount` |
| `GetSaveList` | `std::vector<SaveGameInfo>` | `` |
| `DeleteSave` | `bool` | `const std::string& saveName` |
| `SaveExists` | `bool` | `const std::string& saveName` |
| `EnableAutoSave` | `void` | `bool enable` |
| `SetAutoSaveInterval` | `void` | `float minutes` |
| `UpdateAutoSave` | `void` | `float deltaTime, const World& world, const PlayerController& player, float dayTime, int dayCount` |
| `QuickSave` | `bool` | `const World& world, const PlayerController& player, float dayTime, int dayCount` |
| `QuickLoad` | `bool` | `World& world, PlayerController& player, float& outDayTime, int& outDayCount` |

## `core/serializer.h`

```cpp
namespace vge {
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
| `readString` | `std::string` | `` |
| `readBytes` | `std::vector<uint8_t>` | `size_t length` |
| `getData` | `const std::vector<uint8_t>&` | `` |
| `setData` | `void` | `const std::vector<uint8_t>& d` |
| `getPosition` | `size_t` | `` |
| `getSize` | `size_t` | `` |

## `core/player_controller.h`

```cpp
namespace vge {
```

### `class PlayerController`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Update` | `void` | `float deltaTime, Input& input, World& world` |
| `CheckCollision` | `bool` | `const Vec3& pos, World& world` |
| `PlaceBlock` | `bool` | `World& world, BlockTypeID type` |
| `BreakBlock` | `bool` | `World& world` |
| `GetPosition` | `Vec3` | `` |
| `SetPosition` | `void` | `const Vec3& pos` |
| `GetYaw` | `float` | `` |
| `GetPitch` | `float` | `` |
| `GetLookDirection` | `Vec3` | `` |

## Legacy note

- `core/application.h` remains in the tree, but `game/application.h` is the canonical application facade.
- `core/entity.h` remains in the tree, but `entity/entity.h` plus `entity/components.h` is the canonical entity path.
- `game/save_system.h` remains in the tree, but the audited build-backed save path is `core/save_system.h` + `voxel/world_serializer.h` + `core/save_game.h`.
