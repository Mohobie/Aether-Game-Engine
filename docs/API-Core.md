# Core Module API

**Files:** `core/types.h`, `core/logger.h`, `core/profiler.h`, `core/time_system.h`, `core/save_system.h`, `core/config.h`, `core/achievements.h`, `core/crafting.h`, `core/inventory.h`, `core/raycast.h`, `core/player_controller.h`, `core/utils.h`

---

## `core/types.h`

```cpp
namespace vge {
```

### Type Aliases
| Alias | Type | Description |
|-------|------|-------------|
| `EntityID` | `uint32_t` | Unique entity identifier |
| `ComponentID` | `uint32_t` | Component type identifier |
| `SystemID` | `uint32_t` | System type identifier |
| `AssetID` | `uint64_t` | Asset unique identifier |
| `JobID` | `uint64_t` | Job identifier |
| `EventType` | `uint32_t` | Event type identifier |
| `EventListenerID` | `uint64_t` | Event listener identifier |

---

## `core/logger.h`

### `enum class LogLevel`
| Value | Description |
|-------|-------------|
| `Trace` | Most verbose |
| `Debug` | Debug info |
| `Info` | General info |
| `Warning` | Warnings |
| `Error` | Errors |
| `Fatal` | Fatal errors |

### `class Logger`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `Logger&` | `void` | Singleton |
| `SetLevel` | `void` | `LogLevel level` | Set minimum level |
| `GetLevel` | `LogLevel` | `void` | Get current level |
| `SetOutputFile` | `void` | `const std::string& path` | File output |
| `EnableConsole` | `void` | `bool enabled` | Console output |
| `EnableFile` | `void` | `bool enabled` | File output toggle |
| `Log` | `void` | `LogLevel level, const std::string& msg` | Generic log |
| `Trace` | `void` | `const std::string& msg` | Trace log |
| `Debug` | `void` | `const std::string& msg` | Debug log |
| `Info` | `void` | `const std::string& msg` | Info log |
| `Warning` | `void` | `const std::string& msg` | Warning log |
| `Error` | `void` | `const std::string& msg` | Error log |
| `Fatal` | `void` | `const std::string& msg` | Fatal log |
| `Flush` | `void` | `void` | Flush buffers |

---

## `core/profiler.h`

### `class Profiler`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `Profiler&` | `void` | Singleton |
| `BeginFrame` | `void` | `void` | Start frame profiling |
| `EndFrame` | `void` | `void` | End frame profiling |
| `BeginScope` | `void` | `const std::string& name` | Start scope |
| `EndScope` | `void` | `void` | End scope |
| `GetFrameTime` | `float` | `void` | Last frame time (ms) |
| `GetFPS` | `float` | `void` | Current FPS |
| `GetScopeTime` | `float` | `const std::string& name` | Scope time |
| `GetAllScopes` | `std::vector<std::string>` | `void` | All scope names |
| `Reset` | `void` | `void` | Reset all data |
| `Enable` | `void` | `bool enabled` | Enable/disable |
| `IsEnabled` | `bool` | `void` | Check enabled |

---

## `core/time_system.h`

### `class TimeSystem`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `TimeSystem&` | `void` | Singleton |
| `Initialize` | `void` | `void` | Initialize |
| `Update` | `void` | `void` | Update time |
| `GetDeltaTime` | `float` | `void` | Frame delta time |
| `GetTimeScale` | `float` | `void` | Time scale multiplier |
| `SetTimeScale` | `void` | `float scale` | Set time scale |
| `GetTotalTime` | `float` | `void` | Total elapsed time |
| `GetFrameCount` | `uint64_t` | `void` | Total frames |
| `GetFPS` | `float` | `void` | Current FPS |
| `Pause` | `void` | `void` | Pause time |
| `Resume` | `void` | `void` | Resume time |
| `IsPaused` | `bool` | `void` | Check paused |

---

## `core/save_system.h`

### `class SaveSystem`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `SaveSystem&` | `void` | Singleton |
| `SaveWorld` | `bool` | `const World& world, const std::string& path` | Save world |
| `LoadWorld` | `bool` | `World& world, const std::string& path` | Load world |
| `SavePlayer` | `bool` | `const Entity& player, const std::string& path` | Save player |
| `LoadPlayer` | `bool` | `Entity& player, const std::string& path` | Load player |
| `SaveConfig` | `bool` | `const Config& config, const std::string& path` | Save config |
| `LoadConfig` | `bool` | `Config& config, const std::string& path` | Load config |
| `GetSaveFiles` | `std::vector<std::string>` | `void` | List save files |
| `DeleteSave` | `bool` | `const std::string& path` | Delete save |
| `SaveExists` | `bool` | `const std::string& path` | Check exists |

