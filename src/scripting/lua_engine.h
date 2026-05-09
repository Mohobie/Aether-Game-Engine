#pragma once
#include <string>

namespace vge {

class LuaEngine {
private:
    void* L;  // lua_State* (opaque pointer)
    bool initialized;
    
public:
    LuaEngine();
    ~LuaEngine();
    
    bool Initialize();
    void Shutdown();
    
    bool ExecuteString(const std::string& code);
    bool ExecuteFile(const std::string& path);
    void RegisterFunction(const std::string& name, int (*func)(void*));
    void SetGlobal(const std::string& name, int value);
    void SetGlobal(const std::string& name, double value);
    void SetGlobal(const std::string& name, const std::string& value);
    
    bool IsInitialized() const { return initialized; }
};

} // namespace vge