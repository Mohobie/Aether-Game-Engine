# Aether Game Engine - API Reference

> **Auto-generated** from actual C++ header files in `src/`  
> **Date:** 2026-05-10  
> **Total Headers:** 87  
> **Modules:** 15

---

## Module Index

| Module | Description | Files | API Doc |
|--------|-------------|-------|---------|
| **Math** | Vec3, Mat4, Noise | 3 | [API-Math](API-Math) |
| **Core** | Types, Logger, Profiler, Time, Save, Config, Achievements, Crafting, Inventory, Raycast, PlayerController, Utils, Application, Engine, EventSystem, JobSystem, Math, Memory, Serializer | 21 | [API-Core](API-Core) |
| **Entity** | Entity, Component, EntityManager, System | 4 | [API-Entity](API-Entity) |
| **Voxel** | Block, Chunk, World, WorldGenerator, WorldRenderer, Biome, BlockRegistry, BlockMeshBuilder, ChunkManager | 9 | [API-Voxel](API-Voxel) |
| **Render** | Camera, Mesh, Renderer, Shader, Texture | 5 | [API-Render](API-Render) |
| **Rendering** | Camera, FramebufferRenderer, Lighting, Material, Mesh, Renderer, Shader, Texture | 8 | [API-Rendering](API-Rendering) |
| **Physics** | AABB, Collider, Collision, PhysicsWorld, Ray, RigidBody | 6 | [API-Physics](API-Physics) |
| **Audio** | AudioEngine, MusicPlayer, Sound, SoundManager | 4 | [API-Audio](API-Audio) |
| **Input** | InputManager, KeyBindings | 2 | [API-Input](API-Input) |
| **Platform** | FileSystem, Input, Threading, Timer, Window | 5 | [API-Platform](API-Platform) |
| **Scripting** | LuaEngine, ScriptBindings, ScriptEngine | 3 | [API-Scripting](API-Scripting) |
| **UI** | Button, Console, ImGuiWrapper, Label, MenuSystem, Panel, UIElement, UIManager | 8 | [API-UI](API-UI) |
| **Network** | Client, NetworkManager, Packet | 3 | [API-Network](API-Network) |
| **Game** | Application, GameState, Scene, Serializer | 4 | [API-Game](API-Game) |
| **Resource** | AssetLoader, ConfigManager, ResourceManager | 3 | [API-Resource](API-Resource) |

---

## Canonical Module Map

This table reflects the 2026-05-14 repo audit and the current CMake target layout.

| Area | Canonical | Legacy / inactive |
|------|-----------|-------------------|
| Rendering | `src/rendering/*` | `src/render/*` |
| Input | `src/platform/input_manager.*` | `src/input/*`, `src/platform/input.*` |
| Application facade | `src/game/application.*` and `include/aether_engine.h` | `src/core/application.*` |
| Entity layer | `src/entity/entity.*`, `src/entity/components.*` | `src/core/entity.*`, `src/entity/component.*`, `src/entity/system.*` |
| Save and serialization | `src/core/save_system.*` for direct world save/load, with `src/voxel/world_serializer.*` as the active lower-level world serializer | `src/game/save_system.*`, `src/game/serializer.*`; `src/core/serializer.*` is a generic byte-buffer helper, not the active save API |

### Build Truth Notes

1. `voxel_engine_lib` now owns the canonical runtime sources; `voxel_engine` only adds `src/main.cpp` plus the optional ImGui/editor stack when bundled ImGui files are actually present.
2. The legacy duplicate ownership paths above remain in the tree for reference, but they are not the supported build path for this session.
3. This audit was static only in the current shell: `cmake` is not available on `PATH`, so configure/build/test execution was not revalidated here.

---

## Namespaces Used

| Namespace | Modules Using It |
|-----------|-----------------|
| `vge` | All modules (unified namespace) |

---

## Key Observations

1. **Unified runtime namespace**: active code paths are `vge`, but several generated API pages still mention `aether`
2. **Canonical rendering path**: `rendering/` is the supported renderer family; `render/` is legacy
3. **Canonical input path**: `platform/input_manager.*` is the supported input path in the current build; `src/input/*` and `platform/input.*` are legacy
4. **Layered save path**: `core/save_system.*` and `voxel/world_serializer.*` are the current build-backed save layers; `game/save_system.*` and `game/serializer.*` are inactive
5. **Some module pages are stale**: use this index as the architecture source of truth until the per-module API pages are regenerated

---

*Generated from actual source code*
