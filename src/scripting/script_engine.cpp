#include "scripting/script_engine.h"
#include "scripting/lua_engine.h"
#include "scripting/lua_bindings.h"
#include "core/logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

namespace vge {

ScriptEngine::ScriptEngine()
    : luaEngine(nullptr)
    , initialized(false)
    , world(nullptr)
    , entityManager(nullptr)
    , uiManager(nullptr)
    , input(nullptr)
    , audioSystem(nullptr)
    , craftingSystem(nullptr)
    , blockRegistry(nullptr)
{}

ScriptEngine::~ScriptEngine() {
    if (initialized) Shutdown();
}

bool ScriptEngine::Initialize(const std::string& gameDirectory) {
    gamePath = gameDirectory;
    scriptsPath = gameDirectory + "/scripts";

    luaEngine = std::make_unique<LuaEngine>();
    if (!luaEngine->Initialize()) {
        std::cerr << "[ScriptEngine] Failed to initialize Lua engine" << std::endl;
        return false;
    }

    // Setup sandbox
    luaEngine->SetupSandbox(gamePath);

    // Register all engine bindings
    RegisterDefaultBindings();

    initialized = true;
    std::cout << "[ScriptEngine] Initialized with game path: " << gamePath << std::endl;
    return true;
}

void ScriptEngine::Shutdown() {
    // Trigger shutdown callbacks
    OnShutdown();

    if (luaEngine) {
        luaEngine->Shutdown();
        luaEngine.reset();
    }

    loadedScripts.clear();
    callbacks.clear();
    onInitCallbacks.clear();
    onUpdateCallbacks.clear();
    onShutdownCallbacks.clear();

    initialized = false;
    std::cout << "[ScriptEngine] Shutdown complete" << std::endl;
}

bool ScriptEngine::LoadScript(const std::string& path) {
    if (!initialized || !luaEngine) {
        std::cerr << "[ScriptEngine] Not initialized" << std::endl;
        return false;
    }

    std::string fullPath = scriptsPath + "/" + path;

    // Check if file exists
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        std::cerr << "[ScriptEngine] Failed to open script: " << fullPath << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // Execute the script
    if (!luaEngine->ExecuteString(content)) {
        std::cerr << "[ScriptEngine] Error loading " << path << ": " << luaEngine->GetLastError() << std::endl;
        return false;
    }

    // Track loaded script
    bool found = false;
    for (auto& script : loadedScripts) {
        if (script.path == path) {
            script.content = content;
            script.loaded = true;
            found = true;
            break;
        }
    }

    if (!found) {
        ScriptFile sf;
        sf.path = path;
        sf.content = content;
        sf.loaded = true;
        sf.autoReload = true;
        loadedScripts.push_back(sf);
    }

    // Check for game loop callbacks
    if (luaEngine->FunctionExists("OnInit")) {
        onInitCallbacks.push_back("OnInit");
    }
    if (luaEngine->FunctionExists("OnUpdate")) {
        onUpdateCallbacks.push_back("OnUpdate");
    }
    if (luaEngine->FunctionExists("OnShutdown")) {
        onShutdownCallbacks.push_back("OnShutdown");
    }

    std::cout << "[ScriptEngine] Loaded script: " << path << std::endl;
    return true;
}

bool ScriptEngine::ExecuteCode(const std::string& code) {
    if (!initialized || !luaEngine) return false;
    return luaEngine->ExecuteString(code);
}

bool ScriptEngine::ReloadScript(const std::string& path) {
    if (!initialized || !luaEngine) return false;

    std::string fullPath = scriptsPath + "/" + path;
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        std::cerr << "[ScriptEngine] Failed to reload script: " << path << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // Re-execute
    if (!luaEngine->ExecuteString(content)) {
        std::cerr << "[ScriptEngine] Error reloading " << path << ": " << luaEngine->GetLastError() << std::endl;
        return false;
    }

    // Update tracking
    for (auto& script : loadedScripts) {
        if (script.path == path) {
            script.content = content;
            break;
        }
    }

    std::cout << "[ScriptEngine] Reloaded script: " << path << std::endl;
    return true;
}

bool ScriptEngine::ReloadAllScripts() {
    bool allSuccess = true;
    for (auto& script : loadedScripts) {
        if (script.loaded && script.autoReload) {
            if (!ReloadScript(script.path)) {
                allSuccess = false;
            }
        }
    }
    return allSuccess;
}

void ScriptEngine::EnableAutoReload(bool enable) {
    for (auto& script : loadedScripts) {
        script.autoReload = enable;
    }
}

void ScriptEngine::CheckForReloads() {
    for (auto& script : loadedScripts) {
        if (!script.autoReload || !script.loaded) continue;

        std::string fullPath = scriptsPath + "/" + script.path;
        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0) {
            // Simple reload: just re-execute
            // In production, you'd compare modification times
            ReloadScript(script.path);
        }
    }
}

