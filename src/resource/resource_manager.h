#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <typeindex>
namespace aether {
class Resource {
public:
    virtual ~Resource() = default;
    virtual const std::string& getPath() const = 0;
    virtual bool isLoaded() const = 0;
};
class ResourceManager {
public:
    ResourceManager();
    template<typename T>
    std::shared_ptr<T> load(const std::string& path) {
        auto key = std::type_index(typeid(T)).name() + std::string(":") + path;
        auto it = resources.find(key);
        if (it != resources.end()) return std::static_pointer_cast<T>(it->second);
        auto resource = std::make_shared<T>();
        if (resource->load(path)) {
            resources[key] = resource;
            return resource;
        }
        return nullptr;
    }
    void unload(const std::string& path);
    void unloadAll();
    size_t getResourceCount() const;
private:
    std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
};
} // namespace aether
