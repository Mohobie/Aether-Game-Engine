# Voxel World System

The engine uses a chunk-based voxel world similar to Minecraft. Worlds are composed of 16x16x16 chunks.

## Core Concepts

- **Block**: A single cube in the world (Stone, Dirt, Grass, etc.)
- **Chunk**: 16x16x16 collection of blocks
- **World**: Collection of chunks with terrain generation
- **BlockType**: Enum defining all block types

## Block Types

```cpp
#include "voxel/block_types.h"

// Common block types
vge::BlockType::Air      // Empty space
vge::BlockType::Stone    // Solid rock
vge::BlockType::Dirt     // Dirt block
vge::BlockType::Grass    // Grass block
vge::BlockType::Sand     // Sand block
vge::BlockType::Water    // Water block
vge::BlockType::Wood     // Tree trunk
vge::BlockType::Leaves   // Tree leaves
vge::BlockType::Bedrock  // Unbreakable bottom layer
```

## World API

```cpp
#include "voxel/world.h"

// Create and initialize world
vge::World world;
world.Initialize();

// Generate terrain at chunk coordinates
world.GenerateTerrain(chunkX, chunkZ);

// Get block at world position
vge::BlockType block = world.GetBlock(x, y, z);
vge::BlockType block = world.GetBlock(vge::Vec3(10, 5, 10));

// Set block at world position
world.SetBlock(x, y, z, vge::BlockType::Stone);
world.SetBlock(vge::Vec3(10, 5, 10), vge::BlockType::Wood);

// Remove block (set to Air)
world.SetBlock(x, y, z, vge::BlockType::Air);

// Check if block is solid
bool solid = world.IsBlockSolid(x, y, z);

// Update world (process chunk loading/unloading)
world.Update(deltaTime);
```

## Chunk API

```cpp
#include "voxel/chunk.h"

// Get chunk at chunk coordinates
vge::Chunk* chunk = world.GetChunk(chunkX, chunkZ);

// Get block within chunk (local coordinates 0-15)
vge::BlockType block = chunk->GetBlock(localX, localY, localZ);

// Set block within chunk
chunk->SetBlock(localX, localY, localZ, vge::BlockType::Stone);

// Mark chunk for mesh rebuild
chunk->MarkDirty();

// Check if chunk is loaded
bool loaded = chunk->IsLoaded();
```

## Terrain Generation

```cpp
#include "voxel/world_generator.h"

// Create generator
vge::WorldGenerator generator;
generator.SetSeed(12345);  // Random seed

// Generate heightmap for chunk
std::vector<float> heightmap = generator.GenerateHeightmap(chunkX, chunkZ);

// Custom generation
class CustomGenerator : public vge::WorldGenerator {
protected:
    void GenerateChunk(Chunk* chunk, int cx, int cz) override {
        // Custom terrain logic
        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                int height = GetHeightAt(cx * 16 + x, cz * 16 + z);
                for (int y = 0; y < height; ++y) {
                    if (y < height - 3) {
                        chunk->SetBlock(x, y, z, BlockType::Stone);
                    } else if (y < height - 1) {
                        chunk->SetBlock(x, y, z, BlockType::Dirt);
                    } else {
                        chunk->SetBlock(x, y, z, BlockType::Grass);
                    }
                }
            }
        }
    }
};
```

## Raycasting

```cpp
#include "core/raycast.h"

// Create ray from origin in direction
vge::Ray ray(origin, direction);

// Raycast against world
vge::RaycastHit hit;
if (world.Raycast(ray, maxDistance, hit)) {
    // hit.position - world position of hit
    // hit.normal - surface normal
    // hit.blockPos - integer block coordinates
    // hit.blockType - type of block hit
    
    // Place block adjacent to hit
    vge::Vec3 placePos = hit.position + hit.normal;
    world.SetBlock(placePos, vge::BlockType::Stone);
    
    // Remove hit block
    world.SetBlock(hit.position, vge::BlockType::Air);
}
```

## Block Picker (Player Interaction)

```cpp
#include "voxel/block_picker.h"

// Create picker for player
vge::BlockPicker picker;
picker.SetReachDistance(15.0f);  // How far player can reach

// Update with camera position and direction
picker.Update(cameraPosition, cameraForward);

// Try to pick block
if (picker.Pick(world)) {
    // Get selected block
    vge::Vec3 selectedPos = picker.GetSelectedBlock();
    
    // Get adjacent position for placing
    vge::Vec3 placePos = picker.GetPlacePosition();
    
    // Place block
    world.SetBlock(placePos, vge::BlockType::Wood);
    
    // Remove block
    world.SetBlock(selectedPos, vge::BlockType::Air);
}
```

## World Streaming

```cpp
// Load chunks around player
vge::ChunkManager chunkManager;

void UpdateWorldStreaming(vge::Vec3 playerPos) {
    // Convert player position to chunk coordinates
    int playerChunkX = floor(playerPos.x / 16);
    int playerChunkZ = floor(playerPos.z / 16);
    
    // Load chunks within render distance
    int renderDistance = 8;  // chunks
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

## Save/Load World

```cpp
#include "core/save_system.h"

// Save world
vge::SaveSystem saveSystem;
saveSystem.SaveWorld(world, "world_save.dat");

// Load world
saveSystem.LoadWorld(world, "world_save.dat");

// Auto-save
float autoSaveInterval = 300.0f;  // 5 minutes
float timeSinceSave = 0.0f;

void Update(float dt) {
    timeSinceSave += dt;
    if (timeSinceSave >= autoSaveInterval) {
        saveSystem.SaveWorld(world, "world_save.dat");
        timeSinceSave = 0.0f;
    }
}
```
