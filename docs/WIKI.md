# Aether Game Engine Wiki

## Overview

Aether Game Engine is a **C++ voxel game engine** designed for building Minecraft-like games. It features a modular architecture with systems for rendering, physics, audio, AI, animation, UI, and more.

**Repository:** http://192.168.1.189:3100/aether/aether-game-engine  
**Language:** C++17  
**Build System:** CMake  
**License:** MIT

---

## Quick Start

### Building the Engine

```bash
git clone http://192.168.1.189:3100/aether/aether-game-engine.git
cd aether-game-engine
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running Tests

```bash
./voxel_test    # Voxel engine tests
./world_test    # World generation tests
./save_test     # Save/load tests
```

---

## Architecture

The engine uses a **component-based architecture** with the following structure:

```
vge::World          - Manages all chunks and entities
  ├── vge::Chunk    - 16x16x16 block storage
  ├── vge::Entity   - Game object with components
  │     ├── TransformComponent
  │     ├── MeshRendererComponent
  │     ├── PhysicsComponent
  │     └── ScriptComponent
  └── vge::BlockRegistry  - Block type definitions
```

### Key Namespaces

- `vge` - Main engine namespace
- `vge::math` - Math utilities (Vec3, Mat4, Noise)

---

## Core Systems

### 1. Voxel World System

#### Block Registry

Register block types with properties:

```cpp
#include "voxel/block_registry.h"

vge::BlockRegistry registry;

// Register a simple block
registry.RegisterBlock("stone", {
    .solid = true,
    .transparent = false,
    .hardness = 3.0f,
    .texture = "stone.png"
});

// Register a transparent block
registry.RegisterBlock("water", {
    .solid = false,
    .transparent = true,
    .hardness = 0.0f,
    .texture = "water.png"
});
```

#### World Management

```cpp
#include "voxel/world.h"

vge::World world;

// Set block
world.SetBlock(10, 5, 10, registry.GetBlockID("stone"));

// Get block
uint16_t blockID = world.GetBlock(10, 5, 10);

// Update chunk meshes (call after modifying blocks)
world.UpdateChunks();

// Raycast to find what player is looking at
vge::RayHit hit = world.Raycast(playerPos, lookDir, 5.0f);
if (hit.hit) {
    // Place block adjacent to hit face
    vge::Vec3 placePos = hit.position + hit.normal;
    world.SetBlock(placePos.x, placePos.y, placePos.z, blockID);
}
```

#### Chunk System

```cpp
// Chunks are 16x16x16 blocks
// Automatically loaded/unloaded based on view distance
world.SetViewDistance(8);  // 8 chunks in each direction

// Force chunk update after batch changes
world.UpdateChunks();
```

### 2. Entity Component System

#### Creating Entities

```cpp
#include "entity/entity.h"
#include "entity/components.h"

// Create entity
vge::Entity* player = world.CreateEntity("Player");

// Add transform (position, rotation, scale)
auto* transform = player->AddComponent<vge::TransformComponent>();
transform->SetPosition(vge::Vec3(0, 10, 0));

// Add mesh renderer
auto* renderer = player->AddComponent<vge::MeshRendererComponent>();
renderer->SetMesh("player_model.obj");
renderer->SetMaterial("player_mat");

// Add physics
auto* physics = player->AddComponent<vge::PhysicsComponent>();
physics->SetMass(70.0f);
physics->SetCollider(vge::Collider::Box({0.6f, 1.8f, 0.6f}));

