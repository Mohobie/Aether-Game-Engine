#pragma once
#include <string>
#include <vector>
#include "platform_common.h"

namespace vge {
class FileSystem {
public:
    static bool exists(const std::string& path);
    static std::string readText(const std::string& path);
    static bool writeText(const std::string& path, const std::string& text);
    static bool createDirectory(const std::string& path);
    static bool deleteFile(const std::string& path);
    static std::vector<std::string> listDirectory(const std::string& path);
};
} // namespace vge
