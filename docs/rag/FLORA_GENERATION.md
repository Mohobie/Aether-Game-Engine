# Flora Generation

## Quick Reference

```cpp
vge::FloraGenerator flora;
flora.GenerateFlora(world, 5); // 5 chunk radius
```

## Features

### Grass
- **Placement:** On grass blocks
- **Density:** 50% of flora attempts
- **Type:** Short grass plant

### Flowers
- **Placement:** On grass blocks
- **Density:** 20% of flora attempts
- **Types:** Various colors

### Tall Grass
- **Placement:** On grass blocks
- **Density:** 15% of flora attempts
- **Height:** 1-3 blocks

### Cactus
- **Placement:** On sand blocks (desert)
- **Density:** 30% of sand attempts
- **Height:** 2-4 blocks

### Lily Pads
- **Placement:** On water blocks
- **Density:** 40% of water attempts
- **Type:** Floating plant

### Mushrooms
- **Brown Mushroom:** On grass (15% chance)
- **Red Mushroom:** In caves on stone (20% chance)
- **Placement:** Dark areas preferred

## Implementation

```cpp
vge::FloraGenerator floraGen;
floraGen.GenerateFlora(world, 5);

// Or create specific flora
floraGen.CreateGrass(world, position);
floraGen.CreateFlower(world, position, "red_flower");
floraGen.CreateCactus(world, position);
floraGen.CreateLilyPad(world, position);
floraGen.CreateMushroom(world, position, "mushroom_red");
```

## Files
- `src/voxel/flora_generator.h`
- `src/voxel/flora_generator.cpp`
