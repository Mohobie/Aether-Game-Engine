#pragma once
#include <cstdint>
#include <type_traits>
namespace vge {
using ComponentID = uint32_t;
using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = 0;
class Component {
public:
    virtual ~Component() = default;
    virtual ComponentID getTypeID() const = 0;
    virtual const char* getTypeName() const = 0;
};
template<typename T>
ComponentID getComponentTypeID() {
    static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
    static ComponentID id = nextComponentTypeID++;
    return id;
}
extern ComponentID nextComponentTypeID;
} // namespace vge
