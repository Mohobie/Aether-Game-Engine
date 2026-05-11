# Physics System

The engine includes a physics system for rigidbody simulation, collision detection, and raycasting.

## RigidBody

```cpp
#include "physics/rigidbody.h"

// Create rigidbody
vge::RigidBody body;
body.SetMass(1.0f);
body.SetPosition(vge::Vec3(0, 10, 0));
body.SetGravity(vge::Vec3(0, -9.81f, 0));

// Shapes
body.SetShape(BoxShape(vge::Vec3(1, 1, 1)));      // Box
body.SetShape(SphereShape(1.0f));                  // Sphere
body.SetShape(CapsuleShape(0.5f, 2.0f));           // Capsule (radius, height)
body.SetShape(CylinderShape(0.5f, 2.0f));          // Cylinder

// Properties
body.SetRestitution(0.5f);     // Bounciness (0-1)
body.SetFriction(0.3f);        // Surface friction
body.SetLinearDamping(0.1f);   // Air resistance
body.SetAngularDamping(0.1f);

// Lock axes
body.LockRotation(true);       // Prevent tipping
body.LockPositionY(false);     // Allow vertical movement

// Update
void Update(float dt) {
    body.Update(dt);
    
    // Get results
    vge::Vec3 pos = body.GetPosition();
    vge::Vec3 vel = body.GetVelocity();
    vge::Vec3 rot = body.GetRotation();
}
```

## Physics World

```cpp
#include "physics/physics_world.h"

// Create physics world
vge::PhysicsWorld physics;
physics.Initialize();
physics.SetGravity(vge::Vec3(0, -9.81f, 0));

// Add bodies
physics.AddBody(&playerBody);
physics.AddBody(&enemyBody);

// Update physics
void Update(float dt) {
    // Fixed timestep for stable physics
    physics.Update(dt);
}

// Raycast
vge::RaycastHit hit;
if (physics.Raycast(ray, maxDistance, hit)) {
    // hit.body - rigidbody hit
    // hit.point - world position
    // hit.normal - surface normal
    // hit.distance - distance from ray origin
}

// Sphere cast (swept sphere)
std::vector<vge::RaycastHit> hits;
physics.SphereCast(sphereCenter, radius, direction, maxDist, hits);

// Overlap tests
std::vector<RigidBody*> overlaps;
physics.OverlapSphere(center, radius, overlaps);
physics.OverlapBox(center, extents, overlaps);
physics.OverlapCapsule(center, radius, height, overlaps);

// Remove body
physics.RemoveBody(&body);
```

## Collision Detection

```cpp
#include "physics/collision.h"

// Collision layers
enum CollisionLayer {
    Layer_Default = 1 << 0,
    Layer_Player = 1 << 1,
    Layer_Enemy = 1 << 2,
    Layer_Ground = 1 << 3,
    Layer_Projectile = 1 << 4,
};

// Set collision filters
playerBody.SetLayer(Layer_Player);
playerBody.SetCollisionMask(Layer_Default | Layer_Ground | Layer_Enemy);

// Collision callbacks
body.SetOnCollisionEnter([](RigidBody* other, const CollisionInfo& info) {
    // Called when collision starts
    if (other->GetLayer() == Layer_Enemy) {
        TakeDamage(10);
    }
});

body.SetOnCollisionStay([](RigidBody* other, const CollisionInfo& info) {
    // Called every frame while colliding
});

body.SetOnCollisionExit([](RigidBody* other) {
    // Called when collision ends
});

// Trigger volumes (no physical response)
body.SetIsTrigger(true);
body.SetOnTriggerEnter([](RigidBody* other) {
    // Called when something enters trigger
    if (other->GetLayer() == Layer_Player) {
        ActivateCheckpoint();
    }
});
```

## Character Controller

