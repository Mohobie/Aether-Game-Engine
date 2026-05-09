#pragma once
#include "core/math.h"
#include "entity/entity_manager.h"
#include "voxel/world.h"
#include <string>
namespace aether {
class Scene {
public:
    Scene(const std::string& name);
    void initialize();
    void update(float deltaTime);
    void render();
    EntityManager& getEntityManager();
    World& getWorld();
    const std::string& getName() const;
    void setActiveCamera(EntityID cameraEntity);
    EntityID getActiveCamera() const;
private:
    std::string name;
    EntityManager entityManager;
    World world;
    EntityID activeCamera = INVALID_ENTITY;
};
} // namespace aether