---

## `core/config.h`

### `class Config`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Load` | `bool` | `const std::string& path` | Load from file |
| `Save` | `bool` | `const std::string& path` | Save to file |
| `GetString` | `std::string` | `const std::string& key, const std::string& default = ""` | Get string |
| `GetInt` | `int` | `const std::string& key, int default = 0` | Get int |
| `GetFloat` | `float` | `const std::string& key, float default = 0.0f` | Get float |
| `GetBool` | `bool` | `const std::string& key, bool default = false` | Get bool |
| `SetString` | `void` | `const std::string& key, const std::string& value` | Set string |
| `SetInt` | `void` | `const std::string& key, int value` | Set int |
| `SetFloat` | `void` | `const std::string& key, float value` | Set float |
| `SetBool` | `void` | `const std::string& key, bool value` | Set bool |
| `HasKey` | `bool` | `const std::string& key` | Check key |
| `RemoveKey` | `void` | `const std::string& key` | Remove key |
| `GetKeys` | `std::vector<std::string>` | `void` | All keys |
| `Clear` | `void` | `void` | Clear all |

---

## `core/achievements.h`

### `struct Achievement`
| Member | Type | Description |
|--------|------|-------------|
| `id` | `std::string` | Unique ID |
| `name` | `std::string` | Display name |
| `description` | `std::string` | Description |
| `icon` | `std::string` | Icon path |
| `unlocked` | `bool` | Is unlocked |
| `progress` | `float` | Current progress |
| `target` | `float` | Target value |

### `class AchievementManager`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `AchievementManager&` | `void` | Singleton |
| `Register` | `void` | `const Achievement& achievement` | Register achievement |
| `Unlock` | `void` | `const std::string& id` | Unlock by ID |
| `UpdateProgress` | `void` | `const std::string& id, float progress` | Update progress |
| `IsUnlocked` | `bool` | `const std::string& id` | Check unlocked |
| `Get` | `const Achievement*` | `const std::string& id` | Get achievement |
| `GetAll` | `std::vector<Achievement>` | `void` | All achievements |
| `Load` | `void` | `const std::string& path` | Load from file |
| `Save` | `void` | `const std::string& path` | Save to file |
| `Reset` | `void` | `void` | Reset all |

---

## `core/crafting.h`

### `struct CraftingRecipe`
| Member | Type | Description |
|--------|------|-------------|
| `id` | `std::string` | Recipe ID |
| `result` | `std::string` | Result item |
| `resultCount` | `int` | Output count |
| `ingredients` | `std::vector<std::pair<std::string, int>>` | Required items |
| `craftingTime` | `float` | Time to craft |

### `class CraftingSystem`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `CraftingSystem&` | `void` | Singleton |
| `RegisterRecipe` | `void` | `const CraftingRecipe& recipe` | Add recipe |
| `CanCraft` | `bool` | `const std::string& recipeId, const Inventory& inv` | Check craftable |
| `Craft` | `bool` | `const std::string& recipeId, Inventory& inv` | Execute craft |
| `GetRecipe` | `const CraftingRecipe*` | `const std::string& id` | Get recipe |
| `GetAllRecipes` | `std::vector<CraftingRecipe>` | `void` | All recipes |
| `GetAvailable` | `std::vector<CraftingRecipe>` | `const Inventory& inv` | Available recipes |
| `LoadRecipes` | `void` | `const std::string& path` | Load from file |

---

## `core/inventory.h`

### `struct ItemStack`
| Member | Type | Description |
|--------|------|-------------|
| `itemId` | `std::string` | Item identifier |
| `count` | `int` | Stack count |
| `maxStack` | `int` | Max stack size |
| `metadata` | `std::string` | Item metadata |

