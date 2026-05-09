#pragma once
#include <string>

namespace VoxelEngine {
    class Logger {
    public:
        static void Info(const std::string& msg);
        static void Error(const std::string& msg);
        static void Debug(const std::string& msg);
    };
}
