# Entity Archetype System

The engine provides a **fully generic, data-driven entity system**. The engine comes with NO predefined entities. Your game defines every entity type dynamically through archetypes.

## Core Principle

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

## Entity Archetype

```cpp
#include "ai/entity_ai.h"

using namespace vge;

// YOUR game defines what entities exist
EntityArchetype archetype;
archetype.id = "space_pirate";           // Your unique ID
archetype.displayName = "Space Pirate";
archetype.health = 75.0f;
archetype.maxHealth = 75.0f;
archetype.speed = 8.0f;
archetype.damage = 15.0f;
archetype.attackRange = 15.0f;           // Laser rifle range
archetype.detectionRange = 40.0f;        // Scanner range
archetype.behavior = AIBehaviorType::Aggressive;
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

## Environmental Effects

All environmental effects are **configurable per archetype**:

```cpp
// Damage effects
archetype.damagedBySunlight = false;   // Takes damage in sunlight
archetype.damagedByWater = false;      // Takes damage in water (drowning)
archetype.damagedByDarkness = false;   // Takes damage in darkness

// Healing effects
archetype.healedBySunlight = false;    // Heals in sunlight
archetype.healedByWater = false;       // Heals in water
archetype.healedByDarkness = false;    // Heals in darkness
```

### Example Configurations

```cpp
// Vampire - hurt by sun, healed by darkness
EntityArchetype vampire;
vampire.damagedBySunlight = true;
vampire.healedByDarkness = true;

// Plant - healed by sun, hurt by darkness
EntityArchetype plant;
plant.healedBySunlight = true;
plant.damagedByDarkness = true;

// Aquatic creature - healed by water, hurt by sun
EntityArchetype fish;
fish.healedByWater = true;
fish.damagedBySunlight = true;

// Shadow creature - healed by darkness, hurt by sun
EntityArchetype shadow;
shadow.healedByDarkness = true;
shadow.damagedBySunlight = true;

// Normal mob - no environmental effects
EntityArchetype wolf;
// All environmental effects default to false
```

## Movement Capabilities

```cpp
archetype.canClimbWalls = false;  // Can climb like a spider
archetype.canFly = false;         // Can fly (ignores gravity)
archetype.canSwim = false;        // Can swim (moves freely in water)
```

## Custom Properties

Add game-specific data to archetypes:

```cpp
// Float properties
archetype.customFloats["mana"] = 100.0f;
archetype.customFloats["shield_strength"] = 500.0f;

// String properties
archetype.customStrings["spell_type"] = "necromancy";
archetype.customStrings["faction"] = "pirate_clan";

// Boolean properties
archetype.customBools["has_second_form"] = true;
archetype.customBools["is_boss"] = false;
```

## Registering Archetypes

```cpp
// Register with the archetype registry
vge::EntityArchetypeRegistry::GetInstance()->RegisterArchetype(archetype);

// Batch load from directory
vge::EntityArchetypeRegistry::GetInstance()->LoadFromDirectory("data/archetypes/");
```

## JSON Archetype Definitions

Archetypes can be defined in JSON files:

```json
{
  "id": "space_pirate",
  "displayName": "Space Pirate",
  "health": 75,
  "maxHealth": 75,
  "speed": 8,
  "damage": 15,
  "attackRange": 15,
  "detectionRange": 40,
  "attackCooldown": 1.5,
  "behavior": "aggressive",
  "canFly": true,
  "damagedBySunlight": false,
  "damagedByWater": true,
  "healedByDarkness": true,
  "modelId": "models/space_pirate.obj",
  "textureId": "textures/pirate_diffuse.png",
  "customFloats": {
    "shield_strength": 500,
    "rocket_damage": 100
  },
  "customStrings": {
    "faction": "pirate_clan",
    "weakness": "electricity"
  },
  "customBools": {
    "has_second_form": true
  }
}
```

## Spawning Entities

```cpp
vge::GenericEntitySpawner spawner(world, lightSystem, dayNightCycle);

