#pragma once
#include <string>
#include <vector>
#include <functional>
namespace vge {
class AssetLoader {
public:
    AssetLoader();
    bool loadText(const std::string& path, std::string& out);
    bool loadBinary(const std::string& path, std::vector<uint8_t>& out);
    bool fileExists(const std::string& path) const;
    std::string getExtension(const std::string& path) const;
    void setSearchPath(const std::string& path);
private:
    std::string searchPath;
};
} // namespace vge
