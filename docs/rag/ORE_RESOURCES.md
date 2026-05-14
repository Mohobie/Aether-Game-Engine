# Ores & Resources

## Quick Reference

```cpp
vge::OreGenerator oreGen;
oreGen.GenerateOres(world, 5); // 5 chunk radius
```

## Features

### Ore Types
| Ore | Min Y | Max Y | Vein Size | Density | Veins/Chunk |
|-----|-------|-------|-----------|---------|-------------|
| Coal | 5 | 128 | 8 | 0.7 | 20 |
| Iron | 5 | 64 | 6 | 0.6 | 15 |
| Gold | 5 | 32 | 4 | 0.5 | 4 |
| Diamond | 5 | 16 | 3 | 0.4 | 3 |
| Emerald | 5 | 32 | 3 | 0.3 | 2 |
| Redstone | 5 | 16 | 5 | 0.5 | 8 |
| Lapis | 5 | 32 | 4 | 0.5 | 3 |

### Distribution
- **Coal:** Most common, all heights
- **Iron:** Common, mid-depth
- **Gold:** Rare, deep
- **Diamond:** Very rare, deepest
- **Emerald:** Rare, mountain biomes

## Implementation

```cpp
vge::OreGenerator oreGen;

// Generate all ores
oreGen.GenerateOres(world, 5);

// Or generate specific vein
oreGen.GenerateOreVein(world, position, "diamond", 3, 0.4f);
```

## Files
- `src/voxel/ore_generator.h`
- `src/voxel/ore_generator.cpp`
