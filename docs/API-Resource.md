# Resource Module API

**Files:** `resource/resource_manager.h`, `resource/asset.h`, `resource/asset_loader.h`, `resource/asset_database.h`, `resource/prefab.h`

---

## `resource/resource_manager.h`

```cpp
namespace vge {
```

### `enum class AssetType`
| Value | Description |
|-------|-------------|
| `Texture` | 2D texture |
| `Mesh` | 3D mesh |
| `Material` | Material |
| `Shader` | Shader |
| `Sound` | Audio |
| `Music` | Music |
| `Font` | Font |
| `Script` | Script |
| `Prefab` | Prefab |
| `Scene` | Scene |
| `Level` | Level |
| `Animation` | Animation |
| `Skeleton` | Skeleton |
| `Unknown` | Unknown type |

### `class ResourceManager`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `ResourceManager&` | `void` | Singleton |
| `Initialize` | `void` | `void` | Initialize |
| `Shutdown` | `void` | `void` | Shutdown |
| `Load` | `AssetID` | `const std::string& path, AssetType type` | Load asset |
| `LoadAsync` | `AssetID` | `const std::string& path, AssetType type` | Load async |
| `Unload` | `void` | `AssetID id` | Unload asset |
| `UnloadAll` | `void` | `void` | Unload all |
| `Reload` | `bool` | `AssetID id` | Reload asset |
| `Get` | `Asset*` | `AssetID id` | Get asset |
| `GetByPath` | `Asset*` | `const std::string& path` | Get by path |
| `GetTexture` | `Texture*` | `AssetID id` | Get texture |
| `GetMesh` | `Mesh*` | `AssetID id` | Get mesh |
| `GetMaterial` | `Material*` | `AssetID id` | Get material |
| `GetShader` | `Shader*` | `AssetID id` | Get shader |
| `GetSound` | `Sound*` | `AssetID id` | Get sound |
| `GetMusic` | `Music*` | `AssetID id` | Get music |
| `GetFont` | `UIFont*` | `AssetID id` | Get font |
| `GetScript` | `Script*` | `AssetID id` | Get script |
| `GetPrefab` | `Prefab*` | `AssetID id` | Get prefab |
| `IsLoaded` | `bool` | `AssetID id` | Check loaded |
| `IsLoading` | `bool` | `AssetID id` | Check loading |
| `WaitForLoad` | `void` | `AssetID id` | Wait for load |
| `WaitForAll` | `void` | `void` | Wait for all |
| `GetLoadProgress` | `float` | `AssetID id` | Get load progress |
| `GetTotalLoadProgress` | `float` | `void` | Get total progress |
| `GetLoadedCount` | `size_t` | `void` | Loaded count |
| `GetLoadingCount` | `size_t` | `void` | Loading count |
| `GetTotalCount` | `size_t` | `void` | Total count |
| `SetSearchPath` | `void` | `const std::string& path` | Set search path |
| `AddSearchPath` | `void` | `const std::string& path` | Add search path |
| `GetSearchPaths` | `std::vector<std::string>` | `void` | Get search paths |
| `SetCacheSize` | `void` | `size_t size` | Set cache size |
| `GetCacheSize` | `size_t` | `void` | Get cache size |
| `GetMemoryUsage` | `size_t` | `void` | Get memory usage |
| `SetAutoUnload` | `void` | `bool enabled` | Set auto unload |
| `IsAutoUnloadEnabled` | `bool` | `void` | Check auto unload |
| `RegisterLoader` | `void` | `AssetType type, std::function<Asset*(const std::string&)> loader` | Register loader |
| `GetAssetPath` | `std::string` | `AssetID id` | Get asset path |
| `GetAssetType` | `AssetType` | `AssetID id` | Get asset type |
| `GetAssetName` | `std::string` | `AssetID id` | Get asset name |
| `GetAllAssets` | `std::vector<AssetID>` | `void` | All assets |
| `GetAssetsByType` | `std::vector<AssetID>` | `AssetType type` | By type |
| `ExportAsset` | `bool` | `AssetID id, const std::string& path` | Export |
| `ImportAsset` | `AssetID` | `const std::string& path, AssetType type` | Import |

---

## `resource/asset.h`

