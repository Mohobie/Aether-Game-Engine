# Aether Game Engine — Developer Quick Start Guide

This guide gets you from zero to a running voxel game as fast as possible. Follow each section in order.

---

## 1. Prerequisites

### Required
| Tool | Minimum Version | How to Check |
|------|----------------|--------------|
| C++ Compiler | GCC 8+ / Clang 7+ / MSVC 2019+ | `g++ --version` |
| CMake | 3.14+ | `cmake --version` |
| OpenGL | 3.3+ | `glxinfo | grep "OpenGL version"` (Linux) |
| Git | Any | `git --version` |
| Dear ImGui | Bundled in `third_party/imgui` | Verify sources are present |

> **Note:** Dear ImGui is expected to be present under `third_party/imgui` in this repository checkout.

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential cmake git libgl1-mesa-dev libglu1-mesa-dev
```

### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake and Git via Homebrew
brew install cmake git
```

### Windows
1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with "Desktop development with C++" workload
2. Install [CMake](https://cmake.org/download/)
3. Install [Git](https://git-scm.com/download/win)

---

## 2. Clone & Build

### Clone the Repository
```bash
git clone https://github.com/Mohobie/Aether-Game-Engine.git
cd aether-game-engine

```

### Build (Linux/macOS)
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)  # Use all CPU cores
```

### Build (Windows)
```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Verify Build
Current audited Windows build truth for this repo on 2026-05-15:

```cmd
C:\Program Files\CMake\bin\cmake.exe -S . -B build-vs -G "Visual Studio 17 2022"
C:\Program Files\CMake\bin\cmake.exe --build build-vs --config Debug --target voxel_engine_lib save_test world_test voxel_test
build-vs\Debug\save_test.exe
build-vs\Debug\world_test.exe
build-vs\Debug\voxel_test.exe
```

This shell currently configures with `OpenGL=TRUE`, `GLFW=FALSE`, and `Lua=FALSE`. The verified path on this machine is therefore the core/headless library plus the three test executables above. `voxel_engine` is only expected to build once the GLFW-backed windowed runtime dependency path is restored on the machine.

```bash
# Generic validation once your local dependency set is complete
./voxel_test

# Run the engine when GLFW-backed windowed dependencies are available
./voxel_engine
```

### Build Your Own Game

The engine supports building custom games via a `my_game.cpp` file:

```bash
# Create my_game.cpp in the project root
# (see examples below)

# Build your game
cmake ..
make -j4 my_game

# Run it
./my_game
```

The `my_game` target is automatically created when `my_game.cpp` exists.

---

## 3. Project Structure Overview

```
aether-game-engine/
├── src/                    # All source code
│   ├── core/               # Logger, config, time, save system
│   ├── math/               # Vec2, Vec3, Mat4, noise
│   ├── voxel/              # Blocks, chunks, world, generation
│   ├── rendering/          # Camera, shaders, meshes, lighting
│   ├── entity/             # ECS — entities, components, systems
│   ├── physics/            # Rigid bodies, collisions, constraints
│   ├── ai/                 # Behavior trees, pathfinding, archetypes
│   ├── audio/              # 3D audio, sound packs, music
│   ├── ui/                 # ImGui, buttons, inventory UI, console
│   ├── input/              # Legacy input APIs retained in-tree
│   ├── platform/           # Window, file system, threading
│   ├── game/               # Game modes, player data, quests
│   ├── network/            # Multiplayer, replication, server
│   ├── scripting/          # Lua integration, mod system
│   ├── resource/           # Asset loading, config manager
│   ├── editor/             # In-game editor, viewports, asset browser
│   └── debug/              # Debug visualization, profiler
├── include/                # Third-party headers (json, etc.)
├── docs/                   # Documentation
│   ├── rag/                # AI-optimized docs (19 documents)
│   └── API-INDEX.md        # API reference index
├── assets/                 # Default textures, sounds, models
├── tests/                  # Unit tests
└── CMakeLists.txt         # Build configuration
```

**Key rule:** Everything is in the `vge::` namespace. No `aether::` anymore.

### Canonical module map for the current audited build

- Rendering: `src/rendering/*`; `src/render/*` is legacy.
- Input: `src/platform/input_manager.*`; `src/input/*` is legacy and `src/platform/input.*` is compatibility-only outside the active library target.
- Application facade: `src/game/application.*`; `src/core/application.*` is legacy.
- Entity layer: `src/entity/entity.*` and `src/entity/components.*`; `src/core/entity.*` is legacy.
- Save stack: `src/core/save_system.*` for direct world/chunk IO, `src/voxel/world_serializer.*` for richer world snapshots, and `src/core/save_game.*` as the active higher-level wrapper; `src/game/save_system.*` and `src/game/serializer.*` are legacy.

---

## 4. Your First Voxel World

Create `my_game.cpp` in the project root:

```cpp
#include "voxel/world.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "platform/window.h"
#include "platform/input_manager.h"
#include "core/player_controller.h"
#include "game/block_interaction.h"
#include "debug/debug_renderer.h"
#include "audio/audio_engine.h"
#include "editor/in_game_editor.h"
#include <iostream>

int main() {
    // 1. Create window
    vge::Window window;
    if (!window.Initialize(1280, 720, "My Voxel Game")) {
        std::cerr << "Failed to initialize window" << std::endl;
        return 1;
    }
    
    // 2. Create renderer (OpenGL 3.3+ with VAOs/VBOs)
    vge::Renderer renderer;
    if (!renderer.Initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return 1;
    }
    renderer.SetViewport(0, 0, 1280, 720);
    
    // 3. Create camera
    vge::Camera camera;
    camera.SetPosition(vge::Vec3(0, 50, 0));
    camera.SetRotation(-90, -30, 0);
    
    // 4. Create world and generate terrain
    vge::World world;
    world.SetSeed(12345);
    
    // Generate some chunks
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            world.GetOrCreateChunk(x, 0, z);
        }
    }
    
    // Place some blocks
    world.SetBlock(0, 0, 0, "stone");
    world.SetBlock(1, 0, 0, "stone");
    world.SetBlock(0, 1, 0, "stone");
    world.SetBlock(0, 0, 1, "dirt");
    world.SetBlock(1, 1, 0, "dirt");
    world.SetBlock(0, 2, 0, "grass");
    world.SetBlock(2, 0, 0, "wood");
    world.SetBlock(2, 1, 0, "wood");
    world.SetBlock(2, 2, 0, "leaves");
    world.SetBlock(3, 0, 0, "sand");
    world.SetBlock(4, 0, 0, "water");
    world.SetBlock(5, 0, 0, "bedrock");
    
    // Create a platform
    for (int x = -3; x <= 3; x++) {
        for (int z = -3; z <= 3; z++) {
            world.SetBlock(x, -1, z, "stone");
        }
    }
    
    // 5. Create input
    vge::Input input;
    
    // 6. Create player controller
    vge::PlayerController player;
    player.SetPosition(vge::Vec3(0, 50, 0));
    
    // 7. Create block interaction
    vge::BlockInteraction blockInteraction;
    blockInteraction.Initialize(world);
    
    // 8. Create audio
    vge::AudioEngine audio;
    audio.Initialize();
    
    // 9. Create editor
    vge::InGameEditor editor(&world, &camera, &input, &renderer);
    editor.Initialize();
    
    // 10. Create debug renderer
    vge::DebugRenderer& debug = vge::GetDebugRenderer();
    debug.Initialize();
    
    // 11. Game loop
    bool running = true;
    float deltaTime = 1.0f / 60.0f;
    
    while (running) {
        // Handle window events
        window.PollEvents();
        if (window.ShouldClose()) {
            running = false;
        }
        
        // Update input
        input.Update();
        
        // Toggle editor with Escape
        if (input.IsKeyJustPressed(vge::KeyCode::Escape)) {
            editor.Toggle();
        }
        
        if (editor.IsActive()) {
            // Editor mode
            editor.Update(deltaTime, input);
        } else {
            // Game mode - player movement
            player.Update(deltaTime, input, world);
            
            // Update camera to follow player
            camera.SetPosition(player.GetPosition() + vge::Vec3(0, 1.8f, 0));
            camera.SetRotation(player.GetYaw(), player.GetPitch(), 0);
            
            // Block interaction
            blockInteraction.Update(camera, input, world);
        }
        
        // Render
        renderer.SetClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        renderer.BeginFrame();
        
        renderer.RenderWorld(world, camera);
        
        // Render editor visuals if active
        if (editor.IsActive()) {
            editor.Render();
        }
        
        // Render debug visualization
        debug.Render(camera);
        debug.Update(deltaTime);
        debug.Clear();
        
        renderer.EndFrame();
        window.SwapBuffers();
    }
    
    // Cleanup
    editor.Shutdown();
    audio.Shutdown();
    renderer.Shutdown();
    window.Shutdown();
    
    return 0;
}
```

### Build Your Game

The `my_game` target is automatically created when `my_game.cpp` exists in the project root:

```bash
cd build
cmake ..
make -j4 my_game
./my_game
```

---

## 5. Adding an Entity

### Define an Archetype (Code)

```cpp
#include "ai/entity_ai.h"

// Create a zombie archetype
vge::EntityArchetype zombie;
zombie.id = "zombie";
zombie.displayName = "Zombie";
zombie.health = 100;
zombie.speed = 3.5f;
zombie.damage = 15;
zombie.behavior = vge::AIBehaviorType::Aggressive;
zombie.movementType = vge::MovementType::Walking;
zombie.canSwim = false;
zombie.canFly = false;
zombie.damagedBySunlight = true;   // Burns in sun
zombie.healedByDarkness = true;     // Heals at night
zombie.spawnTime = vge::SpawnTime::Night;

// Register it
vge::EntityArchetypeRegistry::GetInstance()->RegisterArchetype(zombie);
```

### Define an Archetype (JSON)

Create `assets/archetypes/zombie.json`:
```json
{
  "id": "zombie",
  "displayName": "Zombie",
  "health": 100,
  "speed": 3.5,
  "damage": 15,
  "behavior": "aggressive",
  "movementType": "walking",
  "canSwim": false,
  "canFly": false,
  "damagedBySunlight": true,
  "healedByDarkness": true,
  "spawnTime": "night",
  "customFloats": {
    "attack_range": 2.0,
    "detection_range": 20.0
  },
  "customStrings": {
    "death_sound": "zombie_die"
  }
}
```

Load it:
```cpp
vge::EntityArchetypeRegistry::GetInstance()->LoadFromFile(
    "assets/archetypes/zombie.json"
);
```

### Spawn the Entity

```cpp
#include "editor/entity_spawner.h"

vge::EntitySpawner spawner;
spawner.Initialize(&world, &entityManager);

// Spawn at specific position
uint32_t zombieId = spawner.SpawnEntity("zombie", vge::Vec3(10, 20, 10));

// Spawn at camera position
uint32_t zombieId2 = spawner.SpawnAtCamera("zombie", camera);

// Spawn at cursor (requires mouse ray)
vge::Vec3 position, normal;
if (blockPicker.PickBlock(camera.GetPosition(), camera.GetForward(), position, normal)) {
    uint32_t zombieId3 = spawner.SpawnAtCursor("zombie", camera, position);
}
```

---

## 6. Player Controller

```cpp
#include "game/player_controller.h"

vge::PlayerController player;
player.SetPosition(vge::Vec3(0, 50, 0));

// In game loop:
void UpdatePlayer(float deltaTime, vge::Input& input) {
    // Movement
    vge::Vec3 moveDir(0, 0, 0);
    if (input.IsKeyPressed(vge::KeyCode::W)) moveDir.z += 1;
    if (input.IsKeyPressed(vge::KeyCode::S)) moveDir.z -= 1;
    if (input.IsKeyPressed(vge::KeyCode::A)) moveDir.x -= 1;
    if (input.IsKeyPressed(vge::KeyCode::D)) moveDir.x += 1;
    
    player.Move(moveDir, deltaTime);
    
    // Mouse look
    float dx, dy;
    input.GetMouseDelta(dx, dy);
    player.Rotate(dx * 0.1f, dy * 0.1f);
    
    // Jump
    if (input.IsKeyJustPressed(vge::KeyCode::Space)) {
        player.Jump();
    }
    
    // Update camera
    camera.SetPosition(player.GetPosition() + vge::Vec3(0, 1.8f, 0));  // Eye height
    camera.SetRotation(player.GetYaw(), player.GetPitch(), 0);
}
```

---

## 7. Block Interaction

```cpp
#include "game/block_interaction.h"

vge::BlockInteraction blockInteraction(&world);

// In game loop:
void HandleBlockInteraction(vge::Input& input, vge::Camera& camera) {
    // Raycast from camera
    vge::Vec3 position, normal;
    if (blockInteraction.RaycastFromCamera(camera, 5.0f, position, normal)) {
        
        // Left click — break block
        if (input.IsMouseButtonJustPressed(vge::MouseButton::Left)) {
            blockInteraction.BreakBlock(position);
        }
        
        // Right click — place block
        if (input.IsMouseButtonJustPressed(vge::MouseButton::Right)) {
            vge::Vec3 placePos = position + normal;
            blockInteraction.PlaceBlock(placePos, vge::BLOCK_STONE);
        }
        
        // Middle click — pick block
        if (input.IsMouseButtonJustPressed(vge::MouseButton::Middle)) {
            vge::BlockTypeID picked = world.GetBlock(
                (int)position.x, (int)position.y, (int)position.z
            );
            // Set as current block
        }
    }
}
```

---

## 8. Inventory & Crafting

### Create Items

```cpp
#include "core/item_system.h"

// Define items
vge::Item stoneBlock;
stoneBlock.id = "stone";
stoneBlock.name = "Stone Block";
stoneBlock.category = vge::ItemCategory::Block;
stoneBlock.maxStackSize = 64;
stoneBlock.blockType = vge::BLOCK_STONE;

vge::Item woodenPickaxe;
woodenPickaxe.id = "wooden_pickaxe";
woodenPickaxe.name = "Wooden Pickaxe";
woodenPickaxe.category = vge::ItemCategory::Tool;
woodenPickaxe.maxStackSize = 1;
woodenPickaxe.toolType = vge::ToolType::Pickaxe;
woodenPickaxe.toolLevel = 1;
woodenPickaxe.durability = 60;

// Register
vge::ItemSystem::GetInstance()->RegisterItem(stoneBlock);
vge::ItemSystem::GetInstance()->RegisterItem(woodenPickaxe);
```

### Create Inventory

```cpp
#include "core/inventory.h"

vge::Inventory playerInventory(36);  // 36 slots

// Add items
playerInventory.AddItem("stone", 32);
playerInventory.AddItem("wooden_pickaxe", 1);

// Check items
if (playerInventory.HasItem("stone", 10)) {
    // Craft something
}

// Remove items
playerInventory.RemoveItem("stone", 10);
```

### Define Recipes

```cpp
#include "core/crafting.h"

vge::CraftingSystem crafting;

// Shaped recipe (pickaxe)
vge::CraftingRecipe pickaxeRecipe;
pickaxeRecipe.result = "wooden_pickaxe";
pickaxeRecipe.resultCount = 1;
pickaxeRecipe.shape = {
    {"wood", "wood", "wood"},
    {"",     "stick", ""    },
    {"",     "stick", ""    }
};
pickaxeRecipe.ingredients["wood"] = "planks";
pickaxeRecipe.ingredients["stick"] = "stick";

crafting.RegisterRecipe(pickaxeRecipe);

// Check if can craft
if (crafting.CanCraft(pickaxeRecipe, playerInventory)) {
    crafting.Craft(pickaxeRecipe, playerInventory);
}
```

---

## 9. Audio

```cpp
#include "audio/audio_engine.h"

vge::AudioEngine audio;
audio.Initialize();

// Load sound assets directly
int footstep = audio.LoadSound("assets/sounds/step.wav");
int dig = audio.LoadSound("assets/sounds/dig.wav");

// Play simple 2D events
audio.Play2D(footstep);
audio.Play2D(dig);
```

---

## 10. Save & Load

```cpp
#include "core/save_system.h"

// Save world
vge::SaveSystem::SaveWorld(world, "saves/world1.bin");

// Load world
vge::SaveSystem::LoadWorld(world, "saves/world1.bin");
```

For the current audited build, this is the direct world/chunk API. Higher-level named-save flow lives in `core/save_game.h`, which wraps `voxel/world_serializer.h`.

---

## 11. Using the In-Game Editor

### Toggle Editor
```cpp
#include "editor/in_game_editor.h"

vge::InGameEditor editor(&world, &camera, &input, &renderer);
editor.Initialize();

// In game loop:
if (input.IsKeyJustPressed(vge::KeyCode::F1)) {
    editor.Toggle();
}

if (editor.IsActive()) {
    editor.Update(deltaTime, input);
    editor.Render();
}
```

### Editor Features
| Key | Action |
|-----|--------|
| F1 | Toggle editor |
| Enter | Select block / Place block |
| Escape | Remove block |
| Q | Undo |
| E | Redo |
| Shift | Toggle gizmos |
| 1-3 | Select gizmo type (translate/rotate/scale) |

### Multi-Viewport
```cpp
#include "editor/scene_viewport.h"

vge::ViewportManager viewports;
viewports.Initialize(1920, 1080);
viewports.SetLayoutQuad();  // 4 views

// In render loop:
viewports.Render();
```

---

## 12. Debug Visualization

```cpp
#include "debug/debug_renderer.h"

vge::DebugRenderer& debug = vge::GetDebugRenderer();
debug.Initialize();

// In game loop:
debug.DrawLine(start, end, vge::Vec3(1, 0, 0));  // Red line
debug.DrawBox(min, max, vge::Vec3(0, 1, 0));      // Green box
debug.DrawSphere(center, radius, vge::Vec3(0, 0, 1));  // Blue sphere

// Specialized visualizations
debug.DrawNavMesh(navMesh);
debug.DrawPath(path);
debug.DrawChunkBorder(chunkX, chunkY, chunkZ);
debug.DrawEntityBounds(min, max);
debug.DrawAIState(position, "Idle");

// Render all debug commands
debug.Render(camera);
debug.Update(deltaTime);  // Update persistent commands
debug.Clear();            // Clear immediate commands
```

---

## 13. Common Patterns

### Game State Machine
```cpp
enum class GameState {
    MainMenu,
    Playing,
    Paused,
    Inventory,
    Crafting,
    Editor
};

GameState currentState = GameState::MainMenu;

void Update(float deltaTime) {
    switch (currentState) {
        case GameState::Playing:
            UpdateGameplay(deltaTime);
            break;
        case GameState::Paused:
            UpdatePauseMenu(deltaTime);
            break;
        case GameState::Editor:
            editor.Update(deltaTime, input);
            break;
        // ...
    }
}
```

### Loading Screen
```cpp
void ShowLoadingScreen(const std::string& message) {
    renderer.BeginFrame();
    renderer.SetClearColor(0, 0, 0, 1);
    
    // Render loading text
    ui.RenderText(message, screenWidth / 2, screenHeight / 2);
    ui.RenderProgressBar(progress, screenWidth / 2 - 200, screenHeight / 2 + 50, 400, 20);
    
    renderer.EndFrame();
    window.SwapBuffers();
}

// Usage:
ShowLoadingScreen("Generating world...");
world.GenerateTerrain(0, 0);
ShowLoadingScreen("Spawning entities...");
SpawnEntities();
```

### Async Chunk Loading
```cpp
// Load chunks in background
std::thread chunkLoader([&]() {
    while (running) {
        vge::Vec3 playerChunk = WorldToChunk(player.GetPosition());
        
        for (int dx = -renderDistance; dx <= renderDistance; dx++) {
            for (int dz = -renderDistance; dz <= renderDistance; dz++) {
                int cx = playerChunk.x + dx;
                int cz = playerChunk.z + dz;
                
                if (!world.HasChunk(cx, cz)) {
                    world.LoadChunkAsync(cx, cz);
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
});
```

---

## 14. Troubleshooting

### Build Errors

**Error:** `GL/gl.h: No such file or directory`
```bash
# Linux
sudo apt install mesa-common-dev libgl1-mesa-dev

# macOS
# OpenGL is deprecated but still available. Link with -framework OpenGL
```

**Error:** `glfw3.h: No such file or directory`
```bash
# Linux
sudo apt install libglfw3-dev

# Or let CMake fetch it automatically (already configured)
```

**Error:** `undefined reference to 'pthread_create'`
```bash
# Already fixed in CMakeLists.txt — links pthread on Linux
```

### Runtime Errors

**Error:** `Failed to create OpenGL context`
- Update graphics drivers
- Ensure OpenGL 3.3+ support: `glxinfo | grep "OpenGL version"`
- The engine requires OpenGL 3.3+ for VAO/VBO support

**Error:** `Segmentation fault in RenderWorld`
- Check that world.Initialize() was called
- Verify renderer.Initialize() succeeded

**Error:** `Black screen`
- Check camera position (might be inside a block)
- Verify renderer.Initialize() was called successfully
- Check that chunks are loaded (call `world.GetOrCreateChunk(x, y, z)`)
- Ensure blocks are placed above ground level (y >= 0)

**Error:** `Dear ImGui not found`
Verify that the repository checkout includes the bundled files under `third_party/imgui/`.

### Performance Issues

**Low FPS:**
```cpp
// Reduce render distance
world.SetRenderDistance(8);  // chunks (default: 16)

// Disable expensive effects
renderer.SetShadowsEnabled(false);
renderer.SetPostProcessingEnabled(false);
renderer.SetParticleLimit(1000);

// Enable frustum culling
renderer.SetFrustumCulling(true);
```

**Memory usage:**
```cpp
// Limit chunk cache
world.SetMaxCachedChunks(256);  // default: 1024

// Enable chunk unloading
world.SetUnloadDistance(32);  // chunks
```

---

## 15. Next Steps

1. **Read the RAG docs:** `docs/rag/` — 19 documents covering every system
2. **Check the wiki:** https://github.com/Mohobie/Aether-Game-Engine/wiki
3. **Study examples:** Look at `src/main.cpp` for a complete game loop
4. **Add mods:** Create Lua scripts in `mods/` folder
5. **Customize:** Modify block types, entity archetypes, recipes in JSON

### Useful RAG Documents
| Document | What You'll Learn |
|----------|-------------------|
| `01-overview.md` | Engine architecture, conventions |
| `03-voxel-world.md` | Block manipulation, chunk loading |
| `05-entity-component-system.md` | Entities, components, systems |
| `10-ai-system.md` | Behavior trees, pathfinding |
| `13-common-patterns.md` | Complete game implementations |
| `14-inventory-crafting.md` | Items, recipes, smelting |
| `15-entity-archetypes.md` | Entity types, environmental effects |
| `19-editor-system.md` | Editor, debug visualization |

### Renderer Architecture

The engine uses a **modern OpenGL renderer** with:
- **VAOs/VBOs** for efficient geometry management
- **Indexed drawing** (`glDrawElements`) for optimal performance
- **Proper buffer lifecycle** (creation, usage, cleanup)
- **Colored voxel rendering** with per-block-type colors

**Key classes:**
- `Renderer` — Main rendering interface
- `Shader` — GLSL shader program management
- `Mesh` — Geometry buffer management
- `Camera` — View/projection matrices
- `WorldRenderer` — Chunk mesh building and rendering

---

## 16. Getting Help

- **Check the wiki:** https://github.com/Mohobie/Aether-Game-Engine/wiki
- **Read RAG docs:** `docs/rag/` — AI-optimized with copy-paste examples
- **Check tests:** `tests/` — Working examples of every system
- **Git log:** `git log --oneline` — See recent changes

---

*Happy voxel building! 🧱*
