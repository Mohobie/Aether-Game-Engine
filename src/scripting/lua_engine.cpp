#include "scripting/lua_engine.h"
#include "core/logger.h"
#include <iostream>
#include <sstream>
#include <cstring>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace vge {

// ============================================
// Error handler - captures stack traces
// ============================================
static int LuaErrorHandler(lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    luaL_traceback(L, L, msg, 1);
    const char* traceback = lua_tostring(L, -1);
    lua_pushstring(L, traceback);
    return 1;
}

// ============================================
// LuaEngine Implementation
// ============================================

LuaEngine::LuaEngine() : L(nullptr), initialized(false) {}

LuaEngine::~LuaEngine() {
    if (initialized) Shutdown();
}

bool LuaEngine::Initialize() {
    L = luaL_newstate();
    if (!L) {
        lastError = "Failed to create Lua state";
        return false;
    }

    // Open standard libraries (we'll sandbox later)
    luaL_openlibs(L);

    initialized = true;
    return true;
}

void LuaEngine::Shutdown() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
    initialized = false;
}

bool LuaEngine::DoString(const std::string& code) {
    int result = luaL_dostring(L, code.c_str());
    if (result != LUA_OK) {
        lastError = lua_tostring(L, -1);
        lua_pop(L, 1);
        return false;
    }
    return true;
}

bool LuaEngine::DoFile(const std::string& path) {
    int result = luaL_dofile(L, path.c_str());
    if (result != LUA_OK) {
        lastError = lua_tostring(L, -1);
        lua_pop(L, 1);
        return false;
    }
    return true;
}

bool LuaEngine::ExecuteString(const std::string& code) {
    if (!initialized || !L) {
        lastError = "Lua engine not initialized";
        return false;
    }

    // Push error handler
    lua_pushcfunction(L, LuaErrorHandler);
    int errHandler = lua_gettop(L);

    int result = luaL_loadstring(L, code.c_str());
    if (result != LUA_OK) {
        lastError = lua_tostring(L, -1);
        lua_pop(L, 1);
        lua_remove(L, errHandler);
        return false;
    }

    result = lua_pcall(L, 0, LUA_MULTRET, errHandler);
    if (result != LUA_OK) {
        lastError = lua_tostring(L, -1);
        lua_pop(L, 1);
        lua_remove(L, errHandler);
        return false;
    }

    lua_remove(L, errHandler);
    return true;
}

bool LuaEngine::ExecuteFile(const std::string& path) {
    if (!initialized || !L) {
        lastError = "Lua engine not initialized";
        return false;
    }

    // Push error handler
    lua_pushcfunction(L, LuaErrorHandler);
    int errHandler = lua_gettop(L);

    int result = luaL_loadfile(L, path.c_str());
    if (result != LUA_OK) {
        lastError = lua_tostring(L, -1);
        lua_pop(L, 1);
        lua_remove(L, errHandler);
        return false;
    }

    result = lua_pcall(L, 0, LUA_MULTRET, errHandler);
    if (result != LUA_OK) {
        lastError = lua_tostring(L, -1);
        lua_pop(L, 1);
        lua_remove(L, errHandler);
        return false;
    }

    lua_remove(L, errHandler);
    return true;
}

bool LuaEngine::ExecuteFunction(const std::string& name, int nargs, int nresults) {
    return CallFunction(name, nargs, nresults);
}

std::string LuaEngine::GetStackTrace(int level) {
    luaL_traceback(L, L, nullptr, level);
    std::string trace = lua_tostring(L, -1);
    lua_pop(L, 1);
    return trace;
}

// ============================================
// Global Variables
// ============================================

void LuaEngine::SetGlobal(const std::string& name, int value) {
    if (!L) return;
    lua_pushinteger(L, value);
    lua_setglobal(L, name.c_str());
}

void LuaEngine::SetGlobal(const std::string& name, double value) {
    if (!L) return;
    lua_pushnumber(L, value);
    lua_setglobal(L, name.c_str());
}

void LuaEngine::SetGlobal(const std::string& name, float value) {
    if (!L) return;
    lua_pushnumber(L, static_cast<double>(value));
    lua_setglobal(L, name.c_str());
}

void LuaEngine::SetGlobal(const std::string& name, const std::string& value) {
    if (!L) return;
    lua_pushstring(L, value.c_str());
    lua_setglobal(L, name.c_str());
}

void LuaEngine::SetGlobal(const std::string& name, bool value) {
    if (!L) return;
    lua_pushboolean(L, value);
    lua_setglobal(L, name.c_str());
}

