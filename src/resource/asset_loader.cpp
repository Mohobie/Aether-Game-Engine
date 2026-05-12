#include "asset_loader.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
namespace vge {
AssetLoader::AssetLoader() {}
bool AssetLoader::loadText(const std::string& path, std::string& out) {
    std::ifstream file(searchPath + path);
    if (!file) return false;
    std::stringstream ss;
    ss << file.rdbuf();
    out = ss.str();
    return true;
}
bool AssetLoader::loadBinary(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream file(searchPath + path, std::ios::binary);
    if (!file) return false;
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    out.resize(size);
    file.read(reinterpret_cast<char*>(out.data()), size);
    return file.good();
}
bool AssetLoader::fileExists(const std::string& path) const {
    struct stat st;
    return stat((searchPath + path).c_str(), &st) == 0;
}
std::string AssetLoader::getExtension(const std::string& path) const {
    size_t pos = path.find_last_of('.');
    return pos != std::string::npos ? path.substr(pos + 1) : "";
}
void AssetLoader::setSearchPath(const std::string& path) { searchPath = path; }
} // namespace vge
