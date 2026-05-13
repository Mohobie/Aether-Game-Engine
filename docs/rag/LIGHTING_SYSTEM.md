# Lighting System

## Quick Reference

```cpp
vge::LightSystem lights(&world);

// Place a torch
lights.PlaceTorch(10, 5, 10, 14); // x, y, z, light level

// Update sky light based on time of day
lights.UpdateSkyLightForTime(0.5f); // 0.0=night, 1.0=day

// Get light level at position
int light = lights.GetTotalLightLevel(10, 5, 10);

// Check if dark enough for mob spawning
bool spawnMobs = lights.IsDarkEnoughForMobs(10, 5, 10, 7);
```

## Features

### Block Light
- **Sources:** Torches, glowstone, lava
- **Level:** 0-15
- **Radius:** Configurable spread
- **Propagation:** Flood fill algorithm

### Sky Light
- **Day/Night:** Varies with time
- **Intensity:** 0.0-1.0
- **Updates:** When time changes
- **Combined:** Added to block light

### Light Queries
```cpp
// Total light (block + sky)
int total = lights.GetTotalLightLevel(x, y, z);

// Just block light
int block = lights.GetBlockLight(x, y, z);

// Just sky light
int sky = lights.GetSkyLight(x, y, z);
```

### Mob Spawning
```cpp
// Check if dark enough for mobs
if (lights.IsDarkEnoughForMobs(x, y, z, 7)) {
    SpawnMob(x, y, z);
}
```

## Implementation

```cpp
vge::LightSystem lights(&world);

// Add light sources
lights.PlaceTorch(10, 5, 10);
lights.AddBlockLightSource(Vec3(20, 10, 20), 15, 10);

// Update lighting
lights.UpdateAllLighting();

// Update sky light
lights.SetSkyLightIntensity(0.8f); // Day time
```

## Files
- `src/rendering/light_system.h`
- `src/rendering/light_system.cpp`
