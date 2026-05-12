# Aether Game Engine - Overview

## What is This Engine?

Aether Game Engine is a C++ voxel-based game engine with modern rendering, physics, and gameplay systems. It uses OpenGL for graphics and follows an Entity-Component-System (ECS) architecture.

## Key Features

- **Voxel World**: Minecraft-style block-based world with infinite terrain generation
- **Modern Rendering**: PBR materials, shadows, post-processing, day/night cycle
- **Physics**: Rigidbody physics with collision detection
- **AI**: Behavior trees, pathfinding, state machines, fully configurable environmental effects
- **Networking**: Client-server multiplayer architecture
- **Editor**: In-game editor for level design and testing
- **Inventory**: Grid-based inventory with drag-and-drop, crafting, and item categories
- **Audio**: 3D positional audio with event-driven sound packs
- **Entity System**: Fully generic, data-driven entity archetypes (no built-in types)

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
#include "ai/entity_ai.h"

int main() {
    // Create a world
    vge::World world;
    world.Initialize();
    
    // Generate terrain
    world.GenerateTerrain(0, 0);  // Generate at chunk (0,0)
    
    // Place a block
    world.SetBlock(vge::Vec3(5, 10, 5), vge::BlockType::Stone);
    
    // Define a custom entity archetype (your game defines what exists)
    vge::EntityArchetype spacePirate;
    spacePirate.id = "space_pirate";
    spacePirate.displayName = "Space Pirate";
    spacePirate.health = 75.0f;
    spacePirate.speed = 8.0f;
    spacePirate.damage = 15.0f;
    spacePirate.behavior = vge::AIBehaviorType::Aggressive;
    spacePirate.canFly = true;
    spacePirate.damagedBySunlight = false;  // Space helmet
    vge::EntityArchetypeRegistry::GetInstance()->RegisterArchetype(spacePirate);
    
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
6. **Entity Archetypes**: Fully generic - your game defines all entity types
7. **Environmental Effects**: Configurable per entity (sunlight/water/darkness damage/heal)
8. **Item Categories**: Block, Material, Consumable, Weapon, Armor, Tool, Misc
