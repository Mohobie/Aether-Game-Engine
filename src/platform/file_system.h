#pragma once
#include <string>
#include <vector>
namespace aether {
class FileSystem {
public:
    static bool exists(const std::string& path);
    static std::string readText(const std::string& path);
    static bool writeText(const std::string& path, const std::string& text);
};
} // namespace aether
