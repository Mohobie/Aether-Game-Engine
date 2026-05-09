#include "lua_engine.h"
#include <iostream>

namespace vge {

LuaEngine::LuaEngine() : L(nullptr), initialized(false) {}

LuaEngine::~LuaEngine() {
    if (initialized) Shutdown();
}

bool LuaEngine::Initialize() {
    L = luaL_newstate();
    if (!L) {
        std::cerr << "[Lua] Failed to create Lua state" << std::endl;
        return false;
    }
    
    luaL_openlibs(L);
    initialized = true;
    
    std::cout << "[Lua] Engine initialized" << std::endl;
    return true;
}

void LuaEngine::Shutdown() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
    initialized = false;
    std::cout << "[Lua] Engine shutdown" << std::endl;
}

bool LuaEngine::ExecuteString(const std::string& code) {
    if (!initialized) {
        std::cerr << "[Lua] Engine not initialized" << std::endl;
        return false;
    }
    
    int result = luaL_dostring(L, code.c_str());
    if (result != LUA_OK) {
        std::cerr << "[Lua] Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}

bool LuaEngine::ExecuteFile(const std::string& path) {
    if (!initialized) {
        std::cerr << "[Lua] Engine not initialized" << std::endl;
        return false;
    }
    
    int result = luaL_dofile(L, path.c_str());
    if (result != LUA_OK) {
        std::cerr << "[Lua] Error loading " << path << ": " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}

void LuaEngine::RegisterFunction(const std::string& name, lua_CFunction func) {
    if (!initialized) return;
    lua_pushcfunction(L, func);
    lua_setglobal(L, name.c_str());
}

void LuaEngine::SetGlobal(const std::string& name, int value) {
    if (!initialized) return;
    lua_pushinteger(L, value);
    lua_setglobal(L, name.c_str());
}

void LuaEngine::SetGlobal(const std::string& name, double value) {
    if (!initialized) return;
    lua_pushnumber(L, value);
    lua_setglobal(L, name.c_str());
}

void LuaEngine::SetGlobal(const std::string& name, const std::string& value) {
    if (!initialized) return;
    lua_pushstring(L, value.c_str());
    lua_setglobal(L, name.c_str());
}

} // namespace vge