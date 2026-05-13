# Mob AI System

## Quick Reference

```cpp
vge::MobAIController ai(&mob, &world);

// Set target (player position)
ai.SetTarget(playerPos);

// Update AI
ai.Update(deltaTime);

// Check state
if (ai.GetState() == vge::MobAIState::Chase) {
    // Mob is chasing player
}
```

## Features

### AI States
| State | Behavior | Trigger |
|-------|----------|---------|
| Idle | Standing still | No target |
| Wander | Random walking | No target, timer |
| Chase | Following target | Target in range |
| Attack | Attacking target | Target in attack range |
| Flee | Running away | Low health |
| Patrol | Following path | Predefined route |

### Detection
- **Detection Range:** 10 blocks default
- **Attack Range:** 1.5 blocks default
- **Lose Interest:** 20 blocks default
- **Line of Sight:** Checks for block obstructions

### Pathfinding
- **Simple path:** Straight line with obstacle avoidance
- **Path update:** Every 0.5 seconds
- **Climbing:** Goes up when blocked

## Implementation

```cpp
// Create AI controller
vge::MobAIController ai(&mob, &world);

// Set callbacks
ai.onStateChange = []() {
    std::cout << "Mob state changed" << std::endl;
};

ai.onAttack = []() {
    player.TakeDamage(mob.damage);
};

// Set detection ranges
ai.SetDetectionRange(15.0f);
ai.SetAttackRange(2.0f);

// In game loop
ai.SetTarget(player.GetPosition());
ai.Update(deltaTime);
```

## Files
- `src/ai/mob_ai.h`
- `src/ai/mob_ai.cpp`
