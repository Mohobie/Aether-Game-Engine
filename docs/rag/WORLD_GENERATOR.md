# World Generator

## Quick Reference

```cpp
// Generate flat world
vge::WorldGenerator::GenerateFlatWorld(world, 50); // 50 = radius in chunks

// Generate hilly terrain
vge::WorldGenerator::GenerateHillyWorld(world, 50);

// Generate forest
vge::WorldGenerator::GenerateForestWorld(world, 50);

// Generate desert
vge::WorldGenerator::GenerateDesertWorld(world, 50);
```

## Generation Types

### Flat World
- Flat grass terrain at y=10
- Trees scattered randomly
- Bedrock at y=0
- Good for creative building

### Hilly World
- Perlin noise-based terrain
- Hills and valleys
- Caves underground
- Stone/dirt/grass layers
- Most common survival world

### Forest World
- Dense tree coverage
- Water bodies
- Mixed terrain
- Good for wood gathering

### Desert World
- Sand dunes
- Cactus generation
- Minimal water
- Flat areas for building

## Implementation

```cpp
// Set world seed first
world.SetSeed(12345);

// Generate terrain
vge::WorldGenerator::GenerateHillyWorld(world, 50);

// Add ores after terrain
vge::OreGenerator oreGen;
// ... configure ores ...
oreGen.GenerateOresInWorld(world, 50);
```

## Files
- `src/voxel/world_generator.h`
- `src/voxel/world_generator.cpp`