### `struct AssetInfo`
| Member | Type | Description |
|--------|------|-------------|
| `id` | `AssetID` | Asset ID |
| `name` | `std::string` | Asset name |
| `path` | `std::string` | File path |
| `type` | `AssetType` | Asset type |
| `size` | `size_t` | File size |
| `modifiedTime` | `uint64_t` | Last modified |
| `hash` | `uint64_t` | Content hash |
| `references` | `int` | Reference count |
| `memorySize` | `size_t` | Memory usage |

### `class Asset`
Base class for all assets.

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Asset` | (ctor) | `void` | Constructor |
| `GetID` | `AssetID` | `void` | Get ID |
| `GetName` | `std::string` | `void` | Get name |
| `SetName` | `void` | `const std::string& name` | Set name |
| `GetPath` | `std::string` | `void` | Get path |
| `SetPath` | `void` | `const std::string& path` | Set path |
| `GetType` | `AssetType` | `void` | Get type |
| `SetType` | `void` | `AssetType type` | Set type |
| `Load` | `bool` | `const std::string& path` | Load from file |
| `Unload` | `void` | `void` | Unload |
| `Reload` | `bool` | `void` | Reload |
| `IsLoaded` | `bool` | `void` | Check loaded |
| `IsLoading` | `bool` | `void` | Check loading |
| `GetSize` | `size_t` | `void` | Get file size |
| `GetMemorySize` | `size_t` | `void` | Get memory size |
| `GetReferenceCount` | `int` | `void` | Get references |
| `AddReference` | `void` | `void` | Add reference |
| `RemoveReference` | `void` | `void` | Remove reference |
| `GetHash` | `uint64_t` | `void` | Get hash |
| `GetModifiedTime` | `uint64_t` | `void` | Get modified time |
| `SetModifiedTime` | `void` | `uint64_t time` | Set modified time |
| `Serialize` | `std::string` | `void` | Serialize metadata |
| `Deserialize` | `bool` | `const std::string& data` | Deserialize metadata |
| `GetError` | `std::string` | `void` | Get error |
| `HasError` | `bool` | `void` | Check error |
| `ClearError` | `void` | `void` | Clear error |

---

## `resource/asset_loader.h`

### `class AssetLoader`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `AssetLoader` | (ctor) | `void` | Constructor |
| `LoadTexture` | `Texture*` | `const std::string& path` | Load texture |
| `LoadMesh` | `Mesh*` | `const std::string& path` | Load mesh |
| `LoadMaterial` | `Material*` | `const std::string& path` | Load material |
| `LoadShader` | `Shader*` | `const std::string& vertPath, const std::string& fragPath` | Load shader |
| `LoadSound` | `Sound*` | `const std::string& path` | Load sound |
| `LoadMusic` | `Music*` | `const std::string& path` | Load music |
| `LoadFont` | `UIFont*` | `const std::string& path, float size` | Load font |
| `LoadScript` | `Script*` | `const std::string& path` | Load script |
| `LoadPrefab` | `Prefab*` | `const std::string& path` | Load prefab |
| `LoadScene` | `Scene*` | `const std::string& path` | Load scene |
| `LoadLevel` | `Level*` | `const std::string& path` | Load level |
| `LoadAnimation` | `Animation*` | `const std::string& path` | Load animation |
| `LoadSkeleton` | `Skeleton*` | `const std::string& path` | Load skeleton |
| `SetAsync` | `void` | `bool async` | Set async loading |
| `IsAsync` | `bool` | `void` | Check async |
| `SetPriority` | `void` | `int priority` | Set priority |
| `GetPriority` | `int` | `void` | Get priority |
| `GetProgress` | `float` | `void` | Get progress |
| `IsLoading` | `bool` | `void` | Check loading |
| `Wait` | `void` | `void` | Wait for completion |
| `Cancel` | `void` | `void` | Cancel loading |
| `GetError` | `std::string` | `void` | Get error |
| `HasError` | `bool` | `void` | Check error |

---

## `resource/asset_database.h`

### `class AssetDatabase`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `AssetDatabase&` | `void` | Singleton |
| `Initialize` | `void` | `void` | Initialize |
| `Shutdown` | `void` | `void` | Shutdown |
| `Scan` | `void` | `const std::string& directory` | Scan directory |
| `ScanAll` | `void` | `void` | Scan all paths |
| `AddAsset` | `void` | `const AssetInfo& info` | Add asset |
| `RemoveAsset` | `void` | `AssetID id` | Remove asset |
| `UpdateAsset` | `void` | `AssetID id` | Update asset |
| `GetAsset` | `AssetInfo*` | `AssetID id` | Get asset info |
| `GetAssetByPath` | `AssetInfo*` | `const std::string& path` | Get by path |
| `GetAssetByName` | `AssetInfo*` | `const std::string& name` | Get by name |
| `GetAllAssets` | `std::vector<AssetInfo>` | `void` | All assets |
| `GetAssetsByType` | `std::vector<AssetInfo>` | `AssetType type` | By type |
| `GetAssetsByDirectory` | `std::vector<AssetInfo>` | `const std::string& dir` | By directory |
| `Search` | `std::vector<AssetInfo>` | `const std::string& query` | Search |
| `Exists` | `bool` | `AssetID id` | Check exists |
| `Exists` | `bool` | `const std::string& path` | Check exists by path |
| `Import` | `AssetID` | `const std::string& sourcePath, const std::string& destPath, AssetType type` | Import |
| `Export` | `bool` | `AssetID id, const std::string& path` | Export |
| `Move` | `bool` | `AssetID id, const std::string& newPath` | Move |
| `Rename` | `bool` | `AssetID id, const std::string& newName` | Rename |
| `Delete` | `bool` | `AssetID id` | Delete |
| `GetDirectory` | `std::string` | `AssetID id` | Get directory |
| `SetDirectory` | `void` | `AssetID id, const std::string& dir` | Set directory |
| `GetTags` | `std::vector<std::string>` | `AssetID id` | Get tags |
| `AddTag` | `void` | `AssetID id, const std::string& tag` | Add tag |
| `RemoveTag` | `void` | `AssetID id, const std::string& tag` | Remove tag |
| `HasTag` | `bool` | `AssetID id, const std::string& tag` | Check tag |
| `GetDependencies` | `std::vector<AssetID>` | `AssetID id` | Get dependencies |
| `AddDependency` | `void` | `AssetID id, AssetID dependency` | Add dependency |
| `RemoveDependency` | `void` | `AssetID id, AssetID dependency` | Remove dependency |
| `GetDependents` | `std::vector<AssetID>` | `AssetID id` | Get dependents |
| `BuildDependencyGraph` | `void` | `void` | Build dependency graph |
| `GetUnusedAssets` | `std::vector<AssetID>` | `void` | Get unused |
| `CleanUp` | `void` | `void` | Clean up unused |
| `Save` | `bool` | `const std::string& path` | Save database |
| `Load` | `bool` | `const std::string& path` | Load database |
| `Watch` | `void` | `bool enable` | Watch for changes |
| `IsWatching` | `bool` | `void` | Check watching |
| `GetChangeCount` | `size_t` | `void` | Get pending changes |
| `ApplyChanges` | `void` | `void` | Apply pending changes |

---

## `resource/prefab.h`

### `class Prefab`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Prefab` | (ctor) | `void` | Constructor |
| `Create` | `Entity*` | `void` | Instantiate prefab |
| `Create` | `Entity*` | `const Vec3& position` | Instantiate at position |
| `Create` | `Entity*` | `const Vec3& position, const Vec3& rotation` | Instantiate with rotation |
| `Create` | `Entity*` | `const Vec3& position, const Vec3& rotation, const Vec3& scale` | Instantiate with transform |
| `Save` | `bool` | `const std::string& path` | Save prefab |
| `Load` | `bool` | `const std::string& path` | Load prefab |
| `FromEntity` | `void` | `Entity* entity` | Create from entity |
| `GetRootEntity` | `Entity*` | `void` | Get root entity |
| `SetRootEntity` | `void` | `Entity* entity` | Set root entity |
| `GetName` | `std::string` | `void` | Get name |
| `SetName` | `void` | `const std::string& name` | Set name |
| `GetPath` | `std::string` | `void` | Get path |
| `SetPath` | `void` | `const std::string& path` | Set path |
| `IsValid` | `bool` | `void` | Check valid |
| `GetEntityCount` | `size_t` | `void` | Entity count |
| `GetComponentCount` | `size_t` | `void` | Component count |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `bool` | `const std::string& data` | Deserialize |
