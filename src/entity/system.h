#pragma once
#include "entity_manager.h"
namespace vge {
class System {
public:
    virtual ~System() = default;
    virtual void initialize() {}
    virtual void update(float deltaTime) {}
    virtual void shutdown() {}
    virtual const char* getName() const = 0;
};
class SystemManager {
public:
    void addSystem(std::unique_ptr<System> system);
    void initializeAll();
    void updateAll(float deltaTime);
    void shutdownAll();
private:
    std::vector<std::unique_ptr<System>> systems;
};
} // namespace vge
