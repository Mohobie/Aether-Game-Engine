#pragma once

namespace VoxelEngine {
    class LuaEngine {
    public:
        void Initialize();
        void Execute(const char* script);
        void ExecuteFile(const std::string& path);
        void Shutdown();
    };
}
