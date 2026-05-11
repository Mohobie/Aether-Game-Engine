# Aether Game Engine - Overview

## What is This Engine?

Aether Game Engine is a C++ voxel-based game engine with modern rendering, physics, and gameplay systems. It uses OpenGL for graphics and follows an Entity-Component-System (ECS) architecture.

## Key Features

- **Voxel World**: Minecraft-style block-based world with infinite terrain generation
- **Modern Rendering**: PBR materials, shadows, post-processing, day/night cycle
- **Physics**: Rigidbody physics with collision detection
- **AI**: Behavior trees, pathfinding, state machines
- **Networking**: Client-server multiplayer architecture
- **Editor**: In-game editor for level design and testing

## Project Structure

```
aether-game-engine/
├── src/
│   ├── core/           # Engine core (math, logging, config, time)
│   ├── rendering/      # Graphics (OpenGL, shaders, meshes, textures)
│   ├── voxel/          # Block world (chunks, terrain, block types)
│   ├── physics/        # Physics engine (rigidbodies, collisions)
│   ├── entity/         # ECS (entities, components)
│   ├── ai/             # AI systems (behavior trees, pathfinding)
│   ├── audio/          # Sound engine
│   ├── ui/             # User interface system
│   ├── platform/       # Input, window, file system
│   ├── network/        # Multiplayer networking
│   └── editor/         # In-game editor tools
├── tests/              # Unit tests for all systems
└── docs/               # Documentation
```

## Building the Engine

```bash
mkdir build && cd build
cmake ..
make -j4
```

## Running Tests

```bash
./editor_test      # Editor system tests
./inventory_test   # Inventory system tests
./culling_test     # Culling/LOD tests
./day_night_test   # Day/night cycle tests
./save_test        # Save/load tests
./console_test     # Console command tests
```

## Quick Start Example

```cpp
#include "voxel/world.h"
#include "rendering/renderer.h"

int main() {
    // Create a world
    vge::World world;
    world.Initialize();
    
    // Generate terrain
    world.GenerateTerrain(0, 0);  // Generate at chunk (0,0)
    
    // Place a block
    world.SetBlock(vge::Vec3(5, 10, 5), vge::BlockType::Stone);
    
    // Render loop
    while (running) {
        world.Update(deltaTime);
        renderer.Render(world);
    }
    
    return 0;
}
```

## Core Concepts

1. **Chunks**: 16x16x16 blocks. Worlds are made of chunks.
2. **Blocks**: Each block has a type (Stone, Dirt, Grass, etc.)
3. **Entities**: Game objects with components (position, mesh, physics)
4. **Systems**: Process entities (rendering, physics, AI)
5. **Events**: Communication between systems (input, collisions, etc.)