// Add script (Lua)
auto* script = player->AddComponent<vge::ScriptComponent>();
script->LoadScript("scripts/player_controller.lua");
```

#### Component Types

| Component | Purpose | Key Methods |
|-----------|---------|-------------|
| `TransformComponent` | Position, rotation, scale | `SetPosition()`, `SetRotation()`, `GetForward()` |
| `MeshRendererComponent` | Visual mesh | `SetMesh()`, `SetMaterial()`, `SetVisible()` |
| `PhysicsComponent` | Physics body | `SetMass()`, `SetVelocity()`, `ApplyForce()` |
| `ScriptComponent` | Lua scripting | `LoadScript()`, `CallFunction()` |
| `CameraComponent` | Camera view | `SetFOV()`, `SetNearFar()`, `GetViewMatrix()` |
| `LightComponent` | Light source | `SetType()`, `SetColor()`, `SetIntensity()` |
| `AudioSourceComponent` | 3D audio | `Play()`, `Stop()`, `SetLooping()` |
| `ParticleEmitterComponent` | Particles | `Emit()`, `SetRate()`, `SetColor()` |
| `AnimatorComponent` | Animation | `Play()`, `SetSpeed()`, `GetCurrentTime()` |
| `NavMeshAgentComponent` | Pathfinding | `SetDestination()`, `SetSpeed()`, `IsMoving()` |

### 3. Rendering System

#### Mesh Builder

```cpp
#include "rendering/mesh.h"

vge::MeshBuilder builder;

// Add vertices
builder.AddVertex({0, 0, 0}, {0, 0, 1}, {0, 0});  // pos, normal, uv
builder.AddVertex({1, 0, 0}, {0, 0, 1}, {1, 0});
builder.AddVertex({1, 1, 0}, {0, 0, 1}, {1, 1});
builder.AddVertex({0, 1, 0}, {0, 0, 1}, {0, 1});

// Add triangle indices
builder.AddTriangle(0, 1, 2);
builder.AddTriangle(0, 2, 3);

// Build mesh
vge::Mesh* mesh = builder.Build();
```

#### Materials

```cpp
#include "rendering/material.h"

vge::Material material;
material.SetShader("standard");
material.SetTexture("diffuse", "texture.png");
material.SetColor("tint", vge::Vec3(1, 1, 1));
material.SetFloat("roughness", 0.5f);
```

#### PBR Materials

```cpp
#include "rendering/pbr_material.h"

vge::PBRMaterial pbr;
pbr.SetAlbedo("albedo.png");
pbr.SetNormal("normal.png");
pbr.SetRoughness("roughness.png");
pbr.SetMetallic("metallic.png");
pbr.SetAO("ao.png");
```

### 4. Physics System

#### Physics World

```cpp
#include "physics/physics_world.h"

vge::PhysicsWorld physics;
physics.SetGravity(vge::Vec3(0, -9.81f, 0));

// Create rigidbody
vge::Rigidbody* body = physics.CreateBody({
    .mass = 1.0f,
    .position = vge::Vec3(0, 10, 0),
    .collider = vge::Collider::Box({1, 1, 1})
});

// Apply forces
body->ApplyForce(vge::Vec3(0, 100, 0));  // Jump
body->ApplyImpulse(vge::Vec3(10, 0, 0)); // Push

// Update physics (call every frame)
physics.Update(deltaTime);
```

#### Collision Detection

```cpp
// Check collision between two bodies
if (physics.CheckCollision(bodyA, bodyB)) {
    // Handle collision
}

// Raycast
vge::RayHit hit = physics.Raycast(origin, direction, maxDistance);
if (hit.hit) {
    std::cout << "Hit at: " << hit.position << std::endl;
}
```

### 5. Audio System

#### 3D Audio

```cpp
#include "audio/audio_system.h"

vge::AudioSystem audio;
audio.Initialize();

// Load sounds
audio.LoadSound("jump", "sounds/jump.wav");
audio.LoadSound("music", "music/ambient.ogg", true); // Streaming

// Play 3D positioned sound
audio.Play3D("jump", playerPos, {1.0f, 0.5f, 2.0f});

// Background music
audio.Play("music");
audio.SetVolume("music", 0.5f);

// Update listener position (player camera)
audio.SetListenerPosition(cameraPos, cameraForward, cameraUp);
```

### 6. Animation System

#### Skeletal Animation

```cpp
#include "animation/animation.h"

// Create skeleton
vge::Skeleton skeleton;
int root = skeleton.AddJoint("Root", -1);
int spine = skeleton.AddJoint("Spine", root);
int armR = skeleton.AddJoint("ArmR", spine);

