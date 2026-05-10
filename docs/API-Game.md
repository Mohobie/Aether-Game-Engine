# Game Module API

**Files:** `game/game.h`, `game/scene.h`, `game/scene_manager.h`, `game/game_state.h`, `game/level.h`

---

## `game/game.h`

```cpp
namespace vge {
```

### `enum class GameStateType`
| Value | Description |
|-------|-------------|
| `Loading` | Loading state |
| `MainMenu` | Main menu |
| `Playing` | Playing |
| `Paused` | Paused |
| `GameOver` | Game over |
| `Victory` | Victory |
| `Settings` | Settings menu |
| `Credits` | Credits |

### `class Game`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `Game&` | `void` | Singleton |
| `Initialize` | `bool` | `void` | Initialize game |
| `Shutdown` | `void` | `void` | Shutdown game |
| `Run` | `void` | `void` | Main game loop |
| `Quit` | `void` | `void` | Quit game |
| `IsRunning` | `bool` | `void` | Check running |
| `SetState` | `void` | `GameStateType state` | Set game state |
| `GetState` | `GameStateType` | `void` | Get game state |
| `PushState` | `void` | `GameStateType state` | Push state |
| `PopState` | `void` | `void` | Pop state |
| `GetPreviousState` | `GameStateType` | `void` | Get previous state |
| `SetPaused` | `void` | `bool paused` | Set paused |
| `IsPaused` | `bool` | `void` | Check paused |
| `SetTimeScale` | `void` | `float scale` | Set time scale |
| `GetTimeScale` | `float` | `void` | Get time scale |
| `GetDeltaTime` | `float` | `void` | Get delta time |
| `GetTotalTime` | `float` | `void` | Get total time |
| `GetFPS` | `float` | `void` | Get FPS |
| `SetTargetFPS` | `void` | `float fps` | Set target FPS |
| `GetTargetFPS` | `float` | `void` | Get target FPS |
| `SetVSync` | `void` | `bool enabled` | Set VSync |
| `IsVSyncEnabled` | `bool` | `void` | Check VSync |
| `GetWindow` | `Window*` | `void` | Get window |
| `GetRenderer` | `Renderer*` | `void` | Get renderer |
| `GetWorld` | `World*` | `void` | Get world |
| `GetPlayer` | `Entity*` | `void` | Get player entity |
| `SetPlayer` | `void` | `Entity* player` | Set player entity |
| `GetCamera` | `Camera*` | `void` | Get camera |
| `SetCamera` | `void` | `Camera* camera` | Set camera |
| `GetInput` | `Input*` | `void` | Get input |
| `GetAudio` | `AudioEngine*` | `void` | Get audio |
| `GetPhysics` | `PhysicsWorld*` | `void` | Get physics |
| `GetUIManager` | `UIManager*` | `void` | Get UI manager |
| `GetSceneManager` | `SceneManager*` | `void` | Get scene manager |
| `GetEntityManager` | `EntityManager*` | `void` | Get entity manager |
| `GetScriptEngine` | `ScriptEngine*` | `void` | Get script engine |
| `GetConfig` | `Config*` | `void` | Get config |
| `GetSaveSystem` | `SaveSystem*` | `void` | Get save system |
| `GetAchievementManager` | `AchievementManager*` | `void` | Get achievements |
| `GetProfiler` | `Profiler*` | `void` | Get profiler |
| `GetLogger` | `Logger*` | `void` | Get logger |
| `LoadLevel` | `bool` | `const std::string& path` | Load level |
| `UnloadLevel` | `void` | `void` | Unload level |
| `ReloadLevel` | `bool` | `void` | Reload current level |
| `GetCurrentLevel` | `Level*` | `void` | Get current level |
| `SetCurrentLevel` | `void` | `Level* level` | Set current level |
| `SaveGame` | `bool` | `const std::string& slot` | Save game |
| `LoadGame` | `bool` | `const std::string& slot` | Load game |
| `NewGame` | `void` | `void` | Start new game |
| `GetSaveSlots` | `std::vector<std::string>` | `void` | Get save slots |
| `SetGameSpeed` | `void` | `float speed` | Set game speed |
| `GetGameSpeed` | `float` | `void` | Get game speed |
| `SetDifficulty` | `void` | `int difficulty` | Set difficulty |
| `GetDifficulty` | `int` | `void` | Get difficulty |
| `SetGameMode` | `void` | `const std::string& mode` | Set game mode |
| `GetGameMode` | `std::string` | `void` | Get game mode |
| `IsMultiplayer` | `bool` | `void` | Check multiplayer |
| `SetMultiplayer` | `void` | `bool multiplayer` | Set multiplayer |
| `GetNetworkEngine` | `NetworkEngine*` | `void` | Get network |
| `GetFrameCount` | `uint64_t` | `void` | Get frame count |
| `GetVersion` | `std::string` | `void` | Get game version |
| `SetVersion` | `void` | `const std::string& version` | Set version |
| `GetBuildDate` | `std::string` | `void` | Get build date |
| `SetBuildDate` | `void` | `const std::string& date` | Set build date |

