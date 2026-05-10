# Physics Module API

**Files:** `physics/aabb.h`, `physics/rigidbody.h`, `physics/collider.h`, `physics/physics_world.h`, `physics/raycast.h`, `physics/joint.h`

---

## `physics/aabb.h`

```cpp
namespace vge {
```

### `struct AABB`
Axis-aligned bounding box.

| Member | Type | Description |
|--------|------|-------------|
| `min` | `Vec3` | Minimum corner |
| `max` | `Vec3` | Maximum corner |

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `AABB` | (ctor) | `void` | Default (empty) |
| `AABB` | (ctor) | `const Vec3& min, const Vec3& max` | From min/max |
| `IsValid` | `bool` | `void` | Check valid |
| `GetCenter` | `Vec3` | `void` | Get center |
| `GetExtents` | `Vec3` | `void` | Get half-extents |
| `GetSize` | `Vec3` | `void` | Get full size |
| `Contains` | `bool` | `const Vec3& point` | Contains point |
| `Contains` | `bool` | `const AABB& other` | Contains other AABB |
| `Intersects` | `bool` | `const AABB& other` | Intersects other |
| `Expand` | `void` | `const Vec3& point` | Expand to include point |
| `Expand` | `void` | `const AABB& other` | Expand to include AABB |
| `Translate` | `void` | `const Vec3& offset` | Move AABB |
| `Scale` | `void` | `const Vec3& scale` | Scale AABB |
| `GetVolume` | `float` | `void` | Calculate volume |
| `GetSurfaceArea` | `float` | `void` | Calculate surface area |
| `operator==` | `bool` | `const AABB& other` | Equality |
| `operator!=` | `bool` | `const AABB& other` | Inequality |

---

## `physics/rigidbody.h`

### `enum class BodyType`
| Value | Description |
|-------|-------------|
| `Static` | Immovable |
| `Dynamic` | Affected by forces |
| `Kinematic` | Moved by script, not forces |

### `struct RigidBodyProperties`
| Member | Type | Description |
|--------|------|-------------|
| `mass` | `float` | Mass (kg) |
| `drag` | `float` | Linear drag |
| `angularDrag` | `float` | Angular drag |
| `useGravity` | `bool` | Affected by gravity |
| `isKinematic` | `bool` | Is kinematic |
| `freezePosition` | `Vec3` | Freeze axes (0=free, 1=frozen) |
| `freezeRotation` | `Vec3` | Freeze rotation axes |
| `maxAngularVelocity` | `float` | Max angular speed |
| `maxDepenetrationVelocity` | `float` | Max depenetration |
| `sleepThreshold` | `float` | Sleep velocity threshold |

### `class RigidBody`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `RigidBody` | (ctor) | `void` | Constructor |
| `SetProperties` | `void` | `const RigidBodyProperties& props` | Set properties |
| `GetProperties` | `RigidBodyProperties` | `void` | Get properties |
| `SetPosition` | `void` | `const Vec3& pos` | Set position |
| `GetPosition` | `Vec3` | `void` | Get position |
| `SetRotation` | `void` | `const Vec3& rot` | Set rotation (Euler) |
| `GetRotation` | `Vec3` | `void` | Get rotation |
| `SetVelocity` | `void` | `const Vec3& vel` | Set linear velocity |
| `GetVelocity` | `Vec3` | `void` | Get velocity |
| `SetAngularVelocity` | `void` | `const Vec3& vel` | Set angular velocity |
| `GetAngularVelocity` | `Vec3` | `void` | Get angular velocity |
| `AddForce` | `void` | `const Vec3& force, ForceMode mode = ForceMode::Force` | Add force |
| `AddTorque` | `void` | `const Vec3& torque` | Add torque |
| `AddExplosionForce` | `void` | `float force, const Vec3& position, float radius` | Explosion force |
| `AddImpulse` | `void` | `const Vec3& impulse` | Add impulse |
| `GetMass` | `float` | `void` | Get mass |
| `SetMass` | `void` | `float mass` | Set mass |
| `UseGravity` | `void` | `bool use` | Toggle gravity |
| `IsUsingGravity` | `bool` | `void` | Check gravity |
| `SetKinematic` | `void` | `bool kinematic` | Set kinematic |
| `IsKinematic` | `bool` | `void` | Check kinematic |
| `Sleep` | `void` | `void` | Put to sleep |
| `WakeUp` | `void` | `void` | Wake up |
| `IsSleeping` | `bool` | `void` | Check sleeping |
| `GetAABB` | `AABB` | `void` | Get bounding box |
| `SetAABB` | `void` | `const AABB& aabb` | Set bounding box |