// Create animation clip
vge::AnimationClip clip("wave");
clip.SetDuration(2.0f);

// Add keyframes
clip.AddPositionKey(0.0f, armR, vge::Vec3(0, 0, 0));
clip.AddRotationKey(0.5f, armR, vge::Vec3(0, 0, 45));
clip.AddRotationKey(1.5f, armR, vge::Vec3(0, 0, -45));
clip.AddPositionKey(2.0f, armR, vge::Vec3(0, 0, 0));

// Play animation
vge::Animator animator(&skeleton);
animator.Play(&clip);
animator.SetLooping(true);

// Update each frame
animator.Update(deltaTime);

// Get bone matrices for skinning
const std::vector<vge::Mat4>& matrices = skeleton.GetSkinningMatrices();
```

### 7. AI System

#### Pathfinding

```cpp
#include "ai/ai_system.h"

// Create navigation mesh
vge::NavigationMesh navMesh;
navMesh.BuildFromWorld(&world, vge::Vec3(0, 0, 0), 50.0f, 1.0f);

// Find path
std::vector<vge::Vec3> path = navMesh.FindPath(startPos, goalPos);

// Create agent
vge::Agent agent;
agent.SetPosition(startPos);
agent.SetSpeed(5.0f);
agent.SetPath(path);

// Update each frame
agent.Update(deltaTime);
```

#### Steering Behaviors

```cpp
vge::SteeringBehavior steering;

// Seek target
vge::Vec3 seekForce = steering.Seek(agentPos, targetPos, maxSpeed);

// Flee from danger
vge::Vec3 fleeForce = steering.Flee(agentPos, dangerPos, maxSpeed, 10.0f);

// Wander randomly
vge::Vec3 wanderForce = steering.Wander(agentPos, agentVel, 5.0f, 2.0f, 0.5f);

// Avoid obstacles
vge::Vec3 avoidForce = steering.Avoid(agentPos, agentVel, obstacles, maxSpeed, 3.0f);

// Combine forces
vge::Vec3 totalForce = seekForce * 1.0f + avoidForce * 2.0f + wanderForce * 0.3f;
```

### 8. UI System

#### Creating UI

```cpp
#include "ui/ui_system.h"

vge::UIManager ui;

// Create panel
auto* panel = ui.CreatePanel("MainMenu");
panel->SetPosition(vge::Vec2(100, 100));
panel->SetSize(vge::Vec2(300, 400));
panel->SetAutoLayout(true);

// Add title
auto* title = ui.CreateLabel("Title", "My Game");
title->SetFontSize(24);
panel->AddChild(std::unique_ptr<vge::UIElement>(title));

// Add button
auto* playBtn = ui.CreateButton("PlayBtn", "Play Game");
playBtn->SetOnClick([]() {
    std::cout << "Starting game!" << std::endl;
});
panel->AddChild(std::unique_ptr<vge::UIElement>(playBtn));

// Add slider
auto* volumeSlider = ui.CreateSlider("Volume", 0, 1);
volumeSlider->SetOnValueChanged([](float value) {
    audio.SetMasterVolume(value);
});
panel->AddChild(std::unique_ptr<vge::UIElement>(volumeSlider));

// Handle input
ui.HandleMouseMove(mousePos);
ui.HandleMouseClick(mousePos, true);  // Press
ui.HandleMouseClick(mousePos, false); // Release

// Update and render
ui.Update(deltaTime);
ui.Render();
```

#### UI Element Types

| Element | Description | Key Features |
|---------|-------------|--------------|
| `UIButton` | Clickable button | Text, icon, toggle mode, callbacks |
| `UILabel` | Text display | Auto-size, word wrap, font size |
| `UISlider` | Value slider | Min/max, step, vertical mode |
| `UITextInput` | Text input | Placeholder, password, multiline |
| `UIPanel` | Container | Scrollable, auto-layout, padding |
| `UIImage` | Image display | Texture, UV rect, preserve aspect |
| `UICheckbox` | Toggle checkbox | Checked state, label |
| `UIDropdown` | Dropdown menu | Options, selection callback |

---

## Input System

### Input Mapping

```cpp
#include "input/input_manager.h"

