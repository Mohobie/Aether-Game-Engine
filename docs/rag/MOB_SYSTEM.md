# Mob System

## Quick Reference

```cpp
vge::MobSystem mobSystem;

// Register mob types
vge::MobDef pigDef;
pigDef.id = "pig";
pigDef.type = vge::MobType::Passive;
pigDef.maxHealth = 10.0f;
pigDef.color = vge::Vec3(1.0f, 0.7f, 0.7f);
mobSystem.RegisterMob(pigDef);

// Spawn a mob
mobSystem.SpawnMob("pig", vge::Vec3(10, 15, 10));

// Update in game loop
mobSystem.Update(deltaTime, playerPos, dayNightBlend);
```

## Mob Types

| Type | Behavior | Examples |
|------|----------|----------|
| Passive | Wanders, flees when hit | Pig, Cow, Chicken |
| Neutral | Attacks if provoked | Wolf, Bear |
| Hostile | Attacks on sight | Zombie, Skeleton |

## Mob Properties

```cpp
vge::MobDef def;
def.id = "zombie";
def.name = "Zombie";
def.type = vge::MobType::Hostile;
def.maxHealth = 20.0f;
def.speed = 2.5f;
def.damage = 3.0f;
def.color = vge::Vec3(0.2f, 0.5f, 0.2f);
def.size = 1.0f;
def.spawnsAtDay = false;   // Only at night
def.spawnsAtNight = true;
def.drops = {"rotten_flesh"};
```

## Spawning

```cpp
// Configure spawn rules
mobSystem.SetSpawnRadius(20.0f, 50.0f);  // Min/max distance from player
mobSystem.SetMaxMobs(20);                  // Max mobs in world

// Automatic spawning based on time of day
mobSystem.Update(deltaTime, playerPos, dayNightBlend);
```

## AI Behaviors

### Passive Mobs
- Wander randomly
- Flee when damaged (5 seconds)
- No attack

### Hostile Mobs
- Detect player within 15 blocks
- Chase and attack within 1.5 blocks
- Lose interest beyond 25 blocks
- Attack cooldown: 1 second

## Combat

```cpp
// Damage mob
mobSystem.DamageMob(mobIndex, 5.0f);

// Kill mob (drops items)
mobSystem.KillMob(mobIndex);

// Access mob data
auto& mobs = mobSystem.GetMobs();
for (auto& mob : mobs) {
    if (mob.alive) {
        // Render mob at mob.position
    }
}
```

## Files
- `src/game/mob_system.h`
- `src/game/mob_system.cpp`
