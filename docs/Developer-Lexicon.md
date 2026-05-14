# Aether Game Engine - Developer Lexicon

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                          │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐         │
│  │  Game    │ │  Craft   │ │  Quest   │ │  Dialog  │         │
│  │  State   │ │  System  │ │  Manager │ │  Tree    │         │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘         │
├─────────────────────────────────────────────────────────────┤
│                    CORE SYSTEMS                               │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐         │
│  │  Entity  │ │  Input   │ │  Audio   │ │   Save   │         │
│  │  System  │ │  System  │ │  Engine  │ │  System  │         │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘         │
├─────────────────────────────────────────────────────────────┤
│                    WORLD SIMULATION                           │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐         │
│  │  World   │ │  Chunk   │ │  Block   │ │  Physics │         │
│  │  Manager │ │  Manager │ │  Registry│ │  Engine  │         │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘         │
├─────────────────────────────────────────────────────────────┤
│                    RENDERING PIPELINE                           │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐         │
│  │  OpenGL  │ │ Framebuf │ │  ASCII   │ │  Mesh    │         │
│  │ Renderer │ │ Renderer │ │ Renderer │ │ Builder  │         │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘         │
├─────────────────────────────────────────────────────────────┤
│                    PLATFORM ABSTRACTION                       │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐         │
│  │  Window  │ │   Lua    │ │  Timer   │ │  Logger  │         │
│  │  (GLFW)  │ │  Engine  │ │          │ │          │         │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘         │
└─────────────────────────────────────────────────────────────┘
```

## Core Concepts

### Entity Component System (ECS)

The engine uses a flexible ECS architecture where game objects are composed of behaviors rather than inherited.

```cpp
// Creating entities
Entity* player = Entity::CreatePlayer(Vec3(0, 64, 0));
Entity* block = Entity::CreateBlockEntity(Vec3(5, 64, 5), BlockType::Stone);

// Accessing components
auto physics = player->GetComponent<PhysicsComponent>();
physics->ApplyForce(Vec3(0, 500, 0)); // Jump!

// Custom components
class HealthComponent : public Component {
public:
    float health = 100.0f;
    float maxHealth = 100.0f;
    
    void TakeDamage(float amount) {
        health -= amount;
        if (health <= 0) {
            // Entity death handled here
        }
    }
};
```

**Key Terms:**
- **Entity** - A game object (player, block, item)
- **Component** - A behavior attached to entities (physics, render, health)
- **System** - Logic that processes entities with specific components

### Voxel World

The world is divided into chunks for efficient memory usage and rendering.

```cpp
// World structure
World (infinite)
  └── Chunk (16x16x16 blocks)
        └── Block (type + metadata)

// Coordinate systems
World Coordinates: (wx, wy, wz) - Global position
Chunk Coordinates: (cx, cy, cz) - Chunk index (world / 16)
Local Coordinates: (lx, ly, lz) - Position within chunk (0-15)
```

**Key Terms:**
- **Chunk** - 16x16x16 block region, loaded/unloaded dynamically
- **BlockType** - Enum defining block behavior (Stone, Dirt, Grass, etc.)
- **BlockRegistry** - Maps IDs to block properties
- **WorldGenerator** - Procedural terrain generation
- **Biome** - Environmental region with specific block distributions

### Rendering Pipeline

The engine supports multiple renderers for different use cases:

| Renderer | Use Case | Performance |
|----------|----------|-------------|
| OpenGL | Full graphics, shaders, textures | GPU accelerated |
| Framebuffer | Direct pixel access, no X11 | CPU, medium |
| ASCII | Terminal/debug output | CPU, low |

```cpp
// Renderer selection is automatic
Renderer renderer;
renderer.Initialize(); // Tries OpenGL -> Framebuffer -> ASCII

