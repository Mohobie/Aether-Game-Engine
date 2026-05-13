# Block Textures

## Quick Reference

```cpp
vge::TextureAtlas atlas;
atlas.Load("textures/blocks.png");
atlas.AddTexture("stone", 0, 0, 16, 16);
atlas.AddTexture("dirt", 16, 0, 16, 16);
atlas.AddTexture("grass_side", 32, 0, 16, 16);
atlas.AddTexture("grass_top", 48, 0, 16, 16);

// Use in rendering
renderer.BindTexture(atlas.GetTexture());
renderer.DrawBlock(blockType, atlas.GetUV(blockType));
```

## Features

### Texture Atlas
- **Single texture:** All block textures in one image
- **UV mapping:** Each block has specific coordinates
- **16x16 default:** Standard block texture size
- **Mipmapping:** Multiple resolution levels

### Block Textures
| Block | Texture | UV Coords |
|-------|---------|-----------|
| Stone | stone | 0,0 |
| Dirt | dirt | 16,0 |
| Grass (side) | grass_side | 32,0 |
| Grass (top) | grass_top | 48,0 |
| Wood | wood | 0,16 |
| Leaves | leaves | 16,16 |
| Sand | sand | 32,16 |
| Gravel | gravel | 48,16 |

### Special Textures
- **Animated:** Water, lava, fire
- **Transparent:** Glass, leaves, water
- **Emissive:** Glowstone, torch, lava
- **Normal mapped:** For lighting detail

## Implementation

```cpp
// Load texture atlas
vge::TextureAtlas atlas;
atlas.Load("textures/blocks.png");

// Register block textures
atlas.RegisterBlockTexture("stone", "stone");
atlas.RegisterBlockTexture("dirt", "dirt");
atlas.RegisterBlockTexture("grass", "grass_side", "grass_top", "dirt");

// In renderer
void RenderBlock(BlockTypeID type, const Vec3& pos) {
    UVRect uv = atlas.GetBlockUV(type);
    renderer.DrawCube(pos, uv);
}
```

## Files
- `src/rendering/texture.h`
- `src/rendering/texture.cpp`
