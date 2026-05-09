#include "scripting/lua_engine.h"
#include <iostream>

// Stub implementation when Lua is not available
namespace vge {

LuaEngine::LuaEngine() : L(nullptr), initialized(false) {}

LuaEngine::~LuaEngine() {
    if (initialized) Shutdown();
}

bool LuaEngine::Initialize() {
    std::cout << "[Lua] Stub - would initialize Lua 5.4" << std::endl;
    initialized = true;
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
