# Ore Generation

## Quick Reference

```cpp
vge::OreGenerator oreGen;

// Define an ore type
vge::OreType coalOre;
coalOre.blockId = "coal_ore";
coalOre.blockType = registry.GetBlockId("coal_ore");
coalOre.rarity = 0.02f;
coalOre.minHeight = 5;
coalOre.maxHeight = 60;
coalOre.veinSize = 12;
coalOre.veinsPerChunk = 3;

// Register and generate
oreGen.RegisterOre(coalOre);
oreGen.GenerateOresInWorld(world, 50); // 50 chunk radius
```

## Ore Types

| Ore | Color | Height | Rarity | Vein Size | Tool Required |
|-----|-------|--------|--------|-----------|---------------|
| Coal | Gray-black | 5-60 | Common | 12 | Wood Pickaxe+ |
| Iron | Brown | 5-45 | Uncommon | 8 | Stone Pickaxe+ |
| Gold | Yellow | 5-30 | Rare | 6 | Iron Pickaxe+ |
| Diamond | Cyan | 5-16 | Very Rare | 4 | Iron Pickaxe+ |
| Emerald | Green | 5-16 | Very Rare | 4 | Iron Pickaxe+ |

## Generation Algorithm

1. **Per-Chunk:** Each chunk gets random veins based on `veinsPerChunk`
2. **Vein Size:** Random walk from start point, placing ore blocks
3. **Height Distribution:** Ores only spawn within their height range
4. **Replacement:** Only replaces stone blocks (not dirt, grass, etc.)

## Configuration

```cpp
// Create custom ore
vge::OreType customOre;
customOre.blockId = "my_ore";
customOre.blockType = registry.GetBlockId("my_ore");
customOre.rarity = 0.01f;        // 1% chance per chunk
customOre.minHeight = 10;        // Minimum Y level
customOre.maxHeight = 40;        // Maximum Y level
customOre.veinSize = 8;          // Average vein size
customOre.veinsPerChunk = 2;     // Average veins per chunk

oreGen.RegisterOre(customOre);
```

## Files
- `src/voxel/ore_generator.h`
- `src/voxel/ore_generator.cpp`
