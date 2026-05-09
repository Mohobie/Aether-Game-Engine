#include "resource_manager.h"
namespace aether {
ResourceManager::ResourceManager() {}
void ResourceManager::unload(const std::string& path) {
    auto it = resources.find(path);
    if (it != resources.end()) resources.erase(it);
}
void ResourceManager::unloadAll() { resources.clear(); }
size_t ResourceManager::getResourceCount() const { return resources.size(); }
} // namespace aether
