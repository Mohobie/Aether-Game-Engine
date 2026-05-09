#include "physics_world.h"
namespace aether {
PhysicsWorld::PhysicsWorld() {}
void PhysicsWorld::addCollider(std::shared_ptr<Collider> collider) {
    colliders.push_back(collider);
}
void PhysicsWorld::removeCollider(std::shared_ptr<Collider> collider) {
    auto it = std::find(colliders.begin(), colliders.end(), collider);
    if (it != colliders.end()) colliders.erase(it);
}
bool PhysicsWorld::checkCollision(const Collider& collider) const {
    for (const auto& other : colliders) {
        if (other.get() != &collider && other->intersects(collider)) {
            return true;
        }
    }
    return false;
}
bool PhysicsWorld::raycast(const Ray& ray, RaycastHit& hit) const {
    for (const auto& collider : colliders) {
        RaycastHit temp;
        if (collider->raycast(ray, temp) && temp.hit) {
            hit = temp;
            return true;
        }
    }
    return false;
}
void PhysicsWorld::update(float deltaTime) {}
} // namespace aether
