#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>

namespace vge {

class LuaEngine;
class World;
class EntityManager;
class UIManager;
class Input;
class AudioSystem;
class CraftingSystem;
class BlockRegistry;

// Script callback info
struct ScriptCallback {
    std::string functionName;
    std::string scriptPath;
};

// Hot-reload tracking
struct ScriptFile {
    std::string path;
    std::string content;
    bool loaded;
    bool autoReload;
};

class ScriptEngine {
private:
    std::unique_ptr<LuaEngine> luaEngine;
    bool initialized;
    std::string scriptsPath;
    std::string gamePath;

    // Loaded scripts
    std::vector<ScriptFile> loadedScripts;
    std::unordered_map<std::string, ScriptCallback> callbacks;

    // Engine system pointers (for bindings)
    World* world;
    EntityManager* entityManager;
    UIManager* uiManager;
    Input* input;
    AudioSystem* audioSystem;
    CraftingSystem* craftingSystem;
    BlockRegistry* blockRegistry;

    // Game loop callbacks
    std::vector<std::string> onInitCallbacks;
    std::vector<std::string> onUpdateCallbacks;
    std::vector<std::string> onShutdownCallbacks;

public:
    ScriptEngine();
    ~ScriptEngine();

    // Lifecycle
    bool Initialize(const std::string& gameDirectory);
    void Shutdown();
    bool IsInitialized() const { return initialized; }

    // Script loading
    bool LoadScript(const std::string& path);
    bool ExecuteCode(const std::string& code);
    bool ReloadScript(const std::string& path);
    bool ReloadAllScripts();

    // Hot reload
    void EnableAutoReload(bool enable);
    void CheckForReloads();

    // Callbacks
    void RegisterCallback(const std::string& event, const std::string& functionName);
    void TriggerCallback(const std::string& event, float deltaTime = 0.0f);
    void TriggerCallback(const std::string& event, const std::string& param);

    // Game loop hooks
    void OnInit();
    void OnUpdate(float deltaTime);
    void OnShutdown();

    // Bindings registration
    void RegisterDefaultBindings();
    void RegisterWorldBindings();
    void RegisterEntityBindings();
    void RegisterEventBindings();
    void RegisterInputBindings();
    void RegisterUIBindings();
    void RegisterAudioBindings();
    void RegisterCraftingBindings();

    // System setters
    void SetWorld(World* w) { world = w; }
    void SetEntityManager(EntityManager* em) { entityManager = em; }
    void SetUIManager(UIManager* ui) { uiManager = ui; }
    void SetInput(Input* in) { input = in; }
    void SetAudioSystem(AudioSystem* audio) { audioSystem = audio; }
    void SetCraftingSystem(CraftingSystem* craft) { craftingSystem = craft; }
    void SetBlockRegistry(BlockRegistry* reg) { blockRegistry = reg; }

    // Access
    LuaEngine* GetLuaEngine() { return luaEngine.get(); }
    std::string GetLastError() const;

    // Utility
    bool HasFunction(const std::string& name) const;
    std::vector<std::string> GetLoadedScripts() const;
};

} // namespace vge
