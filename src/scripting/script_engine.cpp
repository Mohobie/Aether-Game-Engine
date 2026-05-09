#include "scripting/script_engine.h"
#include "scripting/lua_engine.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Stub implementation when Lua is not available
namespace vge {

ScriptEngine::ScriptEngine() : initialized(false) {}

ScriptEngine::~ScriptEngine() {
    if (initialized) Shutdown();
}

bool ScriptEngine::Initialize() {
    if (!luaEngine.Initialize()) {
        std::cerr << "[Script] Failed to initialize Lua engine" << std::endl;
        return false;
    }
    
    RegisterDefaultBindings();
    initialized = true;
    
    std::cout << "[Script] Engine initialized" << std::endl;
    return true;
}

void ScriptEngine::Shutdown() {
    luaEngine.Shutdown();
    initialized = false;
    std::cout << "[Script] Engine shutdown" << std::endl;
}

bool ScriptEngine::LoadScript(const std::string& path) {
    if (!initialized) {
        std::cerr << "[Script] Engine not initialized" << std::endl;
        return false;
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Script] Failed to open: " << path << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return luaEngine.ExecuteString(buffer.str());
}

bool ScriptEngine::ExecuteCode(const std::string& code) {
    if (!initialized) {
        std::cerr << "[Script] Engine not initialized" << std::endl;
        return false;
    }
    
    return luaEngine.ExecuteString(code);
}

void ScriptEngine::RegisterDefaultBindings() {
    // Would register C++ functions that scripts can call
    std::cout << "[Script] Registered default bindings" << std::endl;
}

} // namespace vge
