#include "entity.h"
#include <algorithm>
namespace aether {
Entity::Entity(EntityID id) : id(id) {}
bool Entity::hasComponent(ComponentID typeID) const {
    for (const auto& comp : components) {
        if (comp->getTypeID() == typeID) return true;
    }
    return false;
}
void Entity::removeComponent(ComponentID typeID) {
    auto it = std::remove_if(components.begin(), components.end(),
        [typeID](const auto& comp) { return comp->getTypeID() == typeID; });
    components.erase(it, components.end());
}
} // namespace aether
