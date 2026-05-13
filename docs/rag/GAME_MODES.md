# Game Modes

## Quick Reference

```cpp
// Survival Mode
vge::SurvivalMode survival(world, generator, chunks, camera, input, renderer);
survival.Initialize();

// Update in game loop
survival.Update(deltaTime);

// Creative Mode (infinite blocks, flight)
vge::CreativeMode creative(world, camera, input, renderer);
creative.Initialize();
```

## Survival Mode

### Features
- **Health System:** 20 HP, regeneration when well-fed
- **Hunger System:** Decays over time, affects sprinting
- **Stamina:** Sprinting drains stamina
- **Day/Night Cycle:** Mobs spawn at night
- **Crafting:** Must craft tools to mine/progress
- **Death:** Respawn at spawn point with penalty

### Configuration
```cpp
survival.SetSpawnPoint(vge::Vec3(0, 20, 0));
survival.GenerateTerrain(5, 4); // 5 chunk radius, height 4
survival.SetNightHungerMultiplier(1.5f);
```

### Controls
- **WASD:** Movement
- **Shift:** Sprint (drains stamina)
- **E:** Place block
- **Q:** Remove block
- **F3:** Toggle debug overlay

## Creative Mode

### Features
- **Infinite Blocks:** All block types available
- **Flight:** Free movement in all directions
- **Instant Break:** One-click block removal
- **No Damage:** Invulnerable
- **No Hunger:** No survival mechanics

### Controls
- **WASD:** Movement
- **Space:** Fly up
- **Shift:** Fly down
- **E:** Place selected block
- **Q:** Remove block
- **Number keys:** Select block type

## Implementation

```cpp
// Create mode
auto mode = std::make_unique<vge::SurvivalMode>(
    &world, &generator, &chunks, &camera, &input, &renderer
);

// Initialize
if (!mode->Initialize()) {
    std::cerr << "Failed to initialize game mode" << std::endl;
    return 1;
}

// Game loop
while (running) {
    mode->Update(deltaTime);
    mode->Render();
}

// Cleanup
mode->Shutdown();
```

## Files
- `src/game/survival_mode.h`
- `src/game/survival_mode.cpp`
- `src/game/sandbox_mode.h`
- `src/game/sandbox_mode.cpp`
