# Chunk System

## Quick Reference

```cpp
// Get or create chunk
vge::Chunk* chunk = world.GetOrCreateChunk(cx, cy, cz);

// Check if loaded
if (chunk && chunk->loaded) {
    // Use chunk
}

// Get block at world position
BlockTypeID block = world.GetBlock(x, y, z);

// Set block
world.SetBlock(x, y, z, blockType);
```

## Chunk Properties

- **Size:** 16x16x16 blocks
- **Coordinates:** Chunk-space (world position / 16)
- **Loaded Flag:** Must be set for chunk to render
- **Dirty Flag:** Set when modified, cleared when saved

## Fixes Applied

### Chunk Loading Fix (2026-05-12)
**Problem:** `chunk->loaded` flag not set when chunks created
**Solution:** Set `loaded = true` in `GetOrCreateChunk()`

### Chunk Manager Fix (2026-05-12)
**Problem:** Missing `GenerateChunk()` method caused build errors
**Solution:** Use loaded flag check instead of generation method

## World Accessors

```cpp
// Get chunk by chunk coordinates
Chunk* GetChunk(int cx, int cy, int cz);
Chunk* GetOrCreateChunk(int cx, int cy, int cz);

// Get block by world coordinates
BlockTypeID GetBlock(int x, int y, int z);
void SetBlock(int x, int y, int z, BlockTypeID type);

// World properties
int GetSeed() const;
void SetSeed(int seed);
```

## Files
- `src/voxel/chunk.h`
- `src/voxel/chunk.cpp`
- `src/voxel/world.h`
- `src/voxel/world.cpp`
- `src/voxel/chunk_manager.cpp`
