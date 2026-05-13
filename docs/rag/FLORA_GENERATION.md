# Flora Generation

## Quick Reference

```cpp
vge::FloraGenerator flora;
flora.GenerateFlora(world, 5); // 5 chunk radius
```

## Features

### Grass
- **Placement:** On grass blocks
- **Density:** 60% of flora attempts
- **Type:** Short grass plant

### Flowers
- **Placement:** On grass blocks
- **Density:** 20% of flora attempts
- **Types:** Various colors

### Tall Grass
- **Placement:** On grass blocks
- **Density:** 20% of flora attempts
- **Height:** 1-3 blocks

### Cactus
- **Placement:** On sand blocks (desert)
- **Density:** 30% of sand attempts
- **Height:** 2-4 blocks

## Implementation

```cpp
vge::FloraGenerator floraGen;
floraGen.GenerateFlora(world, 5);

// Or create specific flora
floraGen.CreateGrass(world, position);
floraGen.CreateFlower(world, position, "red_flower");
floraGen.CreateCactus(world, position);
```

## Files
- `src/voxel/flora_generator.h`
- `src/voxel/flora_generator.cpp`