---

## `game/scene.h`

### `class Scene`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Scene` | (ctor) | `const std::string& name` | Constructor |
| `GetName` | `std::string` | `void` | Get name |
| `SetName` | `void` | `const std::string& name` | Set name |
| `Initialize` | `void` | `void` | Initialize scene |
| `Shutdown` | `void` | `void` | Shutdown scene |
| `Load` | `bool` | `void` | Load scene |
| `Unload` | `void` | `void` | Unload scene |
| `IsLoaded` | `bool` | `void` | Check loaded |
| `Update` | `void` | `float deltaTime` | Update scene |
| `FixedUpdate` | `void` | `float fixedDeltaTime` | Fixed update |
| `LateUpdate` | `void` | `float deltaTime` | Late update |
| `Render` | `void` | `void` | Render scene |
| `OnEnter` | `void` | `void` | On enter |
| `OnExit` | `void` | `void` | On exit |
| `OnPause` | `void` | `void` | On pause |
| `OnResume` | `void` | `void` | On resume |
| `SetActive` | `void` | `bool active` | Set active |
| `IsActive` | `bool` | `void` | Check active |
| `GetWorld` | `World*` | `void` | Get world |
| `SetWorld` | `void` | `World* world` | Set world |
| `GetCamera` | `Camera*` | `void` | Get camera |
| `SetCamera` | `void` | `Camera* camera` | Set camera |
| `GetEntityManager` | `EntityManager*` | `void` | Get entity manager |
| `GetRootEntity` | `Entity*` | `void` | Get root entity |
| `CreateEntity` | `Entity*` | `const std::string& name` | Create entity |
| `DestroyEntity` | `void` | `Entity* entity` | Destroy entity |
| `FindEntity` | `Entity*` | `const std::string& name` | Find by name |
| `FindEntitiesWithTag` | `std::vector<Entity*>` | `const std::string& tag` | Find by tag |
| `GetAllEntities` | `std::vector<Entity*>` | `void` | All entities |
| `SetGravity` | `void` | `const Vec3& gravity` | Set gravity |
| `GetGravity` | `Vec3` | `void` | Get gravity |
| `SetAmbientLight` | `void` | `const Vec3& color, float intensity` | Set ambient |
| `GetAmbientLight` | `Vec3` | `void` | Get ambient color |
| `GetAmbientIntensity` | `float` | `void` | Get ambient intensity |
| `SetSkyColor` | `void` | `const Vec3& color` | Set sky color |
| `GetSkyColor` | `Vec3` | `void` | Get sky color |
| `SetFog` | `void` | `bool enabled, const Vec3& color, float density` | Set fog |
| `IsFogEnabled` | `bool` | `void` | Check fog |
| `GetFogColor` | `Vec3` | `void` | Get fog color |
| `GetFogDensity` | `float` | `void` | Get fog density |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `bool` | `const std::string& data` | Deserialize |
| `GetEntityCount` | `size_t` | `void` | Entity count |
| `GetLoadedAssetCount` | `size_t` | `void` | Loaded asset count |

---

## `game/scene_manager.h`

### `class SceneManager`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `SceneManager&` | `void` | Singleton |
| `Initialize` | `void` | `void` | Initialize |
| `Shutdown` | `void` | `void` | Shutdown |
| `RegisterScene` | `void` | `const std::string& name, std::function<Scene*()> factory` | Register scene |
| `UnregisterScene` | `void` | `const std::string& name` | Unregister |
| `LoadScene` | `bool` | `const std::string& name` | Load scene |
| `UnloadScene` | `void` | `const std::string& name` | Unload scene |
| `ReloadScene` | `bool` | `const std::string& name` | Reload scene |
| `GetScene` | `Scene*` | `const std::string& name` | Get scene |
| `GetCurrentScene` | `Scene*` | `void` | Get current |
| `GetPreviousScene` | `Scene*` | `void` | Get previous |
| `GetSceneNames` | `std::vector<std::string>` | `void` | All scene names |
| `IsSceneLoaded` | `bool` | `const std::string& name` | Check loaded |
| `TransitionTo` | `void` | `const std::string& name, float duration = 1.0f` | Transition |
| `SetTransitionColor` | `void` | `const Vec3& color` | Set transition color |
| `IsTransitioning` | `bool` | `void` | Check transitioning |
| `GetTransitionProgress` | `float` | `void` | Get transition progress |
| `Update` | `void` | `float deltaTime` | Update |
| `Render` | `void` | `void` | Render |
| `SetOnSceneLoad` | `void` | `std::function<void(Scene*)> callback` | Set load callback |
| `SetOnSceneUnload` | `void` | `std::function<void(Scene*)> callback` | Set unload callback |
| `SetOnTransitionStart` | `void` | `std::function<void()> callback` | Set transition start |
| `SetOnTransitionEnd` | `void` | `std::function<void()> callback` | Set transition end |

