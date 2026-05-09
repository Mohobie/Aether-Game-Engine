#pragma once
#include <string>

namespace vge {
    class Logger {
    public:
        static void Info(const std::string& msg);
        static void Error(const std::string& msg);
        static void Debug(const std::string& msg);
    };
}
