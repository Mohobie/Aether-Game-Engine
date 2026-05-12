#pragma once
#include <string>
#include <functional>
#include <vector>

// Forward declare lua_State to avoid including lua.h in headers
struct lua_State;

namespace vge {

// Lua value types
enum class LuaType {
    None = -1,
    Nil = 0,
    Boolean = 1,
    LightUserData = 2,
    Number = 3,
    String = 4,
    Table = 5,
    Function = 6,
    UserData = 7,
    Thread = 8
};

// Safe wrapper around lua_State
class LuaEngine {
private:
    lua_State* L;
    bool initialized;
    std::string lastError;

    // Internal helpers
    bool DoString(const std::string& code);
    bool DoFile(const std::string& path);
    std::string GetStackTrace(int level = 0);

public:
    LuaEngine();
    ~LuaEngine();

    // Lifecycle
    bool Initialize();
    void Shutdown();
    bool IsInitialized() const { return initialized; }

    // Execution
    bool ExecuteString(const std::string& code);
    bool ExecuteFile(const std::string& path);
    bool ExecuteFunction(const std::string& name, int nargs = 0, int nresults = 0);

    // Error handling
    std::string GetLastError() const { return lastError; }
    void ClearError() { lastError.clear(); }

    // Global variables
    void SetGlobal(const std::string& name, int value);
    void SetGlobal(const std::string& name, double value);
    void SetGlobal(const std::string& name, float value);
    void SetGlobal(const std::string& name, const std::string& value);
    void SetGlobal(const std::string& name, bool value);
    void SetGlobalNil(const std::string& name);

    // Get global
    bool GetGlobal(const std::string& name);
    int GetGlobalInt(const std::string& name, int defaultVal = 0);
    double GetGlobalDouble(const std::string& name, double defaultVal = 0.0);
    std::string GetGlobalString(const std::string& name, const std::string& defaultVal = "");
    bool GetGlobalBool(const std::string& name, bool defaultVal = false);

    // Function registration (C++ -> Lua)
    void RegisterFunction(const std::string& name, int (*func)(lua_State*));
    void RegisterMethod(const std::string& table, const std::string& name, int (*func)(lua_State*));

    // Table creation
    void CreateTable(const std::string& name);
    void SetField(const std::string& table, const std::string& key, int value);
    void SetField(const std::string& table, const std::string& key, double value);
    void SetField(const std::string& table, const std::string& key, const std::string& value);
    void SetField(const std::string& table, const std::string& key, bool value);

    // Table iteration helper
    void PushTable(const std::string& name);
    bool NextTableEntry(int tableIndex);
    std::string GetString(int index, const std::string& defaultVal = "");
    int GetInt(int index, int defaultVal = 0);
    double GetDouble(int index, double defaultVal = 0.0);
    bool GetBool(int index, bool defaultVal = false);
    LuaType GetType(int index);
    void Pop(int n = 1);

    // Sandboxing
    void SetupSandbox(const std::string& allowedPath);
    void DisableDangerousFunctions();

    // Stack management
    int GetStackTop() const;
    void PushNil();
    void PushString(const std::string& str);
    void PushNumber(double num);
    void PushBool(bool b);
    void PushInteger(int n);

    // Call a function by name with pushed args
    bool CallFunction(const std::string& name, int nargs, int nresults);

    // Check if function exists
    bool FunctionExists(const std::string& name);

    // Raw lua_State access (for advanced bindings)
    lua_State* GetState() { return L; }
};

} // namespace vge
