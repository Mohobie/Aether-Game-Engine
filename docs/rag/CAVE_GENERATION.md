# Cave Generation

## Quick Reference

```cpp
vge::CaveGenerator caves;

// Generate caves in a chunk radius
caves.GenerateCaves(world, 5); // 5 chunk radius

// Generate a single tunnel
caves.GenerateCaveTunnel(world, start, end, 2.0f);

// Generate a cave system
caves.GenerateCaveSystem(world, startPos, 5, 20.0f);
```

## Features

### Tunnel Generation
- **Wandering paths:** Natural-looking curves
- **Variable radius:** 1.5-3.0 blocks
- **Branching:** 30% chance of branches
- **Sphere carving:** Smooth cave walls

### Cave Systems
- **Multiple tunnels:** 3-8 per system
- **Random starting points:** Underground
- **Depth range:** Y=5 to Y=30
- **Branching tunnels:** Smaller offshoots

## Implementation

```cpp
// Generate in world
vge::CaveGenerator caveGen;
caveGen.GenerateCaves(world, 5);

// Or manual tunnel
Vec3 start(10, 15, 10);
Vec3 end(30, 12, 40);
caveGen.GenerateCaveTunnel(world, start, end, 2.5f);
```

## Algorithm

1. **Start Point:** Random underground position
2. **End Point:** Random direction, limited length
3. **Step:** Move in direction with wandering
4. **Carve:** Sphere at each step
5. **Branch:** 30% chance for smaller tunnel

## Files
- `src/voxel/cave_generator.h`
- `src/voxel/cave_generator.cpp`
