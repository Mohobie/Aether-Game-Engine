# Aether Game Engine - Generic Entity System

## Overview

The Aether Game Engine uses a **fully generic, data-driven entity system**. There are NO hardcoded mob types like "zombie" or "skeleton". Instead, games define their own entities through archetypes.

## Quick Start

### 1. Define an Entity Archetype

```cpp
#include "ai/entity_ai.h"

using namespace vge;

// Create a goblin archetype
EntityArchetype goblin;
goblin.id = "goblin";
goblin.displayName = "Forest Goblin";
goblin.health = 50.0f;
goblin.speed = 6.0f;
goblin.damage = 8.0f;
goblin.attackRange = 1.5f;
goblin.detectionRange = 15.0f;
goblin.behavior = AIBehaviorType::Aggressive;
goblin.minLightLevel = 0;
goblin.maxLightLevel = 7;
goblin.spawnsOnGround = true;
goblin.modelId = "goblin_model";
goblin.textureId = "goblin_texture";

// Register it
EntityArchetypeRegistry::GetInstance()->RegisterArchetype(goblin);
```

### 2. Spawn Entities

```cpp
GenericEntitySpawner spawner(world, lightSystem, dayNightCycle);

// Spawn a goblin at position (10, 5, 10)
auto* entity = spawner.SpawnEntity("goblin", Vec3(10, 5, 10));
```

### 3. Define Any Entity Type

```cpp
// Passive wildlife
EntityArchetype deer;
deer.id = "deer";
deer.displayName = "White-tailed Deer";
deer.health = 20.0f;
deer.speed = 12.0f;
deer.behavior = AIBehaviorType::Passive;
deer.damagedBySunlight = false;

// Boss enemy
EntityArchetype dragon;
dragon.id = "dragon";
dragon.displayName = "Ancient Dragon";
dragon.health = 1000.0f;
dragon.speed = 8.0f;
dragon.damage = 50.0f;
dragon.attackRange = 10.0f;
dragon.detectionRange = 50.0f;
dragon.behavior = AIBehaviorType::Boss;
dragon.canFly = true;
dragon.damagedBySunlight = false;

// Aquatic creature
EntityArchetype shark;
shark.id = "shark";
shark.displayName = "Great White Shark";
shark.health = 80.0f;
shark.speed = 10.0f;
shark.damage = 25.0f;
shark.behavior = AIBehaviorType::Aggressive;
shark.spawnsInWater = true;
shark.spawnsOnGround = false;
shark.canSwim = true;

// Nocturnal predator
EntityArchetype werewolf;
werewolf.id = "werewolf";
werewolf.displayName = "Werewolf";
werewolf.health = 120.0f;
werewolf.speed = 9.0f;
werewolf.damage = 15.0f;
werewolf.behavior = AIBehaviorType::Aggressive;
werewolf.minTimeOfDay = 0.7f;  // Only spawns at night
werewolf.maxTimeOfDay = 0.3f;
werewolf.damagedBySunlight = true;  // Burns in daylight
```

## AI Behavior Types

| Type | Description |
|------|-------------|
| `Passive` | Runs from player when approached |
| `Neutral` | Ignores player unless attacked |
| `Aggressive` | Attacks on sight |
| `Territorial` | Defends a specific area |
| `Fleeing` | Runs away when damaged |
| `Boss` | Special attack patterns, high stats |

## AI States

| State | Description |
|-------|-------------|
| `IDLE` | Standing still, looking around |
| `WANDER` | Random movement |
| `CHASE` | Pursuing target |
| `ATTACK` | In attack range, dealing damage |
| `FLEE` | Running away |
| `DEAD` | Death animation/cleanup |
| `CUSTOM_1-3` | Game-specific states |

## Spawn Conditions

Entities can have conditions for spawning:

```cpp
EntityArchetype ghost;
ghost.minLightLevel = 0;     // Only in darkness
ghost.maxLightLevel = 3;
ghost.minTimeOfDay = 0.7f;   // Only at night
ghost.maxTimeOfDay = 0.3f;
ghost.spawnsOnGround = true;
ghost.spawnsInWater = false;
ghost.canClimbWalls = true;  // Can climb walls
```

## Environmental Effects

```cpp
EntityArchetype vampire;
vampire.damagedBySunlight = true;  // Takes damage in sun
vampire.damagedByWater = true;     // Takes damage in water

EntityArchetype fireElemental;
fireElemental.damagedByWater = true;
```

## Custom Properties

Add game-specific data:

```cpp
EntityArchetype wizard;
wizard.customFloats["mana"] = 100.0f;
wizard.customFloats["spell_power"] = 25.0f;
wizard.customStrings["element"] = "fire";
wizard.customBools["can_teleport"] = true;
```

## Events

```cpp
spawner.onEntitySpawned = [](const std::string& archetypeId, const Vec3& pos) {
    std::cout << "Spawned " << archetypeId << " at (" 
              << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
};

// Access spawned entities
auto entities = spawner.GetEntitiesByArchetype("goblin");
for (auto* entity : entities) {
    entity->controller->TakeDamage(10.0f);
}
```

## File-Based Definitions (Future)

When JSON support is added, archetypes can be defined in files:

```json
{
  "id": "goblin",
  "displayName": "Forest Goblin",
  "health": 50,
  "speed": 6,
  "damage": 8,
  "behavior": "aggressive",
  "spawnsOnGround": true,
  "minLightLevel": 0,
  "maxLightLevel": 7,
  "modelId": "goblin_model",
  "textureId": "goblin_texture"
}
```

## Comparison: Old vs New

### Old (Hardcoded)
```cpp
// Only zombies and skeletons existed
MobType::Zombie
MobType::Skeleton
MobType::Spider
```

### New (Generic)
```cpp
// Any entity type you define
"goblin"
"dragon"
"deer"
"werewolf"
"shark"
"fire_elemental"
// ... unlimited possibilities
```

## Migration Guide

If you have code using the old system:

```cpp
// OLD
MobSpawner spawner;
Mob* zombie = spawner.SpawnMob(MobType::Zombie, pos);

// NEW
GenericEntitySpawner spawner(world, lights, cycle);

// Define archetype once
EntityArchetype zombieArch;
zombieArch.id = "zombie";
zombieArch.health = 20;
// ... configure
EntityArchetypeRegistry::GetInstance()->RegisterArchetype(zombieArch);

// Spawn anytime
auto* entity = spawner.SpawnEntity("zombie", pos);
```

## Summary

The new entity system is:
- **Fully generic** - No hardcoded types
- **Data-driven** - Define entities through code or files
- **Extensible** - Custom properties and behaviors
- **Game-agnostic** - Works for any genre (RPG, FPS, survival, etc.)

Your game defines what entities exist, not the engine.