void LuaEngine::SetGlobalNil(const std::string& name) {
    if (!L) return;
    lua_pushnil(L);
    lua_setglobal(L, name.c_str());
}

bool LuaEngine::GetGlobal(const std::string& name) {
    if (!L) return false;
    lua_getglobal(L, name.c_str());
    return !lua_isnil(L, -1);
}

int LuaEngine::GetGlobalInt(const std::string& name, int defaultVal) {
    if (!L) return defaultVal;
    lua_getglobal(L, name.c_str());
    int val = lua_isinteger(L, -1) ? static_cast<int>(lua_tointeger(L, -1)) : defaultVal;
    lua_pop(L, 1);
    return val;
}

double LuaEngine::GetGlobalDouble(const std::string& name, double defaultVal) {
    if (!L) return defaultVal;
    lua_getglobal(L, name.c_str());
    double val = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : defaultVal;
    lua_pop(L, 1);
    return val;
}

std::string LuaEngine::GetGlobalString(const std::string& name, const std::string& defaultVal) {
    if (!L) return defaultVal;
    lua_getglobal(L, name.c_str());
    std::string val = lua_isstring(L, -1) ? lua_tostring(L, -1) : defaultVal;
    lua_pop(L, 1);
    return val;
}

bool LuaEngine::GetGlobalBool(const std::string& name, bool defaultVal) {
    if (!L) return defaultVal;
    lua_getglobal(L, name.c_str());
    bool val = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : defaultVal;
    lua_pop(L, 1);
    return val;
}

// ============================================
// Function Registration
// ============================================

void LuaEngine::RegisterFunction(const std::string& name, int (*func)(lua_State*)) {
    if (!L) return;
    lua_pushcfunction(L, func);
    lua_setglobal(L, name.c_str());
}

void LuaEngine::RegisterMethod(const std::string& table, const std::string& name, int (*func)(lua_State*)) {
    if (!L) return;
    lua_getglobal(L, table.c_str());
    if (lua_istable(L, -1)) {
        lua_pushcfunction(L, func);
        lua_setfield(L, -2, name.c_str());
    }
    lua_pop(L, 1);
}

// ============================================
// Tables
// ============================================

void LuaEngine::CreateTable(const std::string& name) {
    if (!L) return;
    lua_newtable(L);
    lua_setglobal(L, name.c_str());
}

void LuaEngine::SetField(const std::string& table, const std::string& key, int value) {
    if (!L) return;
    lua_getglobal(L, table.c_str());
    if (lua_istable(L, -1)) {
        lua_pushinteger(L, value);
        lua_setfield(L, -2, key.c_str());
    }
    lua_pop(L, 1);
}

void LuaEngine::SetField(const std::string& table, const std::string& key, double value) {
    if (!L) return;
    lua_getglobal(L, table.c_str());
    if (lua_istable(L, -1)) {
        lua_pushnumber(L, value);
        lua_setfield(L, -2, key.c_str());
    }
    lua_pop(L, 1);
}

void LuaEngine::SetField(const std::string& table, const std::string& key, const std::string& value) {
    if (!L) return;
    lua_getglobal(L, table.c_str());
    if (lua_istable(L, -1)) {
        lua_pushstring(L, value.c_str());
        lua_setfield(L, -2, key.c_str());
    }
    lua_pop(L, 1);
}

void LuaEngine::SetField(const std::string& table, const std::string& key, bool value) {
    if (!L) return;
    lua_getglobal(L, table.c_str());
    if (lua_istable(L, -1)) {
        lua_pushboolean(L, value);
        lua_setfield(L, -2, key.c_str());
    }
    lua_pop(L, 1);
}

void LuaEngine::PushTable(const std::string& name) {
    if (!L) return;
    lua_getglobal(L, name.c_str());
}

bool LuaEngine::NextTableEntry(int tableIndex) {
    if (!L) return false;
    return lua_next(L, tableIndex) != 0;
}

// ============================================
// Stack Operations
// ============================================

std::string LuaEngine::GetString(int index, const std::string& defaultVal) {
    if (!L) return defaultVal;
    return lua_isstring(L, index) ? lua_tostring(L, index) : defaultVal;
}

int LuaEngine::GetInt(int index, int defaultVal) {
    if (!L) return defaultVal;
    return lua_isinteger(L, index) ? static_cast<int>(lua_tointeger(L, index)) : defaultVal;
}

double LuaEngine::GetDouble(int index, double defaultVal) {
    if (!L) return defaultVal;
    return lua_isnumber(L, index) ? lua_tonumber(L, index) : defaultVal;
}

