# Mob Spawning Rules

## Quick Reference

```cpp
vge::MobSpawnRules spawnRules;
spawnRules.Initialize();

// Check if mob can spawn
bool canSpawn = spawnRules.CanSpawnMob("zombie", x, y, z, world);
```

## Features

### Spawning Conditions
- **Light Level:** Hostile mobs spawn in darkness (light < 7)
- **Biome:** Different mobs spawn in different biomes
- **Height:** Some mobs only spawn at certain Y levels
- **Space:** Requires 2-block tall space

### Mob Categories
| Category | Light Level | Biome | Time |
|----------|-------------|-------|------|
| Hostile | < 7 | Any | Night |
| Passive | > 7 | Plains/Forest | Day |
| Water | Any | Ocean/River | Any |

## Implementation

```cpp
vge::MobSpawnRules spawnRules;
spawnRules.Initialize();

// In game loop
void UpdateSpawning() {
    for (auto& chunk : loadedChunks) {
        if (spawnRules.CanSpawnMob("zombie", chunk.x, y, chunk.z, world)) {
            SpawnMob("zombie", chunk.x, y, chunk.z);
        }
    }
}
```

## Files
- `src/game/mob_spawn_rules.h`
- `src/game/mob_spawn_rules.cpp`