// Rendering the world
renderer.BeginFrame();
renderer.RenderWorld(world, camera);
renderer.EndFrame();
```

**Key Terms:**
- **Mesh** - Collection of vertices/triangles for GPU rendering
- **Shader** - GPU program for vertex/fragment processing
- **Framebuffer** - Memory buffer for pixel storage
- **Camera** - View position and orientation
- **Projection** - 3D to 2D coordinate transformation

### Physics System

```cpp
// Collision detection
AABB playerBox(position, Vec3(0.6f, 1.8f, 0.6f));
AABB blockBox(blockPos, Vec3(1.0f, 1.0f, 1.0f));

if (AABB::Intersects(playerBox, blockBox)) {
    // Handle collision response
}

// Raycasting (block selection)
Ray ray(cameraPos, cameraForward);
RaycastHit hit;
if (Raycast::World(world, ray, 5.0f, hit)) {
    // hit.blockPosition - The block hit
    // hit.normal - Face normal for placement
}
```

**Key Terms:**
- **AABB** - Axis-Aligned Bounding Box for collision
- **Raycast** - Line intersection test for block selection
- **RigidBody** - Physics object with mass and velocity
- **Collision Response** - How objects react to intersection

## API Reference

### Math Library

```cpp
Vec3  // 3D vector (x, y, z)
Vec2  // 2D vector (x, y)
Mat4  // 4x4 matrix (transformations)

// Common operations
Vec3 a(1, 2, 3);
Vec3 b = a + Vec3(4, 5, 6);  // Addition
float len = a.length();        // Magnitude
Vec3 norm = a.normalized();    // Unit vector
float dot = a.dot(b);          // Dot product
Vec3 cross = a.cross(b);       // Cross product
```

### Input System

```cpp
Input input;
input.Initialize();

// Key states
if (input.IsKeyPressed(GLFW_KEY_W))      // Currently held
if (input.IsKeyJustPressed(GLFW_KEY_E)) // Pressed this frame
if (input.IsKeyReleased(GLFW_KEY_ESC))  // Released this frame

// Mouse
input.GetMouseDelta();     // Movement since last frame
input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
```

### Audio Engine

```cpp
AudioEngine audio;
audio.Initialize();

// Load and play sounds
audio.LoadSound("dig", "assets/sounds/dig.wav");
audio.PlaySound("dig");

// Volume control
audio.SetMasterVolume(0.8f);
```

### Save System

```cpp
vge::SaveSystem::SaveWorld(world, "worlds/my_world.bin");
vge::SaveSystem::LoadWorld(world, "worlds/my_world.bin");
```

`vge::SaveSystem` is the direct world/chunk binary API. The current higher-level save wrapper is `vge::SaveGameManager` in `core/save_game.h`, backed by `voxel/world_serializer.h`.

## Scripting with Lua

The engine exposes APIs to Lua for modding:

```lua
-- Access world
local world = engine.getWorld()

-- Place block
world.setBlock(10, 64, 10, "stone")

-- Spawn entity
local zombie = engine.spawnEntity("zombie", 20, 64, 20)
zombie.setHealth(20)

-- Register callback
engine.onUpdate(function(dt)
    -- Called every frame
end)
```

## File Structure

```
aether-game-engine/
├── src/
│   ├── core/           # Engine fundamentals
│   │   ├── entity.cpp      # ECS system
│   │   ├── logger.cpp      # Logging
│   │   ├── save_system.cpp # Serialization
│   │   └── ...
│   ├── voxel/          # World simulation
│   │   ├── world.cpp       # World manager
│   │   ├── chunk.cpp       # Chunk data
│   │   ├── block.cpp       # Block types
│   │   └── ...
│   ├── rendering/      # Graphics
│   │   ├── renderer.cpp    # Main renderer
│   │   ├── mesh.cpp        # Geometry
│   │   └── ...
│   ├── physics/        # Collision & physics
│   │   ├── collision.cpp   # AABB tests
│   │   └── rigidbody.cpp   # Physics objects
│   ├── platform/       # OS abstraction
│   │   ├── window.cpp      # GLFW wrapper
│   │   └── input.cpp       # Input handling
│   ├── audio/          # Sound
│   │   └── audio_engine.cpp
│   ├── game/           # Game logic
│   │   ├── inventory.cpp   # Item management
│   │   ├── crafting.cpp    # Recipe system
│   │   ├── quests.cpp      # Quest tracking
│   │   └── dialog.cpp      # NPC dialog
│   └── scripting/      # Lua integration
│       └── lua_engine.cpp
├── include/            # Public headers
├── assets/             # Game assets
│   ├── sounds/         # Audio files
│   └── textures/       # Image files
├── tests/              # Unit tests
├── build/              # CMake build files
└── CMakeLists.txt      # Build configuration
```

## Build System

```bash
# Debug build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

