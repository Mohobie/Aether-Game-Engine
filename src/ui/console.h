#pragma once
#include <string>
#include <vector>

namespace VoxelEngine {
    class Console {
    public:
        void Log(const std::string& msg);
        void ExecuteCommand(const std::string& cmd);
        std::vector<std::string> GetHistory();
        void Clear();
    private:
        std::vector<std::string> history;
    };
}