// Spawn entity at position
auto* entity = spawner.SpawnEntity("space_pirate", Vec3(100, 50, 200));

// Spawn with group
spawner.SpawnGroup("space_pirate", Vec3(100, 50, 200), 5, 10.0f);

// Get spawned entities
auto pirates = spawner.GetEntitiesByArchetype("space_pirate");
for (auto* pirate : pirates) {
    pirate->controller->TakeDamage(10.0f);
}
```

## Spawn Conditions

```cpp
EntityArchetype caveTroll;
caveTroll.minLightLevel = 0;     // Only in darkness
caveTroll.maxLightLevel = 5;
caveTroll.minTimeOfDay = 0.0f;   // Any time
caveTroll.maxTimeOfDay = 1.0f;
caveTroll.spawnsOnGround = true;
caveTroll.spawnsInWater = false;
```

## AI States

Entities transition between states based on their behavior:

| State | Description |
|-------|-------------|
| `IDLE` | Standing still, looking around |
| `WANDER` | Random movement |
| `CHASE` | Pursuing target |
| `ATTACK` | In attack range, dealing damage |
| `FLEE` | Running away |
| `DEAD` | Death animation/cleanup |
| `CUSTOM_1-3` | Game-specific states |

## Events

```cpp
// Spawn event
spawner.onEntitySpawned = [](const std::string& archetypeId, const Vec3& pos) {
    std::cout << "Spawned " << archetypeId << " at (" 
              << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
};

// Death event
spawner.onEntityDied = [](const std::string& archetypeId, const Vec3& pos) {
    // Spawn loot, particles, etc.
};
```

## Complete Examples

### Fantasy RPG
```cpp
EntityArchetype darkMage;
darkMage.id = "dark_mage";
darkMage.displayName = "Dark Mage";
darkMage.health = 60;
darkMage.speed = 4;
darkMage.damage = 25;
darkMage.behavior = AIBehaviorType::Aggressive;
darkMage.attackRange = 20;  // Spell range
darkMage.detectionRange = 30;
darkMage.damagedBySunlight = true;
darkMage.healedByDarkness = true;
darkMage.customFloats["mana"] = 100;
darkMage.customStrings["spell_type"] = "necromancy";
```

### Sci-Fi Shooter
```cpp
EntityArchetype droneSwarm;
droneSwarm.id = "drone_swarm";
droneSwarm.displayName = "Drone Swarm";
droneSwarm.health = 30;
droneSwarm.speed = 15;
droneSwarm.damage = 5;
droneSwarm.behavior = AIBehaviorType::Aggressive;
droneSwarm.canFly = true;
droneSwarm.minGroupSize = 3;
droneSwarm.maxGroupSize = 8;
```

### Survival Horror
```cpp
EntityArchetype shadowStalker;
shadowStalker.id = "shadow_stalker";
shadowStalker.displayName = "Shadow Stalker";
shadowStalker.health = 100;
shadowStalker.speed = 12;
shadowStalker.damage = 20;
shadowStalker.behavior = AIBehaviorType::Fleeing;
shadowStalker.minLightLevel = 0;
shadowStalker.maxLightLevel = 3;  // Only in darkness
shadowStalker.damagedBySunlight = true;
shadowStalker.healedByDarkness = true;
```

### Peaceful Wildlife
```cpp
EntityArchetype forestDeer;
forestDeer.id = "forest_deer";
forestDeer.displayName = "Forest Deer";
forestDeer.health = 20;
forestDeer.speed = 15;
forestDeer.behavior = AIBehaviorType::Passive;
forestDeer.damagedBySunlight = false;
```

## Best Practices

1. **Use unique IDs** - Archetype IDs should be globally unique
2. **Set appropriate stats** - Balance health, speed, and damage
3. **Configure environmental effects** - Make entities react to their environment
4. **Use custom properties** - Store game-specific data in custom fields
5. **Group related archetypes** - Use consistent naming (e.g., "forest_deer", "forest_bear")
6. **Document archetypes** - Keep a reference of all archetypes and their purposes
