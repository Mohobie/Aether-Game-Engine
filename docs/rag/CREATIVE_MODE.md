# Creative Mode

## Quick Reference

```cpp
vge::SandboxMode creative(world, generator, chunks, camera, input, renderer);
creative.Initialize();

// Flight controls
creative.SetFlySpeed(10.0f); // Faster flight

// Infinite blocks
creative.SetSelectedBlock(blockId); // Any block type

// Instant break - just press Q
```

## Features

### Flight
- **WASD:** Horizontal movement
- **Space:** Fly up
- **Shift:** Fly down
- **Arrow keys:** Look around
- **Adjustable speed:** SetFlySpeed()

### Block Editing
- **E:** Place selected block
- **Q:** Remove block instantly
- **Any block:** All types available
- **No durability:** Tools don't break

### No Survival Mechanics
- **No health:** Invulnerable
- **No hunger:** No starvation
- **No damage:** Lava, falls, mobs - all safe
- **No death:** Can't die

## Controls

| Key | Action |
|-----|--------|
| WASD | Fly |
| Space | Fly up |
| Shift | Fly down |
| Arrow Keys | Look |
| E | Place block |
| Q | Remove block |
| F1 | Toggle editor |
| F3 | Debug overlay |
| ESC | Quit |

## Files
- `src/game/sandbox_mode.h`
- `src/game/sandbox_mode.cpp`
