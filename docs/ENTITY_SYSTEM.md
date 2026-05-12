# Aether Game Engine - Generic Entity System

## Overview

The Aether Game Engine provides a **fully generic, data-driven entity system**. 

**Important:** The engine comes with NO predefined entities. There are NO built-in zombies, skeletons, goblins, or dragons. Your game defines every entity type dynamically through archetypes.

## Quick Start

### 1. Define Your Own Entity Archetype

```cpp
#include "ai/entity_ai.h"

using namespace vge;

// YOUR game defines what entities exist
// Example: A space pirate for your sci-fi game
EntityArchetype spacePirate;
spacePirate.id = "space_pirate";           // Your unique ID
spacePirate.displayName = "Space Pirate";
spacePirate.health = 75.0f;
spacePirate.speed = 8.0f;
spacePirate.damage = 15.0f;
spacePirate.attackRange = 15.0f;           // Laser rifle range
spacePirate.detectionRange = 40.0f;        // Scanner range
spacePirate.behavior = AIBehaviorType::Aggressive;
spacePirate.canFly = true;                 // Jetpack
spacePirate.damagedBySunlight = false;     // Space helmet
spacePirate.modelId = "your_game/models/space_pirate.obj";
spacePirate.textureId = "your_game/textures/pirate_diffuse.png";

// Register it
EntityArchetypeRegistry::GetInstance()->RegisterArchetype(spacePirate);
```

### 2. Spawn Your Entity

```cpp
GenericEntitySpawner spawner(world, lightSystem, dayNightCycle);

// Spawn YOUR space pirate
auto* entity = spawner.SpawnEntity("space_pirate", Vec3(100, 50, 200));
```

### 3. Define Any Entity YOUR Game Needs

The engine doesn't limit what you can create. Some examples:

**Fantasy RPG:**
```cpp
EntityArchetype darkMage;
darkMage.id = "dark_mage";
darkMage.behavior = AIBehaviorType::Aggressive;
darkMage.customFloats["mana"] = 100.0f;
darkMage.customStrings["spell_type"] = "necromancy";
```

**Sci-Fi Shooter:**
```cpp
EntityArchetype droneSwarm;
droneSwarm.id = "drone_swarm";
droneSwarm.behavior = AIBehaviorType::Aggressive;
droneSwarm.canFly = true;
droneSwarm.minGroupSize = 3;
droneSwarm.maxGroupSize = 8;
```

**Survival Horror:**
```cpp
EntityArchetype shadowStalker;
shadowStalker.id = "shadow_stalker";
shadowStalker.behavior = AIBehaviorType::Fleeing;
shadowStalker.minLightLevel = 0;
shadowStalker.maxLightLevel = 3;  // Only in darkness
shadowStalker.damagedBySunlight = true;
```

**Peaceful Wildlife:**
```cpp
EntityArchetype forestDeer;
forestDeer.id = "forest_deer";
forestDeer.behavior = AIBehaviorType::Passive;
forestDeer.health = 20;
forestDeer.speed = 15;  // Fast runner
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
EntityArchetype caveTroll;
caveTroll.minLightLevel = 0;     // Only in darkness
caveTroll.maxLightLevel = 5;
caveTroll.minTimeOfDay = 0.0f;   // Any time
caveTroll.maxTimeOfDay = 1.0f;
caveTroll.spawnsOnGround = true;
caveTroll.spawnsInWater = false;
```

## Environmental Effects

```cpp
EntityArchetype lavaGolem;
lavaGolem.damagedByWater = true;      // Water hurts it
lavaGolem.damagedBySunlight = false;   // Sun is fine

EntityArchetype frostWraith;
frostWraith.damagedBySunlight = true;  // Melts in sun
frostWraith.canFly = true;
```

## Custom Properties

Add game-specific data:

```cpp
EntityArchetype cyborgBoss;
cyborgBoss.customFloats["shield_strength"] = 500.0f;
cyborgBoss.customFloats["rocket_damage"] = 100.0f;
cyborgBoss.customStrings["weakness"] = "electricity";
cyborgBoss.customBools["has_second_form"] = true;
```

## Events

```cpp
spawner.onEntitySpawned = [](const std::string& archetypeId, const Vec3& pos) {
    std::cout << "Spawned " << archetypeId << " at (" 
              << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
};

// Access spawned entities
auto entities = spawner.GetEntitiesByArchetype("space_pirate");
for (auto* entity : entities) {
    entity->controller->TakeDamage(10.0f);
}
```

## File-Based Definitions (Future)

When JSON support is added, archetypes can be defined in files:

```json
{
  "id": "space_pirate",
  "displayName": "Space Pirate",
  "health": 75,
  "speed": 8,
  "damage": 15,
  "behavior": "aggressive",
  "canFly": true,
  "modelId": "your_game/models/space_pirate.obj"
}
```

## Key Principle

**The engine provides the systems. You provide the content.**

The engine handles:
- AI state machines
- Pathfinding
- Spawning logic
- Combat calculations
- Environmental effects

Your game provides:
- What entities exist
- Their stats and behavior
- Their visual appearance
- Their spawn conditions
- Their special abilities

## Summary

The entity system is:
- **Fully generic** - No built-in entity types
- **Data-driven** - Define entities through code or files
- **Extensible** - Custom properties and behaviors
- **Game-agnostic** - Works for any genre

**You define what exists in your world. The engine just makes it work.**
