#include "scene.h"
#include <iostream>
namespace aether {
Scene::Scene(const std::string& name) : name(name), world(12345) {}
void Scene::initialize() {
    std::cout << "[Scene] Initializing: " << name << std::endl;
    world.initialize();
}
void Scene::update(float deltaTime) {}
void Scene::render() {}
EntityManager& Scene::getEntityManager() { return entityManager; }
World& Scene::getWorld() { return world; }
const std::string& Scene::getName() const { return name; }
void Scene::setActiveCamera(EntityID cameraEntity) { activeCamera = cameraEntity; }
EntityID Scene::getActiveCamera() const { return activeCamera; }
} // namespace aether
