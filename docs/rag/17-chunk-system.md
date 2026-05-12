# Chunk System

The engine uses a chunk-based architecture for efficient voxel world management.

## Chunk Structure

Chunks are 16x16x16 blocks. Worlds are composed of chunks.

```cpp
#include "voxel/chunk.h"

// Chunk dimensions
constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; // 4096 blocks

// Create chunk
vge::Chunk chunk(chunkX, chunkY, chunkZ);
```

## Block Storage

Each chunk stores block data efficiently:

```cpp
// Set block in chunk (local coordinates 0-15)
chunk.SetBlock(localX, localY, localZ, vge::BlockType::Stone);

// Get block
vge::BlockType block = chunk.GetBlock(localX, localY, localZ);

// Check if block is solid
bool solid = chunk.IsBlockSolid(localX, localY, localZ);

// Check if block is transparent
bool transparent = chunk.IsBlockTransparent(localX, localY, localZ);
```

## Chunk Manager

The ChunkManager handles loading, unloading, and saving chunks:

```cpp
#include "voxel/chunk_manager.h"

vge::ChunkManager chunkManager;
chunkManager.Initialize(world, generator);

// Set view distance (in chunks)
chunkManager.SetViewDistance(8);

// Update (call every frame)
chunkManager.Update(playerPosition);

// Check if chunk is loaded
bool loaded = chunkManager.IsChunkLoaded(chunkX, chunkZ);

// Load chunk manually
vge::Chunk* chunk = chunkManager.LoadChunk(chunkX, chunkZ);

// Unload chunk
chunkManager.UnloadChunk(chunkX, chunkZ);

// Unload distant chunks
chunkManager.UnloadDistantChunks(playerChunkX, playerChunkZ, renderDistance + 2);
```

## Chunk Persistence

### Saving Chunks

```cpp
// Save single chunk
vge::SaveSystem::SaveChunk(chunk, fileStream);

// Save all modified chunks
chunkManager.SaveAllChunks("world_save/");

// Auto-save
float autoSaveInterval = 300.0f; // 5 minutes
float timeSinceSave = 0.0f;

void Update(float dt) {
    timeSinceSave += dt;
    if (timeSinceSave >= autoSaveInterval) {
        chunkManager.SaveAllChunks("world_save/");
        timeSinceSave = 0.0f;
    }
}
```

### Loading Chunks

```cpp
// Load single chunk
vge::Chunk* chunk = vge::SaveSystem::LoadChunk(chunkX, chunkZ, fileStream);

// Load from directory
chunkManager.LoadFromDirectory("world_save/");
```

### Save Format

Chunks are saved in a binary format:

```cpp
struct SaveHeader {
    uint32_t magic;           // "VOXE" = 0x564F5845
    uint32_t version;         // Format version
    uint32_t chunkCount;      // Number of chunks
    uint32_t blockPaletteSize; // Number of unique block types
};

struct BlockPaletteEntry {
    char blockId[64];         // String ID of block
    uint16_t typeId;          // Runtime type ID
};

struct ChunkData {
    int32_t chunkX, chunkY, chunkZ;
    uint32_t blockDataSize;
    // Compressed block data follows
};
```

## World Streaming

```cpp
// Stream chunks around player
void UpdateWorldStreaming(vge::Vec3 playerPos) {
    // Convert player position to chunk coordinates
    int playerChunkX = floor(playerPos.x / CHUNK_SIZE);
    int playerChunkZ = floor(playerPos.z / CHUNK_SIZE);
    
    // Load chunks within render distance
    int renderDistance = 8;
    for (int dx = -renderDistance; dx <= renderDistance; ++dx) {
        for (int dz = -renderDistance; dz <= renderDistance; ++dz) {
            int cx = playerChunkX + dx;
            int cz = playerChunkZ + dz;
            
            if (!chunkManager.IsChunkLoaded(cx, cz)) {
                chunkManager.LoadChunk(cx, cz);
            }
        }
    }
    
    // Unload distant chunks
    chunkManager.UnloadDistantChunks(playerChunkX, playerChunkZ, renderDistance + 2);
}
```

## Chunk Mesh Generation

```cpp
// Mark chunk for mesh rebuild
chunk.MarkDirty();

// Rebuild mesh (call after batch modifications)
chunkManager.RebuildMesh(chunk);

// Update chunk mesh with neighbors (for proper culling)
chunkManager.UpdateChunkMeshWithNeighbors(chunk);
```

## Chunk Culling

```cpp
#include "rendering/culling_system.h"

vge::CullingSystem culling;
culling.SetCamera(camera.GetPosition(), camera.GetViewProj());

// Update chunk BVH
std::vector<vge::ChunkBVH::BuildPrimitive> chunks;
// ... fill chunks from world
culling.updateChunkBVH(chunks);

// Cull chunks
vge::CullingResult result = culling.cullChunks();

// Render only visible chunks
for (uint32_t chunkId : result.visibleChunks) {
    int lod = result.chunkLOD[chunkId];
    RenderChunk(chunkId, lod);
}
```

## LOD (Level of Detail)

```cpp
// Set LOD distances
world.SetLODDistance(0, 32);   // Full detail within 32 blocks
world.SetLODDistance(1, 64);   // Half detail within 64 blocks
world.SetLODDistance(2, 128);  // Quarter detail within 128 blocks

// Render with LOD
void RenderChunk(int chunkId, int lod) {
    switch (lod) {
        case 0: RenderFullDetail(chunkId); break;
        case 1: RenderHalfDetail(chunkId); break;
        case 2: RenderQuarterDetail(chunkId); break;
    }
}
```

## Chunk Statistics

```cpp
// Get chunk stats
int loadedChunks = chunkManager.GetLoadedChunkCount();
int renderedChunks = chunkManager.GetRenderedChunkCount();

// Memory usage
size_t chunkMemory = loadedChunks * sizeof(vge::Chunk);
```

## Best Practices

1. **Load asynchronously** - Don't block main thread when loading chunks
2. **Use view distance** - Adjust based on performance
3. **Save incrementally** - Only save modified chunks
4. **Use LOD** - Reduce detail for distant chunks
5. **Cull aggressively** - Don't render chunks outside frustum
6. **Pool chunks** - Reuse chunk objects to reduce allocations
7. **Compress saves** - Use run-length encoding for empty areas
