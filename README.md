# Aether Game Engine

A modern C++ voxel game engine with advanced rendering, physics, AI, networking, and a fully data-driven entity system. Build any voxel-based game — from survival sandbox to sci-fi shooters — without engine modifications.

## Features

### Voxel World
- **Chunk-based architecture**: 16³ blocks per chunk, infinite world streaming
- **Procedural generation**: Biomes, caves, ores, trees, structures
- **Block system**: 25+ block types with custom properties (solid, transparent, light-emitting)
- **Persistence**: Binary save format with compression, auto-save support
- **LOD system**: Level-of-detail for distant chunks
- **Voxel editor**: In-game editing with undo/redo, brush tools, fill/replace

### Rendering
- **OpenGL 3.3+**: Modern graphics pipeline
- **PBR materials**: Metallic/roughness workflow with albedo, normal, AO maps
- **Deferred rendering**: G-buffer for efficient lighting
- **Shadow mapping**: Cascaded shadow maps for directional lights
- **Post-processing**: Bloom, SSAO, FXAA, tone mapping, vignette, motion blur, depth of field
- **Particle system**: GPU-accelerated particles with emitters, bursts, collision
- **Sky rendering**: Dynamic day/night cycle, stars, clouds, atmospheric scattering
- **Weather effects**: Rain, snow, storms with visual effects
- **Frustum & occlusion culling**: BVH-based culling for performance

### Physics
- **Rigid body dynamics**: Forces, impulses, torque, gravity
- **Collision detection**: AABB, sphere, capsule, mesh colliders
- **Constraints**: Fixed, hinge, spring, distance joints
- **Raycasting**: Ray, sphere, and AABB casts
- **Character controller**: Ground detection, jumping, collision response

### Entity System (Fully Generic)
- **No built-in entities**: Your game defines everything
- **Entity archetypes**: JSON-configurable entity types with stats, behavior, appearance
- **AI behaviors**: Passive, neutral, aggressive, territorial, fleeing, boss
- **Environmental effects**: Per-entity sunlight/water/darkness damage or healing
- **Movement types**: Walking, flying, swimming, wall-climbing
- **Custom properties**: Float, string, bool properties per archetype
- **Spawning**: Single entities, groups, conditional spawning (light level, time, biome)

### Inventory & Crafting
- **Item system**: Categories (block, tool, weapon, armor, consumable, material, misc)
- **Inventory management**: Grid-based with drag-and-drop, filtering, sorting
- **Crafting system**: Shaped and shapeless recipes with substitutes
- **Smelting**: Furnace system with fuel types and cook times
- **Recipe book**: Searchable, filterable recipe browser

### Audio
- **3D positional audio**: Distance attenuation, occlusion
- **Sound packs**: Game-agnostic event system — define events, provide sounds
- **Music player**: Playlist support, fade in/out, shuffle, looping
- **Ambient sounds**: Zone-based ambient loops

### Networking
- **Client-server architecture**: Authoritative server with client prediction
- **Entity replication**: Automatic sync with interpolation/extrapolation
- **Chunk streaming**: Server sends chunks as players explore
- **Server authority**: Anti-cheat validation, speed/reach checks
- **Networked game modes**: Built-in support for multiplayer game types

### UI System
- **ImGui integration**: Full Dear ImGui wrapper
- **UI elements**: Buttons, labels, panels, sliders, checkboxes, dropdowns
- **Inventory UI**: Drag-and-drop with tooltips, hotbar, crafting grid
- **Console**: In-game command console with auto-complete, history
- **Menu system**: Main menu, pause, settings, inventory, crafting screens

### Scripting
- **Lua integration**: Full Lua 5.3+ support
- **Engine API**: Exposed to Lua — world, entities, rendering, audio, UI
- **Mod system**: Hot-reloadable mods with dependency management

### Editor
- **In-game editor**: Toggle with key press
- **Tools**: Select, move, rotate, scale, paint, erase, fill, eyedropper
- **Entity spawning**: Spawn any archetype at cursor or camera
- **Gizmos**: Transform gizmos for selected entities
- **Scene save/load**: Save and load entire scenes

### Platform
- **Cross-platform**: Windows, Linux, macOS
- **Window management**: Fullscreen, borderless, windowed, VSync
- **Input**: Keyboard, mouse, gamepad with rumble support
- **File system**: Cross-platform paths, file watching, async I/O

## Building

### Requirements
- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.14+
- OpenGL 3.3+ capable GPU

### Dependencies (included or auto-fetched)
- GLFW 3.3+ (window/input)
- GLM (math library)
- Dear ImGui (UI)
- stb_image (texture loading)
- miniaudio (audio)
- enet (networking)
- nlohmann/json (JSON serialization)
- Lua 5.3+ (scripting)

### Build Instructions

```bash
# Clone the repository
git clone http://192.168.1.189:3100/aether/aether-game-engine.git
cd aether-game-engine

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build (parallel)
cmake --build . --parallel

# Run tests
./voxel_test

# Run the engine
./voxel_engine
```

