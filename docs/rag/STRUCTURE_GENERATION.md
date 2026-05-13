# Structure Generation

## Quick Reference

```cpp
vge::StructureGenerator structures;

// Generate trees
structures.GenerateTrees(world, 5, 2); // 5 chunk radius, 2 trees per chunk

// Create specific structures
structures.CreateTree(world, position);
structures.CreateHouse(world, position);
structures.CreateDungeon(world, position);
```

## Features

### Trees
- **Height:** 4-6 blocks
- **Trunk:** Wood blocks
- **Leaves:** Leaf blocks in canopy
- **Placement:** On ground surface

### Houses
- **Size:** 5x5x4
- **Walls:** Planks
- **Floor:** Planks
- **Roof:** Wood
- **Door:** Opening at front
- **Windows:** Glass blocks

### Dungeons
- **Size:** 7x7x4
- **Walls/Floor/Ceiling:** Cobblestone
- **Mob Spawner:** In center
- **Chests:** In corners

## Implementation

```cpp
vge::StructureGenerator structures;

// Generate trees across world
structures.GenerateTrees(world, 5, 3);

// Create a house
structures.CreateHouse(world, vge::Vec3(10, 10, 10));

// Create a dungeon underground
structures.CreateDungeon(world, vge::Vec3(20, 5, 20));
```

## Files
- `src/voxel/structure_generator.h`
- `src/voxel/structure_generator.cpp`