---

## `physics/collider.h`

### `enum class ColliderType`
| Value | Description |
|-------|-------------|
| `Box` | Box shape |
| `Sphere` | Sphere shape |
| `Capsule` | Capsule shape |
| `Mesh` | Triangle mesh |
| `Terrain` | Heightfield |

### `struct ColliderProperties`
| Member | Type | Description |
|--------|------|-------------|
| `type` | `ColliderType` | Collider type |
| `center` | `Vec3` | Offset from body |
| `size` | `Vec3` | Box size |
| `radius` | `float` | Sphere/capsule radius |
| `height` | `float` | Capsule height |
| `isTrigger` | `bool` | Is trigger volume |
| `contactOffset` | `float` | Contact offset |
| `layer` | `uint32_t` | Physics layer |

### `class Collider`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Collider` | (ctor) | `void` | Constructor |
| `SetProperties` | `void` | `const ColliderProperties& props` | Set properties |
| `GetProperties` | `ColliderProperties` | `void` | Get properties |
| `SetType` | `void` | `ColliderType type` | Set type |
| `GetType` | `ColliderType` | `void` | Get type |
| `SetCenter` | `void` | `const Vec3& center` | Set center offset |
| `GetCenter` | `Vec3` | `void` | Get center |
| `SetSize` | `void` | `const Vec3& size` | Set box size |
| `GetSize` | `Vec3` | `void` | Get size |
| `SetRadius` | `void` | `float radius` | Set radius |
| `GetRadius` | `float` | `void` | Get radius |
| `SetHeight` | `void` | `float height` | Set height |
| `GetHeight` | `float` | `void` | Get height |
| `SetTrigger` | `void` | `bool trigger` | Set trigger |
| `IsTrigger` | `bool` | `void` | Check trigger |
| `SetLayer` | `void` | `uint32_t layer` | Set layer |
| `GetLayer` | `uint32_t` | `void` | Get layer |
| `GetAABB` | `AABB` | `void` | Get AABB |
| `SetMesh` | `void` | `Mesh* mesh` | Set mesh collider |
| `GetMesh` | `Mesh*` | `void` | Get mesh |

---

## `physics/physics_world.h`

### `struct Collision`
| Member | Type | Description |
|--------|------|-------------|
| `bodyA` | `RigidBody*` | First body |
| `bodyB` | `RigidBody*` | Second body |
| `point` | `Vec3` | Contact point |
| `normal` | `Vec3` | Contact normal |
| `penetration` | `float` | Penetration depth |
| `relativeVelocity` | `Vec3` | Relative velocity |

