# Physics Module API
**Files:** src/physics/aabb.h, src/physics/collider.h, src/physics/collision.h, src/physics/physics_world.h, src/physics/ray.h, src/physics/rigidbody.h

## `physics/aabb.h`
```cpp
namespace aether {
```

### `struct AABB`
| Member | Type |
|--------|------|
| `min` | `Vec3` |
| `max` | `Vec3` |

## `physics/collider.h`
```cpp
namespace aether {
```

### `class Collider`

## `physics/collision.h`
```cpp
namespace vge {
```

### `struct AABB`

### `class Physics`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `RayCast` | `static bool` | `const Vec3& origin, const Vec3& direction, float maxDistance, Vec3& hitPoint` |
| `RayCastAABB` | `static bool` | `const Vec3& origin, const Vec3& direction, const AABB& box, float maxDist, Vec3& hit, float& outDist` |

## `physics/physics_world.h`
```cpp
namespace aether {
```

### `class PhysicsWorld`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `addCollider` | `void` | `std::shared_ptr<Collider> collider` |
| `removeCollider` | `void` | `std::shared_ptr<Collider> collider` |
| `checkCollision` | `bool` | `const Collider& collider` |
| `raycast` | `bool` | `const Ray& ray, RaycastHit& hit` |
| `update` | `void` | `float deltaTime` |

## `physics/ray.h`
```cpp
namespace aether {
```

### `struct Ray`
| Member | Type |
|--------|------|
| `origin` | `Vec3` |
| `direction` | `Vec3` |

### `struct RaycastHit`
| Member | Type |
|--------|------|
| `point` | `Vec3` |
| `normal` | `Vec3` |
| `distance` | `float` |
| `hit` | `bool` |

## `physics/rigidbody.h`
```cpp
namespace vge {
```

### `class RigidBody`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `SetPosition` | `void` | `const Vec3& pos` |
| `SetVelocity` | `void` | `const Vec3& vel` |
| `ApplyForce` | `void` | `const Vec3& force` |
| `Update` | `void` | `float deltaTime` |
| `GetBounds` | `AABB` | `` |
