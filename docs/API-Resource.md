# Resource Module API
**Files:** src/resource/asset_loader.h, src/resource/config_manager.h, src/resource/resource_manager.h

## `resource/asset_loader.h`
```cpp
namespace aether {
```

### `class AssetLoader`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `loadText` | `bool` | `const std::string& path, std::string& out` |
| `loadBinary` | `bool` | `const std::string& path, std::vector<uint8_t>& out` |
| `fileExists` | `bool` | `const std::string& path` |
| `setSearchPath` | `void` | `const std::string& path` |

## `resource/config_manager.h`
```cpp
namespace aether {
```

### `class ConfigManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `loadFromFile` | `bool` | `const std::string& path` |
| `saveToFile` | `bool` | `const std::string& path` |
| `setValue` | `void` | `const std::string& key, const ConfigValue& value` |
| `getValue` | `ConfigValue` | `const std::string& key` |
| `hasKey` | `bool` | `const std::string& key` |
| `setDefault` | `void` | `const std::string& key, const ConfigValue& value` |
| `clear` | `void` | `` |

## `resource/resource_manager.h`
```cpp
namespace aether {
```

### `class Resource`

### `class ResourceManager`