### `class PhysicsWorld`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `PhysicsWorld&` | `void` | Singleton |
| `Initialize` | `void` | `void` | Initialize |
| `Shutdown` | `void` | `void` | Shutdown |
| `Update` | `void` | `float deltaTime` | Step simulation |
| `SetGravity` | `void` | `const Vec3& gravity` | Set gravity |
| `GetGravity` | `Vec3` | `void` | Get gravity |
| `AddBody` | `void` | `RigidBody* body` | Add rigid body |
| `RemoveBody` | `void` | `RigidBody* body` | Remove body |
| `AddCollider` | `void` | `Collider* collider` | Add collider |
| `RemoveCollider` | `void` | `Collider* collider` | Remove collider |
| `Raycast` | `bool` | `const Vec3& origin, const Vec3& dir, float maxDist, RaycastHit& hit` | Raycast |
| `SphereCast` | `bool` | `const Vec3& origin, float radius, const Vec3& dir, float maxDist, RaycastHit& hit` | Sphere cast |
| `BoxCast` | `bool` | `const Vec3& origin, const Vec3& halfExtents, const Vec3& dir, float maxDist, RaycastHit& hit` | Box cast |
| `OverlapSphere` | `std::vector<RigidBody*>` | `const Vec3& center, float radius` | Overlap sphere |
| `OverlapBox` | `std::vector<RigidBody*>` | `const Vec3& center, const Vec3& halfExtents` | Overlap box |
| `SetSimulationSpeed` | `void` | `float speed` | Set time scale |
| `GetSimulationSpeed` | `float` | `void` | Get time scale |
| `SetMaxIterations` | `void` | `int iterations` | Set solver iterations |
| `GetMaxIterations` | `int` | `void` | Get iterations |
| `SetSleepThreshold` | `void` | `float threshold` | Set sleep threshold |
| `GetBodyCount` | `size_t` | `void` | Count bodies |
| `GetColliderCount` | `size_t` | `void` | Count colliders |
| `Clear` | `void` | `void` | Remove all |

---

## `physics/raycast.h`

### `struct RaycastHit`
| Member | Type | Description |
|--------|------|-------------|
| `hit` | `bool` | Did hit |
| `point` | `Vec3` | Hit point |
| `normal` | `Vec3` | Hit normal |
| `distance` | `float` | Hit distance |
| `body` | `RigidBody*` | Hit body |
| `collider` | `Collider*` | Hit collider |

---

## `physics/joint.h`

### `enum class JointType`
| Value | Description |
|-------|-------------|
| `Fixed` | Rigid connection |
| `Hinge` | Rotates around axis |
| `Spring` | Spring connection |
| `Distance` | Maintains distance |
| `Slider` | Slides along axis |

### `struct JointProperties`
| Member | Type | Description |
|--------|------|-------------|
| `type` | `JointType` | Joint type |
| `bodyA` | `RigidBody*` | First body |
| `bodyB` | `RigidBody*` | Second body |
| `anchorA` | `Vec3` | Local anchor on A |
| `anchorB` | `Vec3` | Local anchor on B |
| `axis` | `Vec3` | Joint axis |
| `breakForce` | `float` | Break threshold |
| `enableCollision` | `bool` | Bodies collide |

### `class Joint`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Joint` | (ctor) | `void` | Constructor |
| `SetProperties` | `void` | `const JointProperties& props` | Set properties |
| `GetProperties` | `JointProperties` | `void` | Get properties |
| `SetType` | `void` | `JointType type` | Set type |
| `GetType` | `JointType` | `void` | Get type |
| `SetBodies` | `void` | `RigidBody* a, RigidBody* b` | Set bodies |
| `GetBodyA` | `RigidBody*` | `void` | Get body A |
| `GetBodyB` | `RigidBody*` | `void` | Get body B |
| `SetAnchor` | `void` | `const Vec3& anchor` | Set anchor |
| `GetAnchor` | `Vec3` | `void` | Get anchor |
| `SetAxis` | `void` | `const Vec3& axis` | Set axis |
| `GetAxis` | `Vec3` | `void` | Get axis |
| `SetBreakForce` | `void` | `float force` | Set break force |
| `GetBreakForce` | `float` | `void` | Get break force |
| `IsBroken` | `bool` | `void` | Check broken |
| `Break` | `void` | `void` | Force break |
| `EnableCollision` | `void` | `bool enable` | Toggle collision |
| `IsCollisionEnabled` | `bool` | `void` | Check collision |
