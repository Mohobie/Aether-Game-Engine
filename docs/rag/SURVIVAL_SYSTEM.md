# Survival System

## Quick Reference

```cpp
vge::SurvivalSystem survival;
survival.SetSpawnPoint(vge::Vec3(0, 50, 0));

// In game loop
survival.Update(deltaTime);

// Take damage from fall
survival.TakeDamage(fallDamage);

// Heal
survival.Heal(5.0f);

// Eat food
survival.Eat(4.0f);  // Restores 4 hunger, slight health

// Check status
if (survival.IsDead()) {
    survival.Respawn(spawnPoint);
}
```

## Features

### Health System
- **Max Health:** 20 HP
- **Regen:** Requires hunger > 50%, regenerates slowly
- **Damage:** From falls, drowning, starvation, mobs
- **Death:** At 0 HP, triggers respawn

### Hunger System
- **Max Hunger:** 20 points
- **Decay:** Slowly decreases over time
- **Starvation:** At 0 hunger, take 1 damage every 4 seconds
- **Eating:** Food restores hunger and slight health

### Breathing/Drowning
- **Max Breath:** 20 seconds underwater
- **Decay:** 2 breath/second underwater
- **Damage:** 2 HP/second when breath reaches 0
- **Regen:** Quickly regenerates when above water

### Experience/Leveling
- **XP:** Gained from mining, combat, crafting
- **Levels:** Each level requires 1.5x more XP
- **Benefits:** Higher levels = better tool efficiency

### Death/Respawn
- **Spawn Point:** Set with bed or command
- **Invulnerability:** 3 seconds after respawn
- **Death Count:** Tracked for statistics

## Configuration

```cpp
// Health
survival.SetHealth(20.0f);           // Current health
survival.Heal(5.0f);                 // Add health
survival.TakeDamage(3.0f);           // Remove health

// Hunger
survival.SetHunger(20.0f);           // Current hunger
survival.Eat(4.0f);                  // Add hunger
survival.Starve(1.0f);               // Remove hunger

// Breath
survival.SetBreath(20.0f);           // Current breath
survival.SetUnderwater(true);        // Start drowning timer

// Spawn
survival.SetSpawnPoint(pos);         // Set respawn location
Vec3 spawn = survival.GetSpawnPoint();
```

## Callbacks

```cpp
survival.onDeath = []() {
    std::cout << "Player died!" << std::endl;
};

survival.onRespawn = []() {
    std::cout << "Player respawned!" << std::endl;
};

survival.onHealthChanged = [](float newHealth) {
    UpdateHealthBar(newHealth);
};

survival.onHungerChanged = [](float newHunger) {
    UpdateHungerBar(newHunger);
};

survival.onLevelUp = [](int newLevel) {
    std::cout << "Level up! Now level " << newLevel << std::endl;
};
```

## Files
- `src/core/survival_system.h`
- `src/core/survival_system.cpp`
