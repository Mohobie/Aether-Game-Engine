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

## Namespaces Used

| Namespace | Modules Using It |
|-----------|-----------------|
| `vge` | Math, Core (partial), Voxel, Render, Rendering, Physics (partial), Audio (partial), Platform (partial), Scripting, Network, UI (partial) |
| `aether` | Core (partial), Entity, Game, Physics (partial), Audio (partial), Platform (partial), Resource, UI (partial) |

---

## Key Observations

1. **Dual namespaces**: The codebase uses both `vge` and `aether` namespaces inconsistently
2. **Duplicate modules**: `render/` and `rendering/` both exist with similar classes
3. **Missing implementations**: Some headers have no corresponding .cpp files
4. **Platform abstraction**: Window uses GLFW, FileSystem is minimal

---

*Generated from actual source code*