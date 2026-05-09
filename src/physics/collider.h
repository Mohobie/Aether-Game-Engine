#pragma once
#include "aabb.h"
#include "ray.h"
namespace aether {
class Collider {
public:
    virtual ~Collider() = default;
    virtual AABB getBounds() const = 0;
    virtual bool intersects(const Collider& other) const = 0;
    virtual bool raycast(const Ray& ray, RaycastHit& hit) const = 0;
};
class BoxCollider : public Collider {
public:
    BoxCollider(const AABB& bounds) : bounds(bounds) {}
    AABB getBounds() const override { return bounds; }
    bool intersects(const Collider& other) const override;
    bool raycast(const Ray& ray, RaycastHit& hit) const override;
private:
    AABB bounds;
};
} // namespace aether
