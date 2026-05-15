# Physics System

## Overview

The Aether Engine physics system provides rigidbody dynamics, collision detection, and spatial queries for voxel game interactions.

## Architecture

### Core Classes

| Class | Purpose |
|-------|---------|
| `PhysicsWorld` | Main simulation container |
| `Rigidbody` | Physics object with mass, velocity, forces |
| `Collider` | Base collision shape |
| `BoxCollider` | Axis-aligned box shape |
| `SphereCollider` | Sphere shape |
| `Contact` | Collision contact point |

### PhysicsWorld

```cpp
PhysicsWorld world(Vec3(0, -9.81f, 0));  // Gravity

// Create body
Rigidbody* body = world.CreateBody(1.0f);  // 1kg mass
body->SetPosition(Vec3(0, 10, 0));
body->useGravity = true;

// Create collider
BoxCollider* collider = world.CreateBoxCollider(Vec3(0,0,0), Vec3(0.5f,0.5f,0.5f));
body->collider = collider;

// Step simulation
world.Step(deltaTime);
```

## Collision Detection

### Broad Phase
- Spatial hash grid for efficient nearby object lookup
- Configurable cell size (default: 10 units)

### Narrow Phase
- Box-Box: SAT (Separating Axis Theorem)
- Box-Sphere: Closest point on box
- Sphere-Sphere: Distance between centers

### Collision Response
- Impulse-based resolution
- Configurable restitution (bounciness)
- Positional correction to prevent sinking
- Multiple solver iterations (default: 4)

## Rigidbody Properties

| Property | Description | Default |
|----------|-------------|---------|
| mass | Kilograms | 1.0 |
| drag | Air resistance | 0.01 |
| useGravity | Affected by gravity | true |
| isKinematic | Not affected by forces | false |
| isSleeping | Optimization flag | false |

## Queries

### Raycast
```cpp
Vec3 hit, normal;
if (world.Raycast(origin, direction, maxDistance, hit, normal)) {
    // Process hit
}
```

### Overlap Queries
```cpp
auto colliders = world.OverlapSphere(center, radius);
auto colliders = world.OverlapBox(center, halfExtents);
```

## Integration with Game Loop

```cpp
// In game update:
physicsWorld.Step(deltaTime);

// Sync visual transforms:
for (auto* body : bodies) {
    entity.transform.position = body->GetPosition();
}
```

## Performance

| Feature | Cost |
|---------|------|
| Broad phase | O(n) with spatial grid |
| Narrow phase | O(n*m) where m = nearby objects |
| Solver | Iterations * contacts |

**Tips:**
- Mark static objects with `isStatic = true`
- Put sleeping objects to rest
- Use appropriate cell size for spatial grid

## Future Work

- [ ] Capsule collider
- [ ] Mesh collider (BVH-based)
- [ ] Continuous collision detection (CCD)
- [ ] Joints and constraints
- [ ] Ragdoll physics
- [ ] Cloth simulation
- [ ] Buoyancy
- [ ] Vehicle physics

## Status

✅ **Complete** - Core physics with rigidbodies, collision detection, and spatial queries

Last Updated: 2026-05-14
