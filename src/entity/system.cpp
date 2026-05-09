#include "system.h"
namespace aether {
void SystemManager::addSystem(std::unique_ptr<System> system) {
    systems.push_back(std::move(system));
}
void SystemManager::initializeAll() {
    for (auto& system : systems) system->initialize();
}
void SystemManager::updateAll(float deltaTime) {
    for (auto& system : systems) system->update(deltaTime);
}
void SystemManager::shutdownAll() {
    for (auto& system : systems) system->shutdown();
}
} // namespace aether
