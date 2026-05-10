# Scripting Module API
**Files:** src/scripting/lua_engine.h, src/scripting/script_bindings.h, src/scripting/script_engine.h

## `scripting/lua_engine.h`
```cpp
namespace vge {
```

### `class LuaEngine`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `bool` | `` |
| `Shutdown` | `void` | `` |
| `ExecuteString` | `bool` | `const std::string& code` |
| `ExecuteFile` | `bool` | `const std::string& path` |
| `SetGlobal` | `void` | `const std::string& name, int value` |
| `SetGlobal` | `void` | `const std::string& name, double value` |
| `SetGlobal` | `void` | `const std::string& name, const std::string& value` |

## `scripting/script_bindings.h`
```cpp
namespace vge {
```

## `scripting/script_engine.h`
```cpp
namespace vge {
```

### `class ScriptEngine`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `bool` | `` |
| `Shutdown` | `void` | `` |
| `LoadScript` | `bool` | `const std::string& path` |
| `ExecuteCode` | `bool` | `const std::string& code` |
| `RegisterDefaultBindings` | `void` | `` |
| `Update` | `void` | `float deltaTime` |
