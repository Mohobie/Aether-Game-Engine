# Mineshaft Generation System

## Overview

The Mineshaft Generation System creates underground mineshaft structures throughout the world. These structures add exploration depth and danger to cave systems, featuring wooden supports, rail tracks, loot chests, and cave spider spawners.

## Files

- `src/voxel/mineshaft_generator.h` — Header with class declaration
- `src/voxel/mineshaft_generator.cpp` — Implementation

## Class: MineshaftGenerator

### Public Methods

| Method | Description |
|--------|-------------|
| `GenerateMineshafts(World& world, int chunkRadius)` | Main entry point — generates mineshafts across all chunks in radius |
| `GenerateMineshaftSystem(World& world, const Vec3& start, int numSegments)` | Generates a complete mineshaft system from a starting point |
| `GenerateCorridor(World& world, const Vec3& start, const Vec3& end, float width, float height)` | Carves a corridor between two points |
| `GenerateSupports(World& world, const Vec3& start, const Vec3& end, float width, float height)` | Places wooden support beams |
| `GenerateRails(World& world, const Vec3& start, const Vec3& end)` | Places rail tracks along a corridor |
| `PlaceLootChest(World& world, int x, int y, int z)` | Places a chest at the given position |
| `PlaceCaveSpiderSpawner(World& world, int x, int y, int z)` | Places a mob spawner |
| `IsValidMineshaftPosition(World& world, const Vec3& pos)` | Checks if a position is suitable for mineshaft generation |

### Private Methods

| Method | Description |
|--------|-------------|
| `GenerateRoom(World& world, const Vec3& center, int width, int height, int depth)` | Creates a room/hub area |
| `GenerateLadderShaft(World& world, const Vec3& top, int depth)` | Creates a vertical ladder shaft |
| `IsUndergroundBlock(World& world, int x, int y, int z)` | Checks if a block is a valid underground block |

## Mineshaft Features

### 1. Corridors
- Width: 2.0–3.5 blocks
- Height: 2.5–3.5 blocks
- Carved through stone, dirt, and gravel
- Air, water, lava, and bedrock are preserved

### 2. Wooden Supports
- Placed every 4–6 blocks along corridors
- Corner posts (2 blocks tall)
- Ceiling crossbeams
- 80% chance per corridor segment

### 3. Rail Tracks
- Placed on corridor floors
- 60% chance per corridor segment
- Follows corridor path

### 4. Rooms/Hubs
- 20% chance at corridor junctions
- Size: 4–7 blocks wide/deep, 3–5 blocks tall
- Wooden support framing
- 50% chance to contain a loot chest

### 5. Loot Chests
- Placed in rooms and occasionally along corridors
- Uses existing `chest` block type

### 6. Cave Spider Spawners
- 15% chance per corridor segment
- Uses existing `mob_spawner` block type
- Adds danger to exploration

### 7. Ladder Shafts
- 10% chance — vertical connections between levels
- 2×2 shaft with ladder blocks
- Depth: 5–10 blocks

### 8. Branching
- 25% chance for side corridors
- Shorter segments (5–12 blocks)
- May have rails and supports

## Generation Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| Spawn chance | 15% per chunk | Probability of mineshaft in a chunk |
| Y range | 8–40 | Underground depth range |
| Segment count | 3–8 | Number of corridors per mineshaft |
| Segment length | 8–20 blocks | Length of each corridor |
| Branch chance | 25% | Probability of side corridors |

## Integration with Cave Generation

Mineshafts are designed to integrate naturally with existing cave systems:
- Same Y-depth range as caves (8–40)
- Only carves through solid underground blocks
- Preserves air pockets (existing caves)
- Can intersect with cave tunnels naturally

## Usage Example

```cpp
#include "voxel/mineshaft_generator.h"

// In world generation:
vge::MineshaftGenerator mineshaftGen;
mineshaftGen.GenerateMineshafts(world, chunkRadius);
```

## Block Types Used

| Block | Purpose |
|-------|---------|
| `wood` | Support beams, posts |
| `rail` | Minecart tracks |
| `chest` | Loot containers |
| `mob_spawner` | Cave spider spawners |
| `ladder` | Vertical shafts |
| `air` | Carved corridors |

## Notes

- Uses deterministic random seeding based on system time
- Supports chunk-based generation for large worlds
- Corridors follow natural wandering paths with slight vertical variation
- Rooms provide rest points and loot opportunities
- Cave spider spawners add combat challenge
