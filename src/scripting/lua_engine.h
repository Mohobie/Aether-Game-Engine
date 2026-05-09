#pragma once

namespace vge {
    class LuaEngine {
    public:
        void Initialize();
        void Execute(const char* script);
        void ExecuteFile(const std::string& path);
        void Shutdown();
    };
}
