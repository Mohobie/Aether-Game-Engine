# Renderer Fixes

## Face Culling Fix

**Problem:** Blocks at chunk boundaries had missing faces because visibility check only looked within the same chunk.

**Solution:** Changed visibility check to use `world.GetBlock()` for all 6 neighbors, properly checking across chunk boundaries.

```cpp
// Before (broken - only checked within chunk)
bool visible = false;
if (x == 0 || chunk->GetBlock(x-1, y, z) == air) visible = true;
// ...

// After (fixed - checks world space)
bool showLeft = isTransparent(world.GetBlock(wx - 1, wy, wz));
bool showRight = isTransparent(world.GetBlock(wx + 1, wy, wz));
// ...
```

## Transparency Support

**Problem:** Transparent blocks (leaves, water) were treated as opaque, hiding faces behind them.

**Solution:** Use `BlockDef::IsOpaque()` to determine if a face should be visible through a block.

```cpp
auto isTransparent = [](BlockTypeID id) -> bool {
    if (id == BlockRegistry::GetInstance().GetBlockId("air")) return true;
    const BlockDef& def = BlockRegistry::GetInstance().GetBlock(id);
    return !def.IsOpaque();
};
```

## Files Modified
- `src/rendering/renderer.cpp` - `RenderWorld()` method

## Impact
- All block faces now render correctly at chunk boundaries
- Transparent blocks (leaves, water, glass) show faces behind them
- Performance: Only visible faces are rendered (no change)