```cpp
#include "core/player_controller.h"

// Create character controller
vge::PlayerController controller;
controller.Initialize(&physics);

// Settings
controller.SetHeight(2.0f);
controller.SetRadius(0.5f);
controller.SetStepHeight(0.3f);
controller.SetSlopeLimit(45.0f);
controller.SetMoveSpeed(5.0f);
controller.SetJumpForce(10.0f);

// Update
void Update(float dt) {
    // Movement input
    vge::Vec3 moveInput(0, 0, 0);
    if (input.GetKey(Key::W)) moveInput.z += 1;
    if (input.GetKey(Key::S)) moveInput.z -= 1;
    if (input.GetKey(Key::A)) moveInput.x -= 1;
    if (input.GetKey(Key::D)) moveInput.x += 1;
    
    // Normalize to prevent faster diagonal movement
    if (moveInput.Length() > 0) {
        moveInput = moveInput.Normalized();
    }
    
    // Apply movement (relative to camera)
    vge::Vec3 forward = camera.GetForward();
    vge::Vec3 right = camera.GetRight();
    vge::Vec3 worldMove = forward * moveInput.z + right * moveInput.x;
    controller.Move(worldMove, dt);
    
    // Jump
    if (input.GetKeyDown(Key::Space) && controller.IsGrounded()) {
        controller.Jump();
    }
    
    // Update controller
    controller.Update(dt);
    
    // Sync camera to controller
    camera.SetPosition(controller.GetPosition() + vge::Vec3(0, 1.6f, 0));
}
```

## Constraints

```cpp
#include "physics/physics_constraints.h"

// Fixed joint (locks two bodies together)
vge::FixedJoint fixed;
fixed.Initialize(&bodyA, &bodyB);
fixed.SetBreakForce(1000.0f);
physics.AddConstraint(&fixed);

// Hinge joint (door, lever)
vge::HingeJoint hinge;
hinge.Initialize(&door, &frame);
hinge.SetAnchor(vge::Vec3(0, 1, 0));
hinge.SetAxis(vge::Vec3(0, 1, 0));  // Rotate around Y
hinge.SetLimits(-90.0f, 90.0f);      // Open angle
physics.AddConstraint(&hinge);

// Spring joint
vge::SpringJoint spring;
spring.Initialize(&bodyA, &bodyB);
spring.SetAnchorA(vge::Vec3(0, 0, 0));
spring.SetAnchorB(vge::Vec3(0, 2, 0));
spring.SetSpringConstant(100.0f);
spring.SetDamping(5.0f);
physics.AddConstraint(&spring);

// Remove constraint
physics.RemoveConstraint(&spring);
```

## Voxel Collision

```cpp
// Check collision with voxel world
vge::Vec3 playerPos = controller.GetPosition();
float playerRadius = 0.5f;

// Get chunks around player
for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
        for (int z = -1; z <= 1; ++z) {
            vge::Vec3 checkPos = playerPos + vge::Vec3(x, y, z);
            vge::BlockType block = world.GetBlock(checkPos);
            
            if (world.IsBlockSolid(block)) {
                // AABB collision with block
                vge::AABB blockBox(
                    vge::Vec3(floor(checkPos.x), floor(checkPos.y), floor(checkPos.z)),
                    vge::Vec3(ceil(checkPos.x), ceil(checkPos.y), ceil(checkPos.z))
                );
                
                vge::AABB playerBox(
                    playerPos - vge::Vec3(playerRadius, 0, playerRadius),
                    playerPos + vge::Vec3(playerRadius, 2, playerRadius)
                );
                
                if (blockBox.Intersects(playerBox)) {
                    // Resolve collision
                    vge::Vec3 penetration = CalculatePenetration(playerBox, blockBox);
                    controller.SetPosition(playerPos - penetration);
                }
            }
        }
    }
}
```

## Physics Materials

```cpp
// Define surface properties
struct PhysicsMaterial {
    float staticFriction = 0.5f;
    float dynamicFriction = 0.3f;
    float restitution = 0.0f;  // Bounciness
};

// Common materials
PhysicsMaterial mat_ice = { 0.1f, 0.05f, 0.0f };      // Slippery
PhysicsMaterial mat_rubber = { 1.0f, 0.8f, 0.9f };    // Bouncy
PhysicsMaterial mat_metal = { 0.4f, 0.3f, 0.2f };     // Hard
PhysicsMaterial mat_wood = { 0.5f, 0.4f, 0.1f };      // Normal

// Apply to body
body.SetMaterial(mat_ice);
```

## Performance Tips

1. **Use layers**: Only check collisions between relevant layers
2. **Sleep bodies**: Disable simulation for resting objects
3. **Broad phase**: Use spatial hashing for many objects
4. **Fixed timestep**: Update physics at consistent rate (60Hz)

```cpp
// Sleep inactive bodies
body.SetSleepThreshold(0.1f);  // Velocity below this = sleep
body.SetCanSleep(true);

// Check if sleeping
if (body.IsSleeping()) {
    // Skip updates
}

// Wake up on collision
body.WakeUp();
```
