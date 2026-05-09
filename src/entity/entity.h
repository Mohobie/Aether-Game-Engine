#pragma once
#include "component.h"
#include <vector>
#include <memory>
namespace aether {
class Entity {
public:
    explicit Entity(EntityID id);
    EntityID getID() const { return id; }
    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = comp.get();
        components.push_back(std::move(comp));
        return ptr;
    }
    template<typename T>
    T* getComponent() const {
        for (const auto& comp : components) {
            if (comp->getTypeID() == getComponentTypeID<T>()) {
                return static_cast<T*>(comp.get());
            }
        }
        return nullptr;
    }
    bool hasComponent(ComponentID typeID) const;
    void removeComponent(ComponentID typeID);
private:
    EntityID id;
    std::vector<std::unique_ptr<Component>> components;
};
} // namespace aether
