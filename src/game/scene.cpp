#include "scene.h"

namespace vge {

Scene::Scene(const std::string& name) : name(name) {}

void Scene::initialize() {
    world.Initialize();
    world.GenerateTerrain(0, 0);
}

void Scene::update(float deltaTime) {
    entityManager.Update(deltaTime);
}

void Scene::render() {}

EntityManager& Scene::getEntityManager() { return entityManager; }
World& Scene::getWorld() { return world; }
const std::string& Scene::getName() const { return name; }
void Scene::setActiveCamera(EntityID cameraEntity) { activeCamera = cameraEntity; }
EntityID Scene::getActiveCamera() const { return activeCamera; }
} // namespace vge
