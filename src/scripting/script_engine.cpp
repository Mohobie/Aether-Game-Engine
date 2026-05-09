#include "script_engine.h"
#include <iostream>
#include <fstream>
#include <sstream>

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
    // Register C++ functions that scripts can call
    luaEngine.RegisterFunction("LogInfo", [](lua_State* L) -> int {
        const char* msg = luaL_checkstring(L, 1);
        std::cout << "[Script] " << msg << std::endl;
        return 0;
    });
    
    luaEngine.RegisterFunction("GetBlock", [](lua_State* L) -> int {
        // Would interface with World to get block type
        int x = (int)luaL_checkinteger(L, 1);
        int y = (int)luaL_checkinteger(L, 2);
        int z = (int)luaL_checkinteger(L, 3);
        lua_pushinteger(L, 1); // Return dirt as placeholder
        return 1;
    });
    
    luaEngine.RegisterFunction("SetBlock", [](lua_State* L) -> int {
        // Would interface with World to set block
        int x = (int)luaL_checkinteger(L, 1);
        int y = (int)luaL_checkinteger(L, 2);
        int z = (int)luaL_checkinteger(L, 3);
        int type = (int)luaL_checkinteger(L, 4);
        std::cout << "[Script] SetBlock(" << x << ", " << y << ", " << z << ", " << type << ")" << std::endl;
        return 0;
    });
}

} // namespace vge