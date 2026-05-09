#include "file_system.h"
#include <fstream>
#include <sstream>
namespace aether {
bool FileSystem::exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}
std::string FileSystem::readText(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
bool FileSystem::writeText(const std::string& path, const std::string& text) {
    std::ofstream file(path);
    if (!file) return false;
    file << text;
    return file.good();
}
} // namespace aether
