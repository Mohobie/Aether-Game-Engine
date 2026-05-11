# Cody Task List - Aether Game Engine

## Status
- Queue cleared of stale tasks from 2026-05-10
- 15 new tasks organized in 3 phases
- All tasks reference existing engine code (no reinventing)

---

## Phase 1: Make It Runnable (HIGH Priority)

### phase1-001: Create src/main.cpp
**Task:** Game entry point with GLFW window, render loop, and basic game state management. Should initialize all engine systems (renderer, world, input, audio) and run the main loop.

**Key Files to Reference:**
- `src/platform/platform_common.cpp` - Window creation
- `src/rendering/renderer.cpp` - Renderer initialization
- `src/voxel/world.cpp` - World setup
- `src/platform/input_manager.cpp` - Input setup
- `src/audio/audio_engine.cpp` - Audio init

**Requirements:**
- Create GLFW window (1920x1080, resizable)
- Initialize all subsystems in correct order
- Main loop with delta time calculation
- Proper cleanup on exit
- Return 0 on success

---

### phase1-002: Integrate renderer with voxel world
**Task:** Wire the existing Renderer class to render Chunk meshes as colored cubes. Create a simple block mesh builder that generates vertex data for each block type with distinct colors.

**Key Files to Reference:**
- `src/rendering/renderer.cpp` / `.h`
- `src/rendering/mesh.cpp` / `.h`
- `src/voxel/chunk.cpp` / `.h`
- `src/voxel/block_types.h`

**Requirements:**
- Generate cube mesh for each block type
- Color coding: Grass=green, Dirt=brown, Stone=gray, Wood=brown, etc.
- Only render visible faces (check neighboring blocks)
- Batch by chunk for efficiency
- Update mesh when chunk changes

---

### phase1-003: Implement fly camera controller
**Task:** Create a first-person camera controller with WASD movement, mouse look, and scroll wheel for speed. Should use the existing Camera class and InputManager.

**Key Files to Reference:**
- `src/rendering/camera.cpp` / `.h`
- `src/platform/input_manager.cpp` / `.h`
- `src/math/mat4.cpp` / `.h`

**Requirements:**
- WASD movement relative to camera look direction
- Mouse look (yaw/pitch, clamp pitch to -89 to 89 degrees)
- Scroll wheel adjusts movement speed
- Space = up, Shift = down (fly mode)
- Integrate into main game loop

---

### phase1-004: Add block placement and removal
**Task:** Integrate BlockPicker with input system. Left click removes block, right click places selected block type. Show crosshair and highlight selected block face.

**Key Files to Reference:**
- `src/core/raycast.cpp` / `.h`
- `src/voxel/world.cpp` / `.h`
- `src/voxel/block_types.h`
- `src/platform/input_manager.cpp` / `.h`

**Requirements:**
- Raycast from camera center
- Left click = set block to Air
- Right click = place selected block type
- Visual feedback: crosshair + block highlight
- Reach distance: 15 blocks
- Number keys 1-9 select block type

---

### phase1-005: Create sandbox game mode
**Task:** Simple game mode class that ties together world generation, player camera, and block editing. Generate a flat terrain on startup. Creative mode building only.

**Key Files to Reference:**
- `src/voxel/world_generator.cpp` / `.h`
- `src/voxel/world.cpp` / `.h`
- All Phase 1 tasks above

**Requirements:**
- Generate flat grass terrain (10x10 chunks)
- Spawn player at (0, 20, 0)
- Enable fly camera
- Enable block editing
- Press F3 for debug info (FPS, position, chunk coords)
- Press F1 to toggle editor mode

---

## Phase 2: Core Gameplay (MEDIUM Priority)

### phase2-001: Add block textures
**Task:** Load PNG textures for common blocks. Create a texture atlas. Update block mesh builder to use proper UV coordinates.

**Requirements:**
- Load textures from `assets/textures/` directory
- Create texture atlas (256x256 per block, 16x16 grid)
- UV mapping for each block face
- Fallback to colors if texture missing

---

### phase2-002: Implement survival game mode
**Task:** Add health, hunger, and damage systems. Blocks take time to break. Player takes fall damage. Hunger drain over time.

**Key Files to Reference:**
- `src/entity/components.cpp` - HealthComponent
- `src/core/player_controller.cpp`

---

### phase2-003: Add inventory UI to game
**Task:** Integrate existing InventoryUI into main game. Press E to open. Show hotbar. Number keys 1-9 for selection.

**Key Files to Reference:**
- `src/ui/ui_inventory.cpp` / `.h`
- `src/ui/ui_system.cpp` / `.h`
- `tests/test_inventory_system.cpp` - Examples

---

### phase2-004: Implement crafting in-game
**Task:** Integrate CraftingSystem with inventory UI. Add crafting table UI with 3x3 grid.

**Key Files to Reference:**
- `src/core/crafting.cpp` / `.h`
- `src/core/crafting_inventory_bridge.cpp`

---

### phase2-005: Add basic mobs
**Task:** Simple passive mob (pig) that wanders. Hostile mob (zombie) that chases player at night.

**Key Files to Reference:**
- `src/ai/behavior_tree.cpp` / `.h`
- `src/ai/ai_system.cpp` / `.h`
- `src/entity/entity.cpp` / `.h`

---

## Phase 3: Polish (LOW Priority)

### phase3-001: Create main menu
**Task:** Title screen with Singleplayer, Multiplayer, Options, Quit buttons.

---

### phase3-002: Add pause menu
**Task:** ESC opens pause overlay. Resume, Options, Save, Load, Quit.

---

### phase3-003: Implement settings menu
**Task:** Graphics, audio, controls settings. Save to config file.

---

### phase3-004: Add sound effects and music
**Task:** Block sounds, footsteps, ambient music, mob sounds.

---

### phase3-005: Polish and bug fixes
**Task:** Fix voxel_test crash. Add assertions to world_test/save_test. Performance profiling.

---

## Cody Rules
- Ship working code - tests pass, no regressions
- Use existing engine code - don't reinvent
- Commit to Gitea after each task
- Update docs/rag/ if APIs change
- Tests are non-negotiable