void ScriptEngine::RegisterCallback(const std::string& event, const std::string& functionName) {
    callbacks[event] = {functionName, ""};
}

void ScriptEngine::TriggerCallback(const std::string& event, float deltaTime) {
    if (!initialized || !luaEngine) return;

    auto it = callbacks.find(event);
    if (it != callbacks.end()) {
        luaEngine->PushNumber(static_cast<double>(deltaTime));
        luaEngine->CallFunction(it->second.functionName, 1, 0);
    }
}

void ScriptEngine::TriggerCallback(const std::string& event, const std::string& param) {
    if (!initialized || !luaEngine) return;

    auto it = callbacks.find(event);
    if (it != callbacks.end()) {
        luaEngine->PushString(param);
        luaEngine->CallFunction(it->second.functionName, 1, 0);
    }
}

void ScriptEngine::OnInit() {
    if (!initialized || !luaEngine) return;

    for (const auto& func : onInitCallbacks) {
        luaEngine->CallFunction(func, 0, 0);
    }
}

void ScriptEngine::OnUpdate(float deltaTime) {
    if (!initialized || !luaEngine) return;

    for (const auto& func : onUpdateCallbacks) {
        luaEngine->PushNumber(static_cast<double>(deltaTime));
        luaEngine->CallFunction(func, 1, 0);
    }
}

void ScriptEngine::OnShutdown() {
    if (!initialized || !luaEngine) return;

    for (const auto& func : onShutdownCallbacks) {
        luaEngine->CallFunction(func, 0, 0);
    }
}

void ScriptEngine::RegisterDefaultBindings() {
    if (!luaEngine) return;
    lua_State* L = luaEngine->GetState();
    if (!L) return;

    // Store engine pointer in registry for bindings to access
    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, "__vge_script_engine");

    // Store system pointers
    lua_pushlightuserdata(L, world);
    lua_setfield(L, LUA_REGISTRYINDEX, "__vge_world");

    lua_pushlightuserdata(L, entityManager);
    lua_setfield(L, LUA_REGISTRYINDEX, "__vge_entity_manager");

    lua_pushlightuserdata(L, uiManager);
    lua_setfield(L, LUA_REGISTRYINDEX, "__vge_ui");

    lua_pushlightuserdata(L, input);
    lua_setfield(L, LUA_REGISTRYINDEX, "__vge_input");

    lua_pushlightuserdata(L, audioSystem);
    lua_setfield(L, LUA_REGISTRYINDEX, "__vge_audio");

    lua_pushlightuserdata(L, craftingSystem);
    lua_setfield(L, LUA_REGISTRYINDEX, "__vge_crafting");

    RegisterWorldBindings(L);
    RegisterEntityBindings(L);
    RegisterEventBindings(L);
    RegisterInputBindings(L);
    RegisterUIBindings(L);
    RegisterAudioBindings(L);
    RegisterCraftingBindings(L);
}

void ScriptEngine::RegisterWorldBindings() {
    if (!luaEngine) return;
    RegisterWorldBindings(luaEngine->GetState());
}

void ScriptEngine::RegisterEntityBindings() {
    if (!luaEngine) return;
    RegisterEntityBindings(luaEngine->GetState());
}

void ScriptEngine::RegisterEventBindings() {
    if (!luaEngine) return;
    RegisterEventBindings(luaEngine->GetState());
}

void ScriptEngine::RegisterInputBindings() {
    if (!luaEngine) return;
    RegisterInputBindings(luaEngine->GetState());
}

void ScriptEngine::RegisterUIBindings() {
    if (!luaEngine) return;
    RegisterUIBindings(luaEngine->GetState());
}

void ScriptEngine::RegisterAudioBindings() {
    if (!luaEngine) return;
    RegisterAudioBindings(luaEngine->GetState());
}

void ScriptEngine::RegisterCraftingBindings() {
    if (!luaEngine) return;
    RegisterCraftingBindings(luaEngine->GetState());
}

std::string ScriptEngine::GetLastError() const {
    if (luaEngine) {
        return luaEngine->GetLastError();
    }
    return "Lua engine not initialized";
}

bool ScriptEngine::HasFunction(const std::string& name) const {
    if (!luaEngine) return false;
    return luaEngine->FunctionExists(name);
}

std::vector<std::string> ScriptEngine::GetLoadedScripts() const {
    std::vector<std::string> result;
    for (const auto& script : loadedScripts) {
        if (script.loaded) {
            result.push_back(script.path);
        }
    }
    return result;
}

} // namespace vge