vge::InputManager input;

// Map keys to actions
input.MapKey("move_forward", GLFW_KEY_W);
input.MapKey("move_backward", GLFW_KEY_S);
input.MapKey("move_left", GLFW_KEY_A);
input.MapKey("move_right", GLFW_KEY_D);
input.MapKey("jump", GLFW_KEY_SPACE);
input.MapKey("place_block", GLFW_KEY_E);
input.MapKey("break_block", GLFW_KEY_Q);

// Map mouse buttons
input.MapMouseButton("attack", GLFW_MOUSE_BUTTON_LEFT);
input.MapMouseButton("interact", GLFW_MOUSE_BUTTON_RIGHT);

// In game loop
input.Update();

if (input.IsActionPressed("jump")) {
    player->Jump();
}

float moveX = input.GetAxis("move_right", "move_left");
float moveZ = input.GetAxis("move_forward", "move_backward");

// Mouse look
float lookX = input.GetMouseDeltaX();
float lookY = input.GetMouseDeltaY();
camera->Rotate(lookX * sensitivity, lookY * sensitivity);
```

---

## Scripting (Lua)

### Loading Scripts

```cpp
#include "scripting/lua_engine.h"

vge::LuaEngine lua;
lua.Initialize();

// Load and run script
lua.LoadFile("scripts/game_logic.lua");

// Call function
lua.CallFunction("onGameStart");

// Get/set variables
lua.SetGlobal("playerHealth", 100);
int health = lua.GetGlobal<int>("playerHealth");
```

### Lua API

Scripts have access to engine functions:

```lua
-- game_logic.lua

function onGameStart()
    print("Game started!")
    
    -- Access world
    world:SetBlock(0, 0, 0, "stone")
    
    -- Access player
    local pos = player:GetPosition()
    player:SetPosition(pos.x, pos.y + 10, pos.z)
    
    -- Register callback
    RegisterEvent("onUpdate", onUpdate)
end

function onUpdate(deltaTime)
    -- Called every frame
    local vel = player:GetVelocity()
    if vel.y < -0.1 then
        -- Falling
    end
end
```

---

## World Generation

### Biomes

```cpp
#include "voxel/biome.h"

vge::BiomeManager biomes;

// Register biomes
vge::Biome plains{
    .name = "Plains",
    .surfaceBlock = "grass",
    .subsurfaceBlock = "dirt",
    .treeDensity = 0.05f,
    .temperature = 0.5f,
    .humidity = 0.5f
};
biomes.Register(plains);

vge::Biome desert{
    .name = "Desert",
    .surfaceBlock = "sand",
    .subsurfaceBlock = "sandstone",
    .treeDensity = 0.0f,
    .temperature = 0.9f,
    .humidity = 0.1f
};
biomes.Register(desert);
```

### Terrain Generation

```cpp
#include "voxel/world_generator.h"

vge::WorldGenerator generator;
generator.SetSeed(12345);

// Configure noise layers
generator.AddHeightNoise({
    .scale = 0.01f,
    .amplitude = 20.0f,
    .octaves = 4
});

generator.AddCaveNoise({
    .scale = 0.05f,
    .threshold = 0.6f
});

// Generate chunk
vge::Chunk* chunk = generator.GenerateChunk(0, 0, 0, biomes);
```

---

## Save/Load System

### Saving Worlds

```cpp
#include "core/save_manager.h"

vge::SaveManager save;

// Save world
save.SaveWorld(&world, "saves/world1");

// Load world
save.LoadWorld(&world, "saves/world1");

// Auto-save every 5 minutes
save.SetAutoSaveInterval(300.0f);
```

### Save Format

Worlds are saved in a compressed binary format:
- `world.dat` - World metadata (seed, time, player pos)
- `chunks/` - Individual chunk files (compressed)
- `entities/` - Entity data
- `player.dat` - Player inventory, health, position

---

## Multiplayer

### Network Architecture

The engine supports client-server multiplayer:

```cpp
#include "network/network_manager.h"

