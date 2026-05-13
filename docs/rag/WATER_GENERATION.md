# Water Generation

## Quick Reference

```cpp
vge::WaterGenerator water;

// Generate lakes
water.GenerateLakes(world, 5, 3); // 5 chunk radius, 3 lakes

// Generate rivers
water.GenerateRivers(world, 5, 2); // 5 chunk radius, 2 rivers

// Create specific lake
water.CreateLake(world, center, 10.0f, 3); // radius 10, depth 3

// Create specific river
water.CreateRiver(world, start, end, 3.0f); // width 3
```

## Features

### Lakes
- **Shape:** Circular with depth falloff
- **Bottom:** Sand
- **Sides:** Dirt
- **Fill:** Water blocks up to level

### Rivers
- **Path:** Wandering from start to end
- **Width:** Configurable
- **Gradient:** Slightly downhill
- **Banks:** Sand and dirt

## Implementation

```cpp
vge::WaterGenerator waterGen;

// Generate lakes and rivers
waterGen.GenerateLakes(world, 5, 5);
waterGen.GenerateRivers(world, 5, 3);
```

## Files
- `src/voxel/water_generator.h`
- `src/voxel/water_generator.cpp`
