# Scripting Module API

**Files:** `scripting/script_engine.h`, `scripting/script.h`, `scripting/lua_bindings.h`, `scripting/script_component.h`

---

## `scripting/script_engine.h`

```cpp
namespace vge {
```

### `class ScriptEngine`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `ScriptEngine&` | `void` | Singleton |
| `Initialize` | `bool` | `void` | Initialize engine |
| `Shutdown` | `void` | `void` | Shutdown engine |
| `ExecuteString` | `bool` | `const std::string& code` | Execute Lua string |
| `ExecuteFile` | `bool` | `const std::string& path` | Execute Lua file |
| `RegisterFunction` | `void` | `const std::string& name, std::function<int(lua_State*)> func` | Register C++ function |
| `RegisterVariable` | `void` | `const std::string& name, int value` | Register int |
| `RegisterVariable` | `void` | `const std::string& name, float value` | Register float |
| `RegisterVariable` | `void` | `const std::string& name, const std::string& value` | Register string |
| `RegisterVariable` | `void` | `const std::string& name, bool value` | Register bool |
| `GetGlobal` | `int` | `const std::string& name` | Push global to stack |
| `SetGlobal` | `void` | `const std::string& name` | Set global from stack |
| `CallFunction` | `bool` | `const std::string& name, int nargs = 0, int nresults = 0` | Call function |
| `GetError` | `std::string` | `void` | Get last error |
| `ClearError` | `void` | `void` | Clear error |
| `SetPackagePath` | `void` | `const std::string& path` | Set module search path |
| `GetPackagePath` | `std::string` | `void` | Get module search path |
| `AddPackagePath` | `void` | `const std::string& path` | Add search path |
| `CollectGarbage` | `void` | `void` | Run GC |
| `GetMemoryUsage` | `size_t` | `void` | Get Lua memory |
| `SetMemoryLimit` | `void` | `size_t limit` | Set memory limit |
| `IsInitialized` | `bool` | `void` | Check initialized |

---

## `scripting/script.h`

### `enum class ScriptState`
| Value | Description |
|-------|-------------|
| `Unloaded` | Not loaded |
| `Loaded` | Loaded but not running |
| `Running` | Currently executing |
| `Paused` | Paused |
| `Error` | Error state |
| `Stopped` | Stopped |

### `class Script`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Script` | (ctor) | `void` | Constructor |
| `Load` | `bool` | `const std::string& path` | Load from file |
| `LoadFromString` | `bool` | `const std::string& code` | Load from string |
| `Reload` | `bool` | `void` | Reload from file |
| `Start` | `bool` | `void` | Start execution |
| `Stop` | `void` | `void` | Stop execution |
| `Pause` | `void` | `void` | Pause |
| `Resume` | `void` | `void` | Resume |
| `Update` | `void` | `float deltaTime` | Update (called per frame) |
| `GetState` | `ScriptState` | `void` | Get state |
| `GetPath` | `std::string` | `void` | Get file path |
| `GetName` | `std::string` | `void` | Get script name |
| `SetName` | `void` | `const std::string& name` | Set name |
| `GetError` | `std::string` | `void` | Get error message |
| `HasError` | `bool` | `void` | Check has error |
| `SetVariable` | `void` | `const std::string& name, int value` | Set int |
| `SetVariable` | `void` | `const std::string& name, float value` | Set float |
| `SetVariable` | `void` | `const std::string& name, const std::string& value` | Set string |
| `SetVariable` | `void` | `const std::string& name, bool value` | Set bool |
| `GetVariableInt` | `int` | `const std::string& name` | Get int |
| `GetVariableFloat` | `float` | `const std::string& name` | Get float |
| `GetVariableString` | `std::string` | `const std::string& name` | Get string |
| `GetVariableBool` | `bool` | `const std::string& name` | Get bool |
| `HasVariable` | `bool` | `const std::string& name` | Check variable |
| `Call` | `bool` | `const std::string& function, int nargs = 0, int nresults = 0` | Call function |
| `IsLoaded` | `bool` | `void` | Check loaded |
| `IsRunning` | `bool` | `void` | Check running |

