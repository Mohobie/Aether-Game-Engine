# Engine Changes Log

**Purpose:** Track all engine-level modifications (not game-specific code)
**Last Updated:** 2026-05-13

---

## Change #1: Renderer Face Culling Fix
**Date:** 2026-05-12
**Files Modified:**
- `src/rendering/renderer.cpp`

**Description:** Fixed face visibility check to properly handle cross-chunk boundaries. Previously only checked within the same chunk, causing missing faces at chunk borders.

**Engine Impact:** Core rendering system
**Breaking Change:** No

---

## Change #2: Renderer Transparency Support
**Date:** 2026-05-12
**Files Modified:**
- `src/rendering/renderer.cpp`

**Description:** Added proper transparency handling using `BlockDef::IsOpaque()` instead of just checking for air blocks. Now leaves, water, and other transparent blocks correctly show faces behind them.

**Engine Impact:** Core rendering system
**Breaking Change:** No

---

## Change #3: Player Controller Enhancement
**Date:** 2026-05-12
**Files Modified:**
- `src/core/player_controller.cpp`
- `src/core/player_controller.h`

**Description:** Implemented full first-person controller with:
- WASD movement with collision detection
- Mouse look with pitch clamping
- Jumping with gravity
- Sprinting (Shift key)
- Block breaking/placing raycasts
- `GetLookDirection()` and `GetRightDirection()` methods

**Engine Impact:** Core player controller system
**Breaking Change:** No (added methods, changed defaults)

---

## Change #4: World Generator System
**Date:** 2026-05-12
**Files Modified:**
- `src/voxel/world_generator.cpp` (new)
- `src/voxel/world_generator.h` (new)

**Description:** Added procedural world generation with multiple terrain types:
- `GenerateFlatWorld()` - Flat terrain with trees
- `GenerateHillyWorld()` - Perlin noise hills with caves
- `GenerateForestWorld()` - Forest with water bodies
- `GenerateDesertWorld()` - Desert with dunes

**Engine Impact:** World generation system
**Breaking Change:** No

---

## Change #5: Chunk Loading Fix
**Date:** 2026-05-12
**Files Modified:**
- `src/voxel/world.cpp`
- `src/voxel/chunk_manager.cpp`

**Description:** 
- Fixed `chunk->loaded` flag not being set when chunks are created
- Fixed `chunk_manager.cpp` to handle missing `GenerateChunk()` method
- Added `GetSeed()` accessor to World class

**Engine Impact:** Core chunk/world system
**Breaking Change:** No

---

## Change #6: Multiplayer Network Architecture
**Date:** 2026-05-13
**Files Modified:**
- `src/network/game_server.h` (new)
- `src/network/game_server.cpp` (new)
- `src/network/game_client.h` (new)
- `src/network/game_client.cpp` (new)
- `src/network/multiplayer_game.h` (new)
- `src/network/multiplayer_game.cpp` (new)
- `src/network/game_network_protocol.h` (new)
- `CMakeLists.txt`

**Description:** Complete client-server multiplayer system:
- Authoritative game server with player management
- Client with prediction, interpolation, reconciliation
- Network protocol with 15+ message types
- Anti-cheat validation (distance checks)
- Day/night cycle synchronization
- Chat system

**Engine Impact:** Core networking system
**Breaking Change:** No

---

## Change #7: Input Manager Mouse Support
**Date:** 2026-05-12
**Files Modified:**
- `src/platform/window.cpp`
- `src/platform/window.h`

**Description:** Added GLFW mouse callback support for camera look:
- `SetCursorCallback()` for mouse movement
- `SetCursorMode()` for cursor locking

**Engine Impact:** Input system
**Breaking Change:** No

---

## Pending Engine Changes

### Block Registry Extensions
- Add more block properties (hardness, tool requirements, drops)
- Block state system (directional blocks, powered, etc.)

### Physics Improvements
- Better collision detection (AABB sweeps)
- Velocity-based movement
- Step-up for stairs

### Rendering Optimizations
- Frustum culling
- Occlusion culling
- LOD system for distant chunks

---

*Note: Game-specific code (my_game.cpp, demo scenes) should NOT be committed to engine repo*
