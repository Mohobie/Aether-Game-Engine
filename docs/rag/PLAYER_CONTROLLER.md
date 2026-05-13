# Player Controller

## Quick Reference

```cpp
vge::PlayerController player;
player.SetPosition(vge::Vec3(0, 50, 0));

// In game loop
player.Update(deltaTime, input, world);

// Access state
Vec3 pos = player.GetPosition();
float yaw = player.GetYaw();
float pitch = player.GetPitch();
```

## Features

### Movement
- **WASD** - Forward/left/back/right relative to look direction
- **Space** - Jump (only when on ground)
- **Shift** - Sprint (1.5x speed)
- **Mouse** - Look around (yaw/pitch)

### Collision
- AABB collision with world blocks
- Ground detection
- Gravity application

### Block Interaction
- `BreakBlock(world)` - Raycast to break block
- `PlaceBlock(world, type)` - Raycast to place block
- 5 block reach distance

## Configuration

```cpp
player.speed = 8.0f;           // Walk speed
player.jumpForce = 12.0f;      // Jump velocity
player.gravity = 25.0f;        // Gravity acceleration
player.mouseSensitivity = 0.15f; // Mouse look speed
player.height = 1.8f;          // Player height
player.radius = 0.3f;          // Player radius
```

## Implementation Details

### Movement Calculation
```cpp
Vec3 forward = GetLookDirection();
forward.y = 0; // Keep movement horizontal
Vec3 right = GetRightDirection();

Vec3 moveDir = forward * input.moveForward + right * input.moveRight;
// Normalize and apply speed
```

### Collision Detection
```cpp
// Check all blocks in player AABB
int minX = floor(pos.x - radius);
int maxX = floor(pos.x + radius);
// ... similar for Y and Z

for (each block in range) {
    if (block is solid) return true; // Collision
}
```

### Raycast for Block Interaction
```cpp
Vec3 eyePos = position + Vec3(0, height * 0.8f, 0);
Vec3 lookDir = GetLookDirection();

for (float t = 0; t < maxDist; t += step) {
    Vec3 checkPos = eyePos + lookDir * t;
    // Check block at checkPos
}
```

## Files
- `src/core/player_controller.h`
- `src/core/player_controller.cpp`