---

## `scripting/lua_bindings.h`

### `class LuaBindings`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `RegisterAll` | `void` | `ScriptEngine* engine` | Register all bindings |
| `RegisterMath` | `void` | `ScriptEngine* engine` | Register math types |
| `RegisterVec3` | `void` | `ScriptEngine* engine` | Register Vec3 |
| `RegisterMat4` | `void` | `ScriptEngine* engine` | Register Mat4 |
| `RegisterEntity` | `void` | `ScriptEngine* engine` | Register Entity |
| `RegisterComponent` | `void` | `ScriptEngine* engine` | Register Component |
| `RegisterWorld` | `void` | `ScriptEngine* engine` | Register World |
| `RegisterBlock` | `void` | `ScriptEngine* engine` | Register Block |
| `RegisterInput` | `void` | `ScriptEngine* engine` | Register Input |
| `RegisterCamera` | `void` | `ScriptEngine* engine` | Register Camera |
| `RegisterRenderer` | `void` | `ScriptEngine* engine` | Register Renderer |
| `RegisterAudio` | `void` | `ScriptEngine* engine` | Register Audio |
| `RegisterPhysics` | `void` | `ScriptEngine* engine` | Register Physics |
| `RegisterLogger` | `void` | `ScriptEngine* engine` | Register Logger |
| `RegisterTime` | `void` | `ScriptEngine* engine` | Register Time |
| `RegisterEvents` | `void` | `ScriptEngine* engine` | Register Event system |

---

## `scripting/script_component.h`

### `class ScriptComponent`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `ScriptComponent` | (ctor) | `void` | Constructor |
| `SetScript` | `void` | `Script* script` | Set script |
| `GetScript` | `Script*` | `void` | Get script |
| `LoadScript` | `bool` | `const std::string& path` | Load script file |
| `Start` | `void` | `void` | Call Start() |
| `Update` | `void` | `float deltaTime` | Call Update(dt) |
| `LateUpdate` | `void` | `float deltaTime` | Call LateUpdate(dt) |
| `FixedUpdate` | `void` | `float fixedDeltaTime` | Call FixedUpdate(dt) |
| `OnEnable` | `void` | `void` | Call OnEnable() |
| `OnDisable` | `void` | `void` | Call OnDisable() |
| `OnDestroy` | `void` | `void` | Call OnDestroy() |
| `OnCollisionEnter` | `void` | `const Collision& collision` | Call OnCollisionEnter |
| `OnCollisionExit` | `void` | `const Collision& collision` | Call OnCollisionExit |
| `OnTriggerEnter` | `void` | `Collider* other` | Call OnTriggerEnter |
| `OnTriggerExit` | `void` | `Collider* other` | Call OnTriggerExit |
| `OnMouseDown` | `void` | `void` | Call OnMouseDown |
| `OnMouseUp` | `void` | `void` | Call OnMouseUp |
| `OnMouseOver` | `void` | `void` | Call OnMouseOver |
| `OnMouseExit` | `void` | `void` | Call OnMouseExit |
| `SetVariable` | `void` | `const std::string& name, int value` | Set script variable |
| `GetVariableInt` | `int` | `const std::string& name` | Get script variable |
| `HasVariable` | `bool` | `const std::string& name` | Check variable |
| `IsScriptLoaded` | `bool` | `void` | Check script loaded |
| `IsScriptRunning` | `bool` | `void` | Check script running |
| `GetScriptPath` | `std::string` | `void` | Get script path |
| `GetScriptName` | `std::string` | `void` | Get script name |
| `GetScriptError` | `std::string` | `void` | Get error |
| `Reload` | `bool` | `void` | Reload script |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `void` | `const std::string& data` | Deserialize |
