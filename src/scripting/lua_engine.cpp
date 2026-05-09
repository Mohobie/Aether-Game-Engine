#include "scripting/lua_engine.h"
#include "core/logger.h"
#include <iostream>
#include <dlfcn.h>

// Stub implementation when Lua is not available
// Tries to load liblua dynamically if present

namespace vge {

LuaEngine::LuaEngine() : L(nullptr), initialized(false) {}

LuaEngine::~LuaEngine() {
    if (initialized) Shutdown();
}

bool LuaEngine::Initialize() {
    // Try to load Lua dynamically
    void* luaLib = dlopen("liblua5.3.so.0", RTLD_LAZY);
    if (!luaLib) {
        luaLib = dlopen("liblua5.1.so.0", RTLD_LAZY);
    }
    
    if (luaLib) {
        std::cout << "[Lua] Found Lua library, but dynamic loading not yet implemented" << std::endl;
        dlclose(luaLib);
    }
    
    std::cout << "[Lua] Stub - would initialize Lua 5.4" << std::endl;
    std::cout << "[Lua] Install lua5.4-dev for real scripting support" << std::endl;
    
    initialized = true; // Mark as initialized even though it's stubbed
    return true;
}

void LuaEngine::Shutdown() {
    std::cout << "[Lua] Stub - shutting down" << std::endl;
    initialized = false;
}

bool LuaEngine::ExecuteString(const std::string& code) {
    if (!initialized) {
        std::cerr << "[Lua] Engine not initialized" << std::endl;
        return false;
    }
    
    std::cout << "[Lua] Would execute: " << code.substr(0, 50) << "..." << std::endl;
    return true;
}

bool LuaEngine::ExecuteFile(const std::string& path) {
    if (!initialized) {
        std::cerr << "[Lua] Engine not initialized" << std::endl;
        return false;
    }
    
    std::cout << "[Lua] Would execute file: " << path << std::endl;
    return true;
}

void LuaEngine::RegisterFunction(const std::string& name, int (*func)(void*)) {
    if (!initialized) return;
    std::cout << "[Lua] Registered function: " << name << std::endl;
}

void LuaEngine::SetGlobal(const std::string& name, int value) {
    if (!initialized) return;
    std::cout << "[Lua] Set global " << name << " = " << value << std::endl;
}

void LuaEngine::SetGlobal(const std::string& name, double value) {
    if (!initialized) return;
    std::cout << "[Lua] Set global " << name << " = " << value << std::endl;
}

void LuaEngine::SetGlobal(const std::string& name, const std::string& value) {
    if (!initialized) return;
    std::cout << "[Lua] Set global " << name << " = \"" << value << "\"" << std::endl;
}

} // namespace vge