// Server
vge::NetworkManager server;
server.StartServer(7777, 32); // Port, max players

// Client
vge::NetworkManager client;
client.Connect("192.168.1.100", 7777);

// Send player position
client.SendPlayerPosition(pos);

// Receive world updates
client.OnWorldUpdate([](const vge::WorldUpdate& update) {
    world.ApplyUpdate(update);
});
```

---

## Asset Pipeline

### Loading Assets

```cpp
#include "asset/asset_manager.h"

vge::AssetManager assets;

// Load textures
assets.LoadTexture("stone", "textures/stone.png");
assets.LoadTexture("dirt", "textures/dirt.png");

// Load models
assets.LoadModel("player", "models/player.obj");

// Load sounds
assets.LoadSound("jump", "sounds/jump.wav");

// Get cached asset
vge::Texture* stoneTex = assets.GetTexture("stone");
```

### Hot Reloading

Assets can be reloaded at runtime for development:

```cpp
// Enable hot reloading (checks file modification times)
assets.EnableHotReloading(true);

// Check for changes (call periodically)
assets.CheckForChanges();
```

---

## Performance Optimization

### Chunk Culling

```cpp
// Enable frustum culling
renderer.EnableFrustumCulling(true);

// Set LOD distances
world.SetLODDistance(0, 32);   // Full detail within 32 blocks
world.SetLODDistance(1, 64);   // Half detail within 64 blocks
world.SetLODDistance(2, 128);  // Quarter detail within 128 blocks
```

### Instanced Rendering

```cpp
// Render many similar objects efficiently
vge::InstancedMesh trees;
trees.SetMesh("tree_model.obj");
trees.SetPositions(treePositions); // Array of positions
trees.Render();
```

### Occlusion Culling

```cpp
// Don't render blocks hidden by other blocks
world.EnableOcclusionCulling(true);
```

---

## Debugging Tools

### Debug Renderer

```cpp
#include "debug/debug_renderer.h"

vge::DebugRenderer debug;

// Draw wireframe boxes
debug.DrawBox(playerPos, {0.6f, 1.8f, 0.6f}, vge::Vec3(0, 1, 0));

// Draw lines
debug.DrawLine(start, end, vge::Vec3(1, 0, 0));

// Draw text
debug.DrawText(vge::Vec3(0, 2, 0), "Player", 16);

// Render all debug primitives
debug.Render(camera);
```

### Profiling

```cpp
#include "debug/profiler.h"

// Profile a scope
{
    vge::ProfileScope("Chunk Generation");
    GenerateChunk();
}

// Get profiling stats
auto stats = vge::Profiler::GetStats();
for (const auto& [name, time] : stats) {
    std::cout << name << ": " << time << "ms" << std::endl;
}
```

---

## Example: Complete Game Setup

```cpp
#include <engine.h>