---

## `game/game_state.h`

### `class GameState`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GameState` | (ctor) | `GameStateType type` | Constructor |
| `GetType` | `GameStateType` | `void` | Get type |
| `Enter` | `void` | `void` | Enter state |
| `Exit` | `void` | `void` | Exit state |
| `Update` | `void` | `float deltaTime` | Update |
| `FixedUpdate` | `void` | `float fixedDeltaTime` | Fixed update |
| `LateUpdate` | `void` | `float deltaTime` | Late update |
| `Render` | `void` | `void` | Render |
| `OnPause` | `void` | `void` | On pause |
| `OnResume` | `void` | `void` | On resume |
| `HandleInput` | `void` | `Input& input` | Handle input |
| `IsActive` | `bool` | `void` | Check active |
| `SetActive` | `void` | `bool active` | Set active |
| `GetName` | `std::string` | `void` | Get name |
| `SetName` | `void` | `const std::string& name` | Set name |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `void` | `const std::string& data` | Deserialize |

---

## `game/level.h`

### `struct LevelInfo`
| Member | Type | Description |
|--------|------|-------------|
| `name` | `std::string` | Level name |
| `description` | `std::string` | Description |
| `author` | `std::string` | Author |
| `version` | `std::string` | Version |
| `thumbnail` | `std::string` | Thumbnail path |
| `sceneName` | `std::string` | Scene to load |
| `worldGenerator` | `std::string` | World generator config |
| `spawnPoint` | `Vec3` | Player spawn |
| `timeOfDay` | `float` | Initial time |
| `weather` | `std::string` | Weather type |
| `difficulty` | `int` | Difficulty level |
| `maxPlayers` | `int` | Max players |
| `gameMode` | `std::string` | Game mode |

### `class Level`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Level` | (ctor) | `void` | Constructor |
| `Load` | `bool` | `const std::string& path` | Load from file |
| `Save` | `bool` | `const std::string& path` | Save to file |
| `GetInfo` | `LevelInfo` | `void` | Get info |
| `SetInfo` | `void` | `const LevelInfo& info` | Set info |
| `GetName` | `std::string` | `void` | Get name |
| `SetName` | `void` | `const std::string& name` | Set name |
| `GetDescription` | `std::string` | `void` | Get description |
| `SetDescription` | `void` | `const std::string& desc` | Set description |
| `GetScene` | `Scene*` | `void` | Get scene |
| `SetScene` | `void` | `Scene* scene` | Set scene |
| `GetWorld` | `World*` | `void` | Get world |
| `SetWorld` | `void` | `World* world` | Set world |
| `GetSpawnPoint` | `Vec3` | `void` | Get spawn point |
| `SetSpawnPoint` | `void` | `const Vec3& point` | Set spawn point |
| `GetTimeOfDay` | `float` | `void` | Get time |
| `SetTimeOfDay` | `void` | `float time` | Set time |
| `GetWeather` | `std::string` | `void` | Get weather |
| `SetWeather` | `void` | `const std::string& weather` | Set weather |
| `GetDifficulty` | `int` | `void` | Get difficulty |
| `SetDifficulty` | `void` | `int difficulty` | Set difficulty |
| `GetMaxPlayers` | `int` | `void` | Get max players |
| `SetMaxPlayers` | `void` | `int max` | Set max players |
| `GetGameMode` | `std::string` | `void` | Get game mode |
| `SetGameMode` | `void` | `const std::string& mode` | Set game mode |
| `IsLoaded` | `bool` | `void` | Check loaded |
| `Unload` | `void` | `void` | Unload |
| `GetPath` | `std::string` | `void` | Get file path |
| `SetPath` | `void` | `const std::string& path` | Set file path |
| `GetThumbnail` | `Texture*` | `void` | Get thumbnail |
| `SetThumbnail` | `void` | `Texture* thumbnail` | Set thumbnail |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `bool` | `const std::string& data` | Deserialize |
