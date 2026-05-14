# Fall Damage System

## Quick Reference

```cpp
vge::FallDamageSystem fallDamage;

// Update every frame
fallDamage.Update(playerPosition, player.IsOnGround());

// Set callback
fallDamage.onFallDamage = [](float damage) {
    player.TakeDamage(damage);
};
```

## Features

### Damage Calculation
- **Min Fall Distance:** 3 blocks (configurable)
- **Damage Per Block:** 1 heart per block after minimum
- **Max Damage:** 20 hearts (configurable)
- **Tracking:** Tracks highest point during fall

### Example
| Fall Height | Damage |
|-------------|--------|
| 3 blocks | 0 |
| 5 blocks | 2 |
| 10 blocks | 7 |
| 15 blocks | 12 |
| 20+ blocks | 20 (max) |

## Implementation

```cpp
vge::FallDamageSystem fallDamage;

// Configure
fallDamage.SetMinFallDistance(4.0f);  // 4 block minimum
fallDamage.SetDamagePerBlock(2.0f);   // 2 damage per block
fallDamage.SetMaxDamage(10.0f);       // Max 10 damage

// In game loop
void Update(float deltaTime) {
    bool onGround = physics.IsOnGround();
    fallDamage.Update(player.GetPosition(), onGround);
}

// Handle damage
fallDamage.onFallDamage = [](float damage) {
    player.health -= damage;
    audio.PlaySound("fall_impact");
};
```

## Files
- `src/game/fall_damage.h`
- `src/game/fall_damage.cpp`
