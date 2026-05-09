#pragma once
#include <string>
namespace aether {
class Logger {
public:
    static void info(const std::string& msg);
    static void error(const std::string& msg);
};
} // namespace aether
