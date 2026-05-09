#include "collider.h"
namespace aether {
bool BoxCollider::intersects(const Collider& other) const {
    return bounds.intersects(other.getBounds());
}
bool BoxCollider::raycast(const Ray& ray, RaycastHit& hit) const {
    // Simplified ray-AABB intersection
    hit.hit = false;
    return false;
}
} // namespace aether
