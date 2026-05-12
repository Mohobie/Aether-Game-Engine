#pragma once
#include "collider.h"
#include <vector>
#include <memory>
namespace vge {
class PhysicsWorld {
public:
    PhysicsWorld();
    void addCollider(std::shared_ptr<Collider> collider);
    void removeCollider(std::shared_ptr<Collider> collider);
    bool checkCollision(const Collider& collider) const;
    bool raycast(const Ray& ray, RaycastHit& hit) const;
    void update(float deltaTime);
private:
    std::vector<std::shared_ptr<Collider>> colliders;
};
} // namespace vge