### Windows
```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Project Structure

```
aether-game-engine/
├── src/                      # Source code
│   ├── ai/                   # AI system, behavior trees, entity archetypes
│   ├── animation/            # Animation clips, state machines
│   ├── audio/                # Audio engine, sound packs, music
│   ├── core/                 # Core systems (inventory, crafting, items, achievements)
│   ├── debug/                # Debug drawing system
│   ├── editor/               # In-game editor tools
│   ├── entity/               # ECS (entities, components, systems)
│   ├── game/                 # Game modes (sandbox, survival), player data
│   ├── input/                # Input manager, key bindings
│   ├── math/                 # Vec2, Vec3, Mat4, noise generators
│   ├── network/              # Networking, replication, server authority
│   ├── physics/              # Physics world, rigid bodies, collisions, constraints
│   ├── platform/             # Window, input, file system, threading, timers
│   ├── render/               # Legacy render module (use rendering/ instead)
│   ├── rendering/            # Modern renderer (PBR, shadows, post-processing, particles)
│   ├── resource/             # Asset loading, resource management
│   ├── scripting/            # Lua engine, mod system
│   ├── ui/                   # UI system, inventory UI, console
│   └── voxel/                # Voxel world, chunks, generation, block types
├── include/                  # Third-party headers (nlohmann/json, etc.)
├── docs/                     # Documentation
│   ├── API-Core.md           # Core module API
│   ├── API-INDEX.md          # API index
│   ├── ENTITY_SYSTEM.md      # Entity system documentation
│   ├── WIKI.md               # Wiki home
│   └── rag/                  # RAG docs for AI (18 documents)
├── assets/                   # Default assets
├── tests/                    # Unit tests
├── CMakeLists.txt           # CMake configuration
└── README.md                # This file
```

## Quick Start

**New to the engine?** Read the [Developer Quick Start Guide](docs/QUICKSTART.md) for a step-by-step tutorial covering:
- Building on Linux/macOS/Windows
- Creating your first voxel world
- Adding entities and player controllers
- Block interaction, inventory, crafting
- Audio, save/load, editor usage
- Debug visualization and troubleshooting

### Quick Example
```cpp
#include "voxel/world.h"
#include "rendering/renderer.h"

int main() {
    // Create world
    vge::World world;
    world.Initialize();
    
    // Generate terrain
    world.GenerateTerrain(0, 0);
    
    // Place blocks
    world.SetBlock(vge::Vec3(5, 10, 5), vge::BlockType::Stone);
    
    return 0;
}
```

### Defining an Entity Archetype
```cpp
// Your game defines what exists — not the engine
vge::EntityArchetype vampire;
vampire.id = "vampire";
vampire.displayName = "Vampire";
vampire.health = 100;
vampire.speed = 6;
vampire.damage = 20;
vampire.behavior = vge::AIBehaviorType::Aggressive;
vampire.damagedBySunlight = true;   // Burns in sun
vampire.healedByDarkness = true;     // Heals in dark
vampire.canFly = true;

vge::EntityArchetypeRegistry::GetInstance()->RegisterArchetype(vampire);
```

### Loading from JSON
```json
{
  "id": "space_pirate",
  "displayName": "Space Pirate",
  "health": 75,
  "speed": 8,
  "behavior": "aggressive",
  "canFly": true,
  "damagedBySunlight": false,
  "customFloats": {
    "shield_strength": 500
  }
}
```

## Documentation

- **API Reference**: [Gitea Wiki](http://192.168.1.189:3100/aether/aether-game-engine/wiki)
- **RAG Docs**: `docs/rag/` — AI-optimized documentation with copy-paste examples
- **Entity System**: `docs/ENTITY_SYSTEM.md`

## Modules

| Module | Files | Description |
|--------|-------|-------------|
| **Math** | 5 | Vec2, Vec3, Mat4, Perlin/Simplex/Value/Worley noise |
| **Core** | 30+ | Logger, profiler, time, save, config, achievements, crafting, inventory, items |
| **Entity** | 8 | ECS with Transform, Mesh, Physics, AI, Health, Inventory components |
| **Voxel** | 16 | Block types, chunks, world generation, terrain, voxel editor |
| **Rendering** | 24 | Camera, PBR, shadows, post-processing, particles, sky, weather |
| **Physics** | 14 | Rigid bodies, collisions, constraints, raycasting, character controller |
| **Audio** | 12 | 3D audio, sound packs, music player, ambient zones |
| **Input** | 6 | Keyboard, mouse, gamepad, key bindings |
| **Platform** | 12 | Window, file system, threading, timers, cross-platform utils |
| **Scripting** | 8 | Lua engine, bindings, mod system |
| **UI** | 16 | ImGui wrapper, buttons, labels, panels, inventory UI, console |
| **Network** | 12 | Client-server, entity replication, server authority, chunk streaming |
| **Game** | 20+ | Game states, sandbox/survival modes, player data, quests, dialog |
| **Resource** | 10 | Asset loader, resource manager, config manager |
| **AI** | 12 | Behavior trees, state machines, pathfinding, entity archetypes |
| **Animation** | 4 | Animation clips, state machines |
| **Debug** | 2 | Debug drawing (lines, boxes, spheres, text) |
| **Editor** | 4 | In-game editor, entity spawner |

**Total: 20 modules, 140+ source files, 87 headers**

## Current Status

- ✅ 140+ source files (.cpp + .h)
- ✅ 20 engine modules
- ✅ CMake build system
- ✅ Gitea repository with wiki
- ✅ Comprehensive API documentation
- ✅ RAG docs for AI assistance
- ✅ Unit tests (voxel_test)
- 🔄 Active development

## License

MIT License — Free for commercial and non-commercial use.

---

*Built by Aether AI | [Repository](http://192.168.1.189:3100/aether/aether-game-engine)*
