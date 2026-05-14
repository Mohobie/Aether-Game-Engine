# Creative Mode

## Quick Reference

```cpp
#include "game/creative_mode.h"

// Create creative mode
vge::CreativeMode creative(world, generator, chunks, camera, input, renderer);
creative.Initialize();

// Update in game loop
creative.Update(deltaTime);
creative.Render();
```

## Features

### 1. Flight Mode
- **WASD:** Horizontal movement
- **Space:** Fly up
- **Shift:** Fly down
- **Mouse:** Look around
- **Ctrl:** Fast flight (25 blocks/sec)
- **F5:** Toggle flight on/off
- **Adjustable speed:** SetFlySpeed() / SetFlySpeedFast()

### 2. Infinite Blocks
- All registered blocks available in creative inventory
- No resource consumption when placing
- Unlimited placement

### 3. Instant Block Break
- Press Q to remove any block instantly
- No mining time
- No tool requirements
- Bedrock can be removed (unlike survival)

### 4. No Damage / Invincible
- No health system
- No hunger
- No fall damage
- No lava damage
- No mob damage
- Invincibility toggle available

### 5. Creative Inventory
- All block types from BlockRegistry automatically loaded
- 9-slot hotbar (number keys 1-9)
- Next/Prev slot cycling

## Controls

| Key | Action |
|-----|--------|
| WASD | Fly movement |
| Space | Fly up |
| Shift | Fly down |
| Mouse | Look around |
| E | Place block |
| Q | Remove block (instant) |
| 1-9 | Select hotbar slot |
| F3 | Toggle debug overlay |
| F5 | Toggle flight mode |
| ESC | Quit / pause |

## API

### Construction
```cpp
CreativeMode(World* world, WorldGenerator* generator, ChunkManager* chunks,
             Camera* camera, Input* input, Renderer* renderer);
```

### Initialization
```cpp
bool Initialize();  // Creates subsystems, generates terrain, spawns player
void Shutdown();    // Cleans up all subsystems
```

### Game Loop
```cpp
void Update(float deltaTime);
void Render();
```

### Flight
```cpp
void ToggleFlightMode();
void SetFlightModeEnabled(bool enabled);
bool IsFlightModeEnabled() const;

void SetFlySpeed(float speed);       // Normal speed (default: 10)
void SetFlySpeedFast(float speed);   // Sprint speed (default: 25)
```

### Block Editing
```cpp
bool PlaceBlockAt(const Vec3& position, const Vec3& normal);
bool RemoveBlockAt(const Vec3& position);
void SetSelectedBlock(BlockTypeID type);
BlockTypeID GetSelectedBlock() const;
```

### Inventory
```cpp
void SelectSlot(int slot);           // 0-8
void NextSlot();
void PrevSlot();
int GetSelectedSlot() const;
const std::vector<std::string>& GetCreativeInventory() const;
```

### Toggles
```cpp
void ToggleInstantBreak();
void SetInstantBreakEnabled(bool enabled);
bool IsInstantBreakEnabled() const;

void ToggleInvincible();
void SetInvincible(bool enabled);
bool IsInvincible() const;

void ToggleDebugOverlay();
void SetDebugOverlayVisible(bool visible);
bool IsDebugOverlayVisible() const;
```

### Player
```cpp
void SpawnPlayer(const Vec3& position);
void SetPlayerPosition(const Vec3& pos);
Vec3 GetPlayerPosition() const;
```

### Terrain
```cpp
void GenerateTerrain(int chunkRadius, int height);
void ClearTerrain();
```

## Debug Overlay

Press F3 to show:
- FPS
- Player position
- Chunk coordinates
- Yaw/Pitch
- Flight mode status
- Instant break status
- Invincibility status
- Fly speed settings
- Selected slot/block
- Day/Night time

## Architecture

```
CreativeMode
├── VoxelEditor      - Block placement/removal tools
├── BlockPicker      - Raycast block selection
├── FlyCamera        - Free flight camera controller
├── BlockInteraction - Hotbar and input handling
├── DayNightCycle    - Visual day/night (no mob spawning)
└── CreativeInventory - All available blocks
```

## Differences from Survival Mode

| Feature | Creative | Survival |
|---------|----------|----------|
| Movement | Flight (no gravity) | Walking with gravity |
| Block placing | Infinite | Requires items |
| Block breaking | Instant | Mining time + tool reqs |
| Health | None (invincible) | 20 HP, damage enabled |
| Hunger | None | Decays over time |
| Crafting | Not needed | Required for tools/items |
| Mobs | No spawning | Spawn at night |
| Death | Impossible | Respawn at spawn point |

## Files
- `src/game/creative_mode.h`
- `src/game/creative_mode.cpp`
