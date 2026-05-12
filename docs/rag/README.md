# Aether Game Engine - RAG Documentation

This folder contains comprehensive documentation designed for AI models to understand and use the Aether Game Engine.

## Document Structure

Each document covers a specific system or topic:

| Document | Description | Lines |
|----------|-------------|-------|
| `01-overview.md` | Engine overview, features, project structure, quick start | ~80 |
| `02-math-system.md` | Vec3, Mat4, AABB, transformations, common patterns | ~120 |
| `03-voxel-world.md` | Blocks, chunks, world generation, raycasting, save/load | ~170 |
| `04-rendering-system.md` | Camera, meshes, shaders, lighting, shadows, post-processing | ~280 |
| `05-entity-component-system.md` | Entities, components, systems, prefabs, events | ~290 |
| `06-physics-system.md` | RigidBody, physics world, collisions, character controller | ~240 |
| `07-input-system.md` | Keyboard, mouse, gamepad, input mapping, contexts | ~340 |
| `08-ui-system.md` | UI elements, layouts, inventory UI, HUD, console | ~520 |
| `09-audio-system.md` | Sound manager, 3D audio, music, ambient, mixer | ~380 |
| `10-ai-system.md` | Behavior trees, state machines, pathfinding, perception | ~540 |
| `11-networking.md` | Client-server, packets, entity sync, chunk streaming | ~480 |
| `12-editor-tools.md` | Editor mode, block editing, entity spawning, console commands | ~300 |
| `13-common-patterns.md` | Player controller, inventory, crafting, save/load, game loop | ~760 |
| `14-inventory-crafting.md` | Item system, inventory, drag-and-drop, crafting, smelting | ~350 |
| `15-entity-archetypes.md` | Generic entity system, archetypes, environmental effects | ~280 |
| `16-audio-events.md` | Decoupled audio event system, sound packs, JSON loading | ~220 |
| `17-chunk-system.md` | Chunk architecture, persistence, streaming, LOD, culling | ~200 |
| `18-network-system.md` | Client-server multiplayer, entity replication, server authority | ~250 |
| `19-editor-system.md` | In-game editor, ImGui, debug visualization, viewports, asset browser | ~350 |

## How to Use This Documentation

### For AI Models

1. **Start with `01-overview.md`** to understand the engine's architecture
2. **Reference specific systems** as needed during code generation
3. **Use `13-common-patterns.md`** for complete game implementations
4. **Use `14-inventory-crafting.md`** for item/crafting systems
5. **Use `15-entity-archetypes.md`** for entity/AI systems
6. **Use `19-editor-system.md`** for editor, debug visualization, viewports
7. **All code examples are copy-paste ready** with proper includes

### Key Conventions

- **Namespace**: All engine code is in `vge::` namespace (unified 2026-05-12)
- **Math**: Column-major matrices, Y-is-up coordinate system
- **Blocks**: Minecraft-style voxels with 16x16x16 chunks
- **ECS**: Entity-Component-System architecture
- **Rendering**: OpenGL-based with modern features
- **Entity System**: Fully generic - no built-in entity types
- **Environmental Effects**: Configurable per entity (sunlight/water/darkness)
- **Item Categories**: Block, Tool, Weapon, Armor, Consumable, Material, Misc

### Quick Reference

```cpp
// Include common headers
#include "voxel/world.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "entity/entity.h"
#include "entity/components.h"
#include "physics/physics_world.h"
#include "platform/input_manager.h"
#include "ui/ui_system.h"
#include "audio/audio_engine.h"
#include "ai/entity_ai.h"
#include "core/item_system.h"
#include "core/crafting.h"
#include "editor/in_game_editor.h"
#include "editor/scene_viewport.h"
#include "editor/asset_browser.h"
#include "debug/debug_renderer.h"

// Common types
vge::Vec3    // 3D vector
vge::Mat4    // 4x4 matrix
vge::World   // Voxel world
vge::EntityID // Entity handle
vge::Inventory // Item inventory
vge::CraftingSystem // Crafting recipes
vge::InGameEditor   // In-game editor
vge::ViewportManager // Multi-viewport system
vge::AssetBrowser   // Asset browser
vge::DebugRenderer  // Debug visualization
```

## Building with the Engine

```bash
# Clone repository
git clone http://192.168.1.189:3100/aether/aether-game-engine.git
cd aether-game-engine

# Build
mkdir build && cd build
cmake ..
make -j4

# Run tests
./voxel_test
```

## Dependencies

- OpenGL 3.3+
- GLFW (window/input)
- GLM (math) - wrapped in vge:: types
- stb_image (texture loading)
- miniaudio (audio)
- enet (networking)
- nlohmann/json (JSON serialization)

## License

MIT License - Free for commercial and non-commercial use.