int main() {
    // Initialize engine
    vge::Engine engine;
    engine.Initialize(1280, 720, "My Voxel Game");
    
    // Create world
    vge::World world;
    world.SetViewDistance(12);
    
    // Register blocks
    vge::BlockRegistry& registry = world.GetRegistry();
    registry.RegisterBlock("stone", {.solid = true, .texture = "stone.png"});
    registry.RegisterBlock("dirt", {.solid = true, .texture = "dirt.png"});
    registry.RegisterBlock("grass", {.solid = true, .texture = "grass.png"});
    
    // Generate terrain
    vge::WorldGenerator generator;
    generator.SetSeed(12345);
    world.SetGenerator(&generator);
    
    // Create player
    vge::Entity* player = world.CreateEntity("Player");
    auto* camera = player->AddComponent<vge::CameraComponent>();
    auto* physics = player->AddComponent<vge::PhysicsComponent>();
    physics->SetCollider(vge::Collider::Box({0.6f, 1.8f, 0.6f}));
    
    // Setup input
    vge::InputManager input;
    input.MapKey("forward", GLFW_KEY_W);
    input.MapKey("backward", GLFW_KEY_S);
    
    // Game loop
    while (engine.IsRunning()) {
        float deltaTime = engine.GetDeltaTime();
        
        // Update input
        input.Update();
        
        // Update physics
        world.UpdatePhysics(deltaTime);
        
        // Update world (chunk loading)
        world.Update(player->GetPosition());
        
        // Render
        engine.BeginFrame();
        world.Render(camera);
        engine.EndFrame();
    }
    
    return 0;
}
```

---

## API Reference

### Math

| Class | Description |
|-------|-------------|
| `Vec2` | 2D vector (x, y) |
| `Vec3` | 3D vector (x, y, z) |
| `Mat4` | 4x4 matrix |
| `Noise` | Perlin/Simplex noise |

### Core

| Class | Description |
|-------|-------------|
| `World` | Voxel world manager |
| `Chunk` | 16x16x16 block storage |
| `BlockRegistry` | Block type definitions |
| `Entity` | Game object |
| `Component` | Base component class |
| `SaveManager` | Save/load system |

### Rendering

| Class | Description |
|-------|-------------|
| `Mesh` | Vertex mesh |
| `MeshBuilder` | Mesh construction |
| `Material` | Shader material |
| `PBRMaterial` | PBR material |
| `Texture` | 2D texture |
| `Shader` | GPU shader |
| `ShadowSystem` | Shadow mapping |

### Physics

| Class | Description |
|-------|-------------|
| `PhysicsWorld` | Physics simulation |
| `Rigidbody` | Physics body |
| `Collider` | Collision shape |
| `RayHit` | Raycast result |

### Audio

| Class | Description |
|-------|-------------|
| `AudioSystem` | Audio manager |
| `Sound` | Sound effect |
| `Music` | Streaming music |

### Animation

| Class | Description |
|-------|-------------|
| `Skeleton` | Joint hierarchy |
| `AnimationClip` | Animation data |
| `Animator` | Animation playback |

### AI

| Class | Description |
|-------|-------------|
| `NavigationMesh` | Pathfinding mesh |
| `Agent` | AI agent |
| `SteeringBehavior` | Movement behaviors |

### UI

| Class | Description |
|-------|-------------|
| `UIManager` | UI manager |
| `UIElement` | Base UI element |
| `UIButton` | Button |
| `UILabel` | Text label |
| `UISlider` | Slider |
| `UITextInput` | Text input |
| `UIPanel` | Container panel |
| `UIImage` | Image |
| `UICheckbox` | Checkbox |
| `UIDropdown` | Dropdown |

---

## File Structure

```
aether-game-engine/
├── src/
│   ├── core/           # Engine core (world, entity, save)
│   ├── voxel/          # Voxel systems (chunk, block, generation)
│   ├── rendering/      # Graphics (mesh, material, shader, shadow)
│   ├── physics/        # Physics (rigidbody, collision, raycast)
│   ├── audio/          # Audio system
│   ├── animation/      # Skeletal animation
│   ├── ai/             # AI and pathfinding
│   ├── ui/             # User interface
│   ├── input/          # Input handling
│   ├── scripting/      # Lua scripting
│   ├── network/        # Multiplayer networking
│   ├── asset/          # Asset management
│   ├── math/           # Math utilities
│   └── debug/          # Debug tools
├── include/            # Public headers
├── shaders/            # GLSL shaders
├── textures/           # Default textures
├── models/             # Default models
├── sounds/             # Default sounds
├── scripts/            # Example Lua scripts
├── docs/               # Documentation
├── tests/              # Unit tests
├── build/              # Build output
├── CMakeLists.txt      # Build configuration
└── README.md           # Project readme
```

---

## Contributing

1. Fork the repository
2. Create a feature branch
3. Follow coding standards (see STYLE.md)
4. Write tests for new features
5. Submit a pull request

---

## License

MIT License - See LICENSE file for details

---

## Support

- **Issues:** http://192.168.1.189:3100/aether/aether-game-engine/issues
- **Wiki:** http://192.168.1.189:3100/aether/aether-game-engine/wiki
- **Discord:** [Aether Discord Server]

---

*Last Updated: 2026-05-10*
