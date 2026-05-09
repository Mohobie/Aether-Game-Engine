#include "scripting/script_engine.h"
#include "scripting/lua_engine.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Stub implementation when Lua is not available
namespace vge {

ScriptEngine::ScriptEngine() : luaEngine(nullptr), initialized(false) {}

ScriptEngine::~ScriptEngine() {
    if (initialized) Shutdown();
}

bool ScriptEngine::Initialize() {
    luaEngine = new LuaEngine();
    if (!luaEngine->Initialize()) {
        std::cerr << "[Script] Failed to initialize Lua engine" << std::endl;
        delete luaEngine;
        luaEngine = nullptr;
        return false;
    }
    
    RegisterDefaultBindings();
    initialized = true;
    
    std::cout << "[Script] Engine initialized" << std::endl;
    return true;
}

void ScriptEngine::Shutdown() {
    if (luaEngine) {
        luaEngine->Shutdown();
        delete luaEngine;
        luaEngine = nullptr;
    }
    initialized = false;
    std::cout << "[Script] Engine shutdown" << std::endl;
}

bool ScriptEngine::LoadScript(const std::string& path) {
    if (!initialized || !luaEngine) return false;
    return luaEngine->ExecuteFile(path);
}

bool ScriptEngine::ExecuteCode(const std::string& code) {
    if (!initialized || !luaEngine) return false;
    return luaEngine->ExecuteString(code);
}

void ScriptEngine::RegisterFunction(const std::string& name, std::function<void()> func) {
    // Would register C++ function to Lua
}

void ScriptEngine::RegisterDefaultBindings() {
    // Would register engine API to Lua
}

void ScriptEngine::Update(float deltaTime) {
    // Would update any script callbacks
}

} // namespace vge