bool LuaEngine::GetBool(int index, bool defaultVal) {
    if (!L) return defaultVal;
    return lua_isboolean(L, index) ? lua_toboolean(L, index) : defaultVal;
}

LuaType LuaEngine::GetType(int index) {
    if (!L) return LuaType::None;
    return static_cast<LuaType>(lua_type(L, index));
}

void LuaEngine::Pop(int n) {
    if (!L) return;
    lua_pop(L, n);
}

int LuaEngine::GetStackTop() const {
    if (!L) return 0;
    return lua_gettop(L);
}

void LuaEngine::PushNil() {
    if (!L) return;
    lua_pushnil(L);
}

void LuaEngine::PushString(const std::string& str) {
    if (!L) return;
    lua_pushstring(L, str.c_str());
}

void LuaEngine::PushNumber(double num) {
    if (!L) return;
    lua_pushnumber(L, num);
}

void LuaEngine::PushBool(bool b) {
    if (!L) return;
    lua_pushboolean(L, b);
}

void LuaEngine::PushInteger(int n) {
    if (!L) return;
    lua_pushinteger(L, n);
}

// ============================================
// Function Calls
// ============================================

bool LuaEngine::CallFunction(const std::string& name, int nargs, int nresults) {
    if (!L) {
        lastError = "Lua engine not initialized";
        return false;
    }

    // Push error handler
    lua_pushcfunction(L, LuaErrorHandler);
    int errHandler = lua_gettop(L) - nargs - 1;

    // Get the function
    lua_getglobal(L, name.c_str());
    if (!lua_isfunction(L, -1)) {
        lastError = "Function '" + name + "' not found";
        lua_pop(L, 1 + nargs + 1); // pop function, args, error handler
        return false;
    }

    // Move function below args
    if (nargs > 0) {
        lua_insert(L, -(nargs + 1));
    }

    int result = lua_pcall(L, nargs, nresults, errHandler);
    if (result != LUA_OK) {
        lastError = lua_tostring(L, -1);
        lua_pop(L, 1);
        lua_remove(L, errHandler);
        return false;
    }

    lua_remove(L, errHandler);
    return true;
}

bool LuaEngine::FunctionExists(const std::string& name) {
    if (!L) return false;
    lua_getglobal(L, name.c_str());
    bool exists = lua_isfunction(L, -1);
    lua_pop(L, 1);
    return exists;
}

// ============================================
// Sandboxing
// ============================================

void LuaEngine::SetupSandbox(const std::string& allowedPath) {
    if (!L) return;

    // Disable dangerous functions
    DisableDangerousFunctions();

    // Set allowed path
    SetGlobal("GAME_PATH", allowedPath);

    // Override loadfile to restrict paths
    const char* sandboxCode = R"(
        local _loadfile = loadfile
        local _dofile = dofile
        local _io = io
        local _os = os
        local GAME_PATH = GAME_PATH or "."
        
        function loadfile(path)
            if type(path) ~= "string" then return nil, "path must be a string" end
            local fullPath = GAME_PATH .. "/" .. path
            return _loadfile(fullPath)
        end
        
        function dofile(path)
            if type(path) ~= "string" then error("path must be a string") end
            local fullPath = GAME_PATH .. "/" .. path
            return _dofile(fullPath)
        end
        
        -- Restrict io operations
        io = {
            open = function(filename, mode)
                if type(filename) ~= "string" then return nil, "filename must be a string" end
                if string.sub(filename, 1, 1) == "/" or string.sub(filename, 1, 2) == ".." then
                    return nil, "access denied"
                end
                return _io.open(GAME_PATH .. "/" .. filename, mode)
            end,
            lines = function(filename)
                if type(filename) ~= "string" then return nil end
                return _io.lines(GAME_PATH .. "/" .. filename)
            end,
            read = _io.read,
            write = _io.write,
            flush = _io.flush,
            close = _io.close,
            type = _io.type,
            stderr = _io.stderr,
            stdout = _io.stdout,
            stdin = _io.stdin,
        }
        
        -- Restrict os operations
        os = {
            clock = _os.clock,
            date = _os.date,
            difftime = _os.difftime,
            time = _os.time,
        }
    )";

    ExecuteString(sandboxCode);
}

void LuaEngine::DisableDangerousFunctions() {
    if (!L) return;

    // Remove potentially dangerous functions
    const char* dangerous[] = {
        "load", "loadfile", "dofile", "require",
        "package", "debug",
    };

    for (const char* name : dangerous) {
        lua_pushnil(L);
        lua_setglobal(L, name);
    }
}

} // namespace vge
