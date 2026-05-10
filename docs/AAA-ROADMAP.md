# Aether Engine - AAA Roadmap

## Current State Analysis

### Problems Identified
1. **Hardcoded Minecraft content**: DiamondOre, CraftingTable, Furnace, etc.
2. **Hardcoded biomes**: Only Plains, Forest, Desert, Snow
3. **Hardcoded generation**: Trees are hardcoded in world generator
4. **No data-driven design**: Everything is compiled into code
5. **Limited rendering**: ASCII/software renderer only
6. **No asset pipeline**: No way to load custom models, textures, sounds
7. **No modding support**: Everything is hardcoded

---

## Phase 1: Data-Driven Foundation (Critical)

### 1.1 Asset Pipeline
- [ ] Create asset directory structure (assets/models, assets/textures, assets/sounds, assets/scripts)
- [ ] JSON-based block definitions (replace hardcoded BlockType enum)
- [ ] JSON-based biome definitions
- [ ] JSON-based entity definitions
- [ ] Hot-reloading of assets during development

### 1.2 Block System Rewrite
- [ ] Replace `BlockType` enum with string-based block IDs
- [ ] Block properties in JSON (solid, opaque, hardness, color, texture, emission)
- [ ] Block behaviors via scripting (Lua)
- [ ] Custom block models (not just cubes)

### 1.3 Content Removal
- [ ] Remove all Minecraft-specific blocks (DiamondOre, CraftingTable, etc.)
- [ ] Replace with generic blocks (Stone, Dirt, generic ore types)
- [ ] Make game-specific content loadable via mods/scripts

---

## Phase 2: Modern Rendering (Critical for AAA)

### 2.1 Graphics API Abstraction
- [ ] Abstract renderer (support OpenGL, Vulkan, DirectX, Metal)
- [ ] Shader pipeline (GLSL, HLSL, SPIR-V)
- [ ] Material system (PBR - Physically Based Rendering)
- [ ] Post-processing pipeline (bloom, SSAO, SSR, tone mapping)

### 2.2 Advanced Rendering Features
- [ ] Deferred shading
- [ ] Shadow mapping (cascaded)
- [ ] Global illumination (light probes, RT if available)
- [ ] Volumetric fog/clouds
- [ ] Particle systems
- [ ] Animation system (skeletal, blend shapes)

### 2.3 World Rendering
- [ ] LOD system (Level of Detail) for chunks
- [ ] Occlusion culling
- [ ] Instanced rendering
- [ ] Texture atlases/array textures
- [ ] GPU-driven rendering

---

## Phase 3: World Generation Overhaul

### 3.1 Procedural Generation Framework
- [ ] Node-based world generation graph
- [ ] Multiple noise types (Perlin, Simplex, Voronoi, Worley)
- [ ] Erosion simulation
- [ ] Cave generation (3D noise + cellular automata)
- [ ] River/lake generation

### 3.2 Biome System
- [ ] Biome parameters (temperature, humidity, elevation, weirdness)
- [ ] Biome blending (smooth transitions)
- [ ] Sub-biomes and micro-climates
- [ ] Custom biome definitions via JSON

### 3.3 Structure Generation
- [ ] Procedural building generation
- [ ] Dungeon/cave systems
- [ ] Vegetation placement (not just trees)
- [ ] Ore distribution via rules

---

## Phase 4: Entity & Gameplay Systems

### 4.1 Entity Component System (ECS)
- [ ] Proper ECS architecture (not just basic OOP)
- [ ] Component serialization
- [ ] Entity prefabs
- [ ] Entity scripting (Lua/C#)

### 4.2 Physics
- [ ] Integrate Bullet/PhysX/Jolt physics
- [ ] Rigid body dynamics
- [ ] Character controller (capsule, not just AABB)
- [ ] Raycasting improvements
- [ ] Vehicle physics

### 4.3 AI
- [ ] Behavior trees
- [ ] Pathfinding (A*, navmesh)
- [ ] State machines
- [ ] Perception system (sight, hearing)

---

## Phase 5: Audio & Effects

### 5.1 Audio Engine
- [ ] 3D positional audio (OpenAL/SteamAudio)
- [ ] Sound occlusion
- [ ] Dynamic music system
- [ ] Audio mixing (zones, reverb)

### 5.2 Visual Effects
- [ ] Particle systems (fire, smoke, water, magic)
- [ ] Decals
- [ ] Screen-space effects
- [ ] Weather systems (rain, snow, storms)

---

## Phase 6: Tools & Editor

### 6.1 In-Game Editor
- [ ] Level editor mode
- [ ] Block placement/removal tools
- [ ] Terrain sculpting
- [ ] Entity placement
- [ ] Trigger/volume editing

### 6.2 External Tools
- [ ] World editor application
- [ ] Asset browser
- [ ] Material editor
- [ ] Script debugger

---

## Phase 7: Multiplayer & Networking

### 7.1 Networking
- [ ] Client-server architecture
- [ ] Entity replication
- [ ] Chunk streaming to clients
- [ ] Lag compensation
- [ ] Anti-cheat basics

### 7.2 Server Features
- [ ] Dedicated server build
- [ ] Server-side physics
- [ ] Persistence (world save/load)
- [ ] Player accounts/authentication

---

## Phase 8: Performance & Optimization

### 8.1 Memory
- [ ] Pool allocators for blocks/entities
- [ ] Memory budgeting
- [ ] Asset streaming

### 8.2 CPU
- [ ] Job system (multithreaded chunk generation)
- [ ] Parallel chunk meshing
- [ ] SIMD optimizations

### 8.3 GPU
- [ ] Frustum culling
- [ ] Occlusion queries
- [ ] GPU culling
- [ ] Async compute

---

## Phase 9: Platform & Deployment

### 9.1 Platforms
- [ ] Windows (primary)
- [ ] Linux
- [ ] macOS
- [ ] Console support (future)

### 9.2 Distribution
- [ ] Installer/packaging
- [ ] Auto-updater
- [ ] Crash reporter
- [ ] Analytics (optional)

---

## Immediate Next Steps (Priority Order)

1. **Remove Minecraft content** - Make engine generic
2. **JSON block definitions** - Data-driven blocks
3. **Asset pipeline** - Load external assets
4. **Modern renderer** - OpenGL 4.5+/Vulkan
5. **ECS rewrite** - Proper entity system
6. **Physics integration** - Real physics engine

---

*Created 2026-05-10*
