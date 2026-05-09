#pragma once
#include <string>
#include <functional>

namespace vge {

class LuaEngine;

class ScriptEngine {
private:
    LuaEngine* luaEngine;
    bool initialized;
    
public:
    ScriptEngine();
    ~ScriptEngine();
    
    bool Initialize();
    void Shutdown();
    
    bool LoadScript(const std::string& path);
    bool ExecuteCode(const std::string& code);
    void RegisterFunction(const std::string& name, std::function<void()> func);
    void RegisterDefaultBindings();
    void Update(float deltaTime);
};

} // namespace vge
