# Lava Damage System

## Quick Reference

```cpp
vge::LavaDamageSystem lavaDamage(&world);

// Update every frame
lavaDamage.Update(playerPosition, deltaTime);

// Set callback
lavaDamage.onLavaDamage = [](float damage) {
    player.TakeDamage(damage);
};
```

## Features

### Damage
- **Damage Per Second:** 4 damage/second (configurable)
- **Damage Interval:** Every 0.5 seconds (configurable)
- **Continuous:** Damage while standing in lava
- **Tracking:** Time spent in lava

### Detection
- Checks block at player feet
- Also checks one block below
- Works with any block tagged as "lava"

## Implementation

```cpp
vge::LavaDamageSystem lavaDamage(&world);

// Configure
lavaDamage.SetDamagePerSecond(6.0f);  // 6 damage per second
lavaDamage.SetDamageInterval(1.0f);   // Apply every 1 second

// In game loop
void Update(float deltaTime) {
    lavaDamage.Update(player.GetPosition(), deltaTime);
}

// Handle damage
lavaDamage.onLavaDamage = [](float damage) {
    player.health -= damage;
    audio.PlaySound("sizzle");
    particles.SpawnLavaParticles(player.GetPosition());
};
```

## Files
- `src/game/lava_damage.h`
- `src/game/lava_damage.cpp`