# Run tests
./voxel_test
./world_test
./save_test
```

## Common Tasks

### Adding a New Block Type

1. Edit `src/voxel/block.h`:
```cpp
enum class BlockType : uint16_t {
    // ... existing types ...
    RubyOre = 50,  // Your new block
};
```

2. Add properties in `BlockRegistry::Initialize()`:
```cpp
RegisterBlock(BlockType::RubyOre, {
    .name = "Ruby Ore",
    .solid = true,
    .color = Vec3(0.8f, 0.1f, 0.2f),
    .hardness = 3.0f
});
```

3. Add generation in `WorldGenerator`:
```cpp
// Spawn ruby ore in caves
if (noiseValue > 0.7f && y < 32) {
    chunk->SetBlock(x, y, z, BlockType::RubyOre);
}
```

### Creating a New Component

```cpp
// MyComponent.h
#pragma once
#include "entity/entity.h"

class MyComponent : public Component {
public:
    float customValue = 0.0f;
    
    void Update(float deltaTime) override {
        // Per-frame logic
        customValue += deltaTime;
    }
};

// Usage
Entity* entity = new Entity("MyEntity");
auto myComp = entity->AddComponent<MyComponent>();
myComp->customValue = 42.0f;
```

### Adding a Lua Binding

```cpp
// In lua_engine.cpp
lua.set_function("spawnParticle", [&](Vec3 pos, Vec3 color) {
    auto particle = Entity::CreateParticle(pos, color);
    world->AddEntity(particle);
});
```

```lua
-- In Lua script
engine.spawnParticle(
    {x=10, y=64, z=10},
    {r=1.0, g=0.5, b=0.0}
)
```

## Performance Guidelines

- **Chunk Updates**: Mark chunks dirty instead of rebuilding meshes every frame
- **Rendering**: Use face culling - only draw exposed block faces
- **Collision**: Use broad-phase filtering before AABB tests
- **Memory**: Pool frequently allocated objects (particles, entities)
- **Lua**: Cache Lua function references, don't look up every frame

## Debugging Tools

```cpp
// Enable debug logging
Logger::SetLevel(LogLevel::Debug);

// Visualize collision boxes
renderer.SetDebugDraw(true);

// Profile performance
Profiler::Begin("Frame");
// ... game code ...
Profiler::End("Frame");
Profiler::PrintResults();
```

## Glossary

| Term | Definition |
|------|------------|
| **AABB** | Axis-Aligned Bounding Box - simple collision volume |
| **Chunk** | 16x16x16 block region of the world |
| **ECS** | Entity Component System - composition-based architecture |
| **Framebuffer** | Memory buffer holding pixel data |
| **GLFW** | Windowing library for OpenGL |
| **Lua** | Lightweight scripting language |
| **Mesh** | Collection of vertices forming 3D geometry |
| **Raycast** | Line intersection test for block selection |
| **Shader** | GPU program for rendering |
| **Voxel** | Volume pixel - a 3D grid cell |
| **World Gen** | Procedural terrain generation algorithm |

## Resources

- **Repository**: https://github.com/Mohobie/Aether-Game-Engine
- **Issues**: Check `.gitea-issues.json` for known bugs
- **Tests**: Run `./voxel_test`, `./world_test`, `./save_test`
- **Build**: `cd build && cmake .. && make -j4`

---

*Last Updated: 2026-05-09*
*Engine Version: 0.1.0*
*Maintainer: Aether <aether@aether-ai.online>*
