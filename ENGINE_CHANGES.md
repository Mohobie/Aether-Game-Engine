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
**RAG Doc:** `docs/rag/RENDERER_FIXES.md`

---

## Change #2: Renderer Transparency Support
**Date:** 2026-05-12
**Files Modified:**
- `src/rendering/renderer.cpp`

**Description:** Added proper transparency handling using `BlockDef::IsOpaque()` instead of just checking for air blocks. Now leaves, water, and other transparent blocks correctly show faces behind them.

**Engine Impact:** Core rendering system
**Breaking Change:** No
**RAG Doc:** `docs/rag/RENDERER_FIXES.md`

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
**RAG Doc:** `docs/rag/PLAYER_CONTROLLER.md`

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
**RAG Doc:** `docs/rag/NETWORKING.md`
**Wiki Doc:** `docs/MULTIPLAYER_ARCHITECTURE.md`

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

## Change #8: World Persistence (Save/Load)
**Date:** 2026-05-13
**Files Modified:**
- `src/core/save_game.h` (new)
- `src/core/save_game.cpp` (new)

**Description:** Save game manager with:
- Multiple save slots
- Quick save/load
- Auto-save every 5 minutes
- Player position, world seed, day time persistence
- Save metadata (date, size)

**Engine Impact:** Core save system
**Breaking Change:** No
**RAG Doc:** `docs/rag/SAVE_SYSTEM.md`

---

## Change #9: Day/Night Cycle Integration
**Date:** 2026-05-13
**Files Modified:**
- `src/rendering/renderer.cpp`
- `src/rendering/light_system.h`
- `src/rendering/light_system.cpp`
- `my_game.cpp` (example integration)

**Description:** Integrated existing day/night cycle into renderer:
- Sky color changes based on time (dawn/day/dusk/night)
- Clear color uses sky top color from cycle
- Time skip controls
- Dynamic sky light updates based on day/night blend
- Light level checks for mob spawning

**Engine Impact:** Rendering system
**Breaking Change:** No
**RAG Doc:** `docs/rag/DAY_NIGHT_CYCLE.md`

---

## Change #10: Survival System
**Date:** 2026-05-13
**Files Modified:**
- `src/core/survival_system.h` (new)
- `src/core/survival_system.cpp` (new)

**Description:** Complete survival mechanics:
- Health system (20 max, regen when fed)
- Hunger system (decays over time, starvation damage)
- Breath/drowning system (underwater damage)
- Death and respawn with invulnerability
- Experience/leveling system

**Engine Impact:** Core gameplay system
**Breaking Change:** No
**RAG Doc:** `docs/rag/SURVIVAL_SYSTEM.md`

---

## Change #11: Ore Generator
**Date:** 2026-05-13
**Files Modified:**
- `src/voxel/ore_generator.h` (new)
- `src/voxel/ore_generator.cpp` (new)

**Description:** Procedural ore generation:
- Configurable ore types (coal, iron, gold, diamond, emerald)
- Vein-based generation with random walk
- Height-based distribution
- Rarity and vein size configuration

**Engine Impact:** World generation system
**Breaking Change:** No
**RAG Doc:** `docs/rag/ORE_GENERATION.md`

---

## Change #12: Tool System
**Date:** 2026-05-13
**Files Modified:**
- `src/game/tool_system.h` (new)
- `src/game/tool_system.cpp` (new)

**Description:** Tool crafting and usage:
- 5 tool types (pickaxe, axe, shovel, sword, hoe)
- 5 material tiers (wood, stone, iron, gold, diamond)
- Mining speed and durability stats
- Block mining requirements
- Combat damage calculation

**Engine Impact:** Gameplay system
**Breaking Change:** No
**RAG Doc:** `docs/rag/TOOL_SYSTEM.md`

---

## Change #14: Day/Night Cycle - Bed System
**Date:** 2026-05-13
**Files Modified:**
- `src/game/bed_system.h` (new)
- `src/game/bed_system.cpp` (new)
- `src/platform/input_manager.h`
- `src/platform/input_manager.cpp`
- `my_game.cpp`
- `CMakeLists.txt`

**Description:** Complete bed/sleep system for night skipping:
- Bed placement with directional orientation (head/foot)
- Sleep mechanic with requirements (night time, proximity, no monsters)
- Night skip with accelerated time (50x speed)
- Respawn point setting
- Monster proximity detection (8 block radius)
- Sleep state callbacks (on sleep start/end)
- Integration with DayNightCycle and LightSystem

**Engine Impact:** Gameplay system
**Breaking Change:** No
**RAG Doc:** `docs/rag/DAY_NIGHT_CYCLE.md`

---

## Change #15: Input Manager Key Additions
**Date:** 2026-05-13
**Files Modified:**
- `src/platform/input_manager.h`
- `src/platform/input_manager.cpp`

**Description:** Added missing key codes to input manager:
- `F` key for bed interaction
- `Key0` for debug time skip
- Cross-platform support (Linux terminal + Windows)

**Engine Impact:** Input system
**Breaking Change:** No

---

## Change #13: Mineshaft Generation System
**Date:** 2026-05-13
**Files Modified:**
- `src/voxel/mineshaft_generator.h` (new)
- `src/voxel/mineshaft_generator.cpp` (new)
- `CMakeLists.txt`

**Description:** Underground mineshaft structure generation:
- Corridor networks with wooden support beams
- Rail tracks along corridors (60% chance)
- Loot chests in rooms and corridors
- Cave spider spawners (15% chance per segment)
- Ladder shafts for vertical connections
- Room/hub areas at junctions (20% chance)
- Branching corridors (25% chance)
- Integration with existing cave generation

**Engine Impact:** World generation system
**Breaking Change:** No
**RAG Doc:** `docs/rag/MINESHAFTS.md`

---

## Documentation Status

| Change | Git Commit | RAG Doc | Wiki Doc | Status |
|--------|-----------|---------|----------|--------|
| Face Culling Fix | d2e0e67 | ✅ RENDERER_FIXES.md | - | ✅ Complete |
| Transparency | d2e0e67 | ✅ RENDERER_FIXES.md | - | ✅ Complete |
| Player Controller | d2e0e67 | ✅ PLAYER_CONTROLLER.md | - | ✅ Complete |
| World Generator | d2e0e67 | - | - | ⚠️ Missing RAG |
| Chunk Loading Fix | d2e0e67 | - | - | ⚠️ Missing RAG |
| Multiplayer | 0e3bd16 | ✅ NETWORKING.md | ✅ MULTIPLAYER_ARCHITECTURE.md | ✅ Complete |
| Input Mouse | d2e0e67 | - | - | ⚠️ Missing RAG |
| Save System | b285636 | ✅ SAVE_SYSTEM.md | - | ✅ Complete |
| Day/Night | 19e8d66 | ✅ DAY_NIGHT_CYCLE.md | - | ✅ Complete |
| Survival | 3ed4b79 | ✅ SURVIVAL_SYSTEM.md | - | ✅ Complete |
| Ore Generator | cb2f93e | ✅ ORE_GENERATION.md | - | ✅ Complete |
| Tool System | 12b6496 | ✅ TOOL_SYSTEM.md | - | ✅ Complete |
| Mineshaft Generator | - | ✅ MINESHAFTS.md | - | ✅ Complete |

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
