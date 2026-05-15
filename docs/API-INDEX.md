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
| **Core** | Types, Logger, Time, Save, Config, PlayerController, Serializer helper; legacy Application and Entity headers still exist in-tree | 21 | [API-Core](API-Core) |
| **Entity** | Entity, Component, EntityManager, System | 4 | [API-Entity](API-Entity) |
| **Voxel** | Block, Chunk, World, WorldGenerator, WorldRenderer, Biome, BlockRegistry, BlockMeshBuilder, ChunkManager | 9 | [API-Voxel](API-Voxel) |
| **Render** | Camera, Mesh, Renderer, Shader, Texture | 5 | [API-Render](API-Render) |
| **Rendering** | Camera, FramebufferRenderer, Lighting, Material, Mesh, Renderer, Shader, Texture | 8 | [API-Rendering](API-Rendering) |
| **Physics** | AABB, Collider, Collision, PhysicsWorld, Ray, RigidBody | 6 | [API-Physics](API-Physics) |
| **Audio** | AudioEngine, MusicPlayer, Sound, SoundManager | 4 | [API-Audio](API-Audio) |
| **Input** | Legacy input docs retained as a compatibility pointer; canonical runtime input lives under Platform | 2 | [API-Input](API-Input) |
| **Platform** | FileSystem, canonical Input, Threading, Timer, Window | 5 | [API-Platform](API-Platform) |
| **Scripting** | LuaEngine, ScriptBindings, ScriptEngine | 3 | [API-Scripting](API-Scripting) |
| **UI** | Button, Console, ImGuiWrapper, Label, MenuSystem, Panel, UIElement, UIManager | 8 | [API-UI](API-UI) |
| **Network** | Client, NetworkManager, Packet | 3 | [API-Network](API-Network) |
| **Game** | Application, GameState, Scene; legacy Serializer header still exists in-tree | 4 | [API-Game](API-Game) |
| **Resource** | AssetLoader, ConfigManager, ResourceManager | 3 | [API-Resource](API-Resource) |

---

## Canonical Module Map

This table reflects the 2026-05-15 repo audit and the current CMake target layout.

| Area | Canonical | Legacy / inactive |
|------|-----------|-------------------|
| Rendering | `src/rendering/*` | `src/render/*` |
| Input | `src/platform/input_manager.*` for the active windowed/runtime build | `src/input/*`, plus `src/platform/input.*` retained only for older headless/engine paths that are not in the active library target |
| Application facade | `src/game/application.*`, `include/aether_engine.h`, and compatibility include `include/voxel_engine.h` | `src/core/application.*` |
| Entity layer | `src/entity/entity.*`, `src/entity/components.*` | `src/core/entity.*`, `src/entity/component.*`, `src/entity/system.*` |
| Save and serialization | `src/core/save_system.*` for direct world/chunk binary IO, `src/voxel/world_serializer.*` for richer world snapshots, and `src/core/save_game.*` as the current higher-level save wrapper | `src/game/save_system.*`, `src/game/serializer.*`; `src/core/serializer.*` is a generic byte-buffer helper, not the active save API |

### Build Truth Notes

1. `voxel_engine_lib` owns the canonical runtime sources. Legacy parallel implementations such as `src/network/packet.cpp` and `src/ui/ui_manager.cpp` are intentionally excluded from the active library target in favor of `network_architecture.*` and `ui_system.*`.
2. The legacy duplicate ownership paths above remain in the tree for reference, but they are not the supported build path for this session.
3. Verified in this shell on 2026-05-15 using `C:/Program Files/CMake/bin/cmake.exe` from the existing `build-vs` cache: `voxel_engine_lib`, `save_test`, `world_test`, and `voxel_test` build and the three test executables pass. `voxel_engine` is still skipped here because `GLFW` is unavailable.

---

## Namespaces Used

| Namespace | Modules Using It |
|-----------|-----------------|
| `vge` | All modules (unified namespace) |

---

## Key Observations

1. **Unified runtime namespace**: active code paths are `vge`, but several generated API pages still mention `aether`
2. **Canonical rendering path**: `rendering/` is the supported renderer family; `render/` is legacy
3. **Canonical input path**: `platform/input_manager.*` is the supported input path in the active audited build; `src/input/*` is legacy and `platform/input.*` remains only as an older compatibility path outside the active library target
4. **Layered save path**: `core/save_system.*` handles direct world/chunk files, `voxel/world_serializer.*` handles richer world snapshots, and `core/save_game.*` is the active higher-level save wrapper; `game/save_system.*` and `game/serializer.*` are inactive
5. **Some module pages are stale**: use this index as the architecture source of truth until the per-module API pages are regenerated

---

*Generated from actual source code*