### `class Inventory`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Inventory` | (ctor) | `size_t capacity = 36` | Constructor |
| `AddItem` | `bool` | `const std::string& itemId, int count = 1` | Add item |
| `RemoveItem` | `bool` | `const std::string& itemId, int count = 1` | Remove item |
| `HasItem` | `bool` | `const std::string& itemId, int count = 1` | Check has item |
| `GetCount` | `int` | `const std::string& itemId` | Get item count |
| `GetSlot` | `const ItemStack*` | `size_t slot` | Get slot |
| `SetSlot` | `void` | `size_t slot, const ItemStack& item` | Set slot |
| `ClearSlot` | `void` | `size_t slot` | Clear slot |
| `IsFull` | `bool` | `void` | Check full |
| `IsEmpty` | `bool` | `void` | Check empty |
| `GetCapacity` | `size_t` | `void` | Get capacity |
| `GetUsedSlots` | `size_t` | `void` | Used slot count |
| `GetItems` | `std::vector<ItemStack>` | `void` | All items |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `void` | `const std::string& data` | Deserialize |
| `Clear` | `void` | `void` | Clear all |

---

## `core/raycast.h`

### `struct RaycastHit`
| Member | Type | Description |
|--------|------|-------------|
| `hit` | `bool` | Did hit |
| `point` | `Vec3` | Hit point |
| `normal` | `Vec3` | Hit normal |
| `distance` | `float` | Hit distance |
| `blockPosition` | `Vec3` | Block position |
| `blockType` | `BlockType` | Block type |

### `class Raycast`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `World` | `bool` | `const World& world, const Vec3& origin, const Vec3& dir, float maxDist, RaycastHit& outHit` | Raycast against world |
| `AABB` | `bool` | `const Vec3& origin, const Vec3& dir, const AABB& aabb, float& outDist` | Raycast AABB |
| `Plane` | `bool` | `const Vec3& origin, const Vec3& dir, const Vec3& normal, float distance, float& outDist` | Raycast plane |

---

## `core/player_controller.h`

### `class PlayerController`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `PlayerController` | (ctor) | `Entity* entity` | Constructor |
| `Update` | `void` | `float deltaTime, Input& input, World& world` | Update |
| `SetPosition` | `void` | `const Vec3& pos` | Set position |
| `GetPosition` | `Vec3` | `void` | Get position |
| `SetRotation` | `void` | `const Vec3& rot` | Set rotation |
| `GetRotation` | `Vec3` | `void` | Get rotation |
| `Move` | `void` | `const Vec3& delta` | Move relative |
| `Rotate` | `void` | `float yaw, float pitch` | Rotate view |
| `Jump` | `void` | `void` | Jump |
| `IsGrounded` | `bool` | `void` | Check grounded |
| `SetSpeed` | `void` | `float speed` | Set move speed |
| `GetSpeed` | `float` | `void` | Get move speed |
| `SetFlyMode` | `void` | `bool enabled` | Toggle fly mode |
| `IsFlying` | `bool` | `void` | Check flying |

---

## `core/utils.h`

### Utility Functions
| Function | Return Type | Parameters | Description |
|----------|-------------|------------|-------------|
| `Clamp` | `T` | `T value, T min, T max` | Clamp value |
| `Lerp` | `T` | `T a, T b, float t` | Linear interpolate |
| `Min` | `T` | `T a, T b` | Minimum |
| `Max` | `T` | `T a, T b` | Maximum |
| `Abs` | `T` | `T value` | Absolute value |
| `Sign` | `int` | `T value` | Sign (-1, 0, 1) |
| `RandomFloat` | `float` | `float min = 0.0f, float max = 1.0f` | Random float |
| `RandomInt` | `int` | `int min, int max` | Random int |
| `RandomVec3` | `Vec3` | `float min = -1.0f, float max = 1.0f` | Random vector |
| `ToRadians` | `float` | `float degrees` | Degrees to radians |
| `ToDegrees` | `float` | `float radians` | Radians to degrees |
| `StringFormat` | `std::string` | `const std::string& fmt, ...` | Format string |
| `SplitString` | `std::vector<std::string>` | `const std::string& str, char delimiter` | Split string |
| `TrimString` | `std::string` | `const std::string& str` | Trim whitespace |
| `FileExists` | `bool` | `const std::string& path` | Check file exists |
| `ReadFile` | `std::string` | `const std::string& path` | Read file to string |
| `WriteFile` | `bool` | `const std::string& path, const std::string& content` | Write string to file |
| `GetFileSize` | `size_t` | `const std::string& path` | Get file size |
| `GetTimeStamp` | `std::string` | `void` | Get timestamp string |
| `HashString` | `uint64_t` | `const std::string& str` | Hash string |
