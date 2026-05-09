#pragma once
#include <string>
#include <functional>

namespace VoxelEngine {
    class ScriptEngine {
    public:
        void Initialize();
        void ExecuteFile(const std::string& path);
        void RegisterFunction(const std::string& name, std::function<void()> func);
        void Update(float deltaTime);
        void Shutdown();
    };
}
