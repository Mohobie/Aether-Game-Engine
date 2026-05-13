# Ore Generation

## Quick Reference

```cpp
vge::OreGenerator ores;
ores.GenerateOres(world, 5); // 5 chunk radius
```

## Features

### Ore Types
| Ore | Vein Size | Density | Veins/Chunk | Depth |
|-----|-----------|---------|-------------|-------|
| Coal | 8 | 0.7 | 20 | 5-128 |
| Iron | 6 | 0.6 | 15 | 5-64 |
| Gold | 4 | 0.5 | 4 | 5-32 |
| Diamond | 3 | 0.4 | 3 | 5-16 |
| Emerald | 3 | 0.3 | 2 | 5-32 |
| Redstone | 5 | 0.5 | 8 | 5-16 |
| Lapis | 4 | 0.5 | 3 | 5-32 |

### Generation
- **Vein shape:** Rough sphere
- **Density falloff:** Less ore at edges
- **Stone replacement:** Only replaces stone blocks
- **Depth appropriate:** Different ores at different depths

## Implementation

```cpp
vge::OreGenerator oreGen;
oreGen.GenerateOres(world, 5);
```

## Files
- `src/voxel/ore_generator.h`
- `src/voxel/ore_generator.cpp`
