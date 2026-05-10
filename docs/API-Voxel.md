# Voxel Module API
**Files:** src/voxel/biome.h, src/voxel/block.h, src/voxel/block_mesh_builder.h, src/voxel/block_registry.h, src/voxel/chunk.h, src/voxel/chunk_manager.h, src/voxel/world.h, src/voxel/world_generator.h, src/voxel/world_renderer.h

## `voxel/biome.h`
```cpp
namespace vge {
```

### `enum class BiomeType`
| Value | Description |
|-------|-------------|
| `Plains` | |
| `Forest` | |
| `Desert` | |
| `Snow` | |

### `class BiomeType`

### `class Biome`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `GetBiomeType` | `static BiomeType` | `float temperature, float humidity` |
| `GetBaseHeight` | `static float` | `BiomeType type` |
| `GetHeightVariation` | `static float` | `BiomeType type` |
| `GetSurfaceBlock` | `static BlockType` | `BiomeType type` |
| `GetSubsurfaceBlock` | `static BlockType` | `BiomeType type` |
| `GenerateChunkColumn` | `static void` | `Chunk* chunk, int x, int z, float worldX, float worldZ, int seed` |

## `voxel/block.h`
```cpp
namespace vge {
```

### `enum class BlockType : uint16_t`
| Value | Description |
|-------|-------------|
| `Air` | |
| `Stone` | |
| `Dirt` | |
| `Grass` | |
| `Sand` | |
| `Water` | |
| `Wood` | |
| `Leaves` | |
| `Glass` | |
| `Planks` | |
| `CraftingTable` | |
| `Furnace` | |
| `Torch` | |
| `Glowstone` | |
| `Bedrock` | |
| `CoalOre` | |
| `IronOre` | |
| `GoldOre` | |
| `DiamondOre` | |
| `Snow` | |
| `Ice` | |
| `Cactus` | |
| `Flower` | |
| `TallGrass` | |
| `Lava` | |
| `Stick` | |
| `Count` | |

### `struct Block`
| Member | Type |
|--------|------|
| `type` | `BlockType` |
| `metadata` | `uint8_t` |
| `light_level` | `uint8_t` |

### `class BlockType`

## `voxel/block_mesh_builder.h`
```cpp
namespace vge {
```

### `class BlockMeshBuilder`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `BuildChunkMesh` | `static void` | `const Chunk* chunk, Mesh& mesh` |
| `AddCube` | `static void` | `Mesh& mesh, const Vec3& position, const Vec3& color` |
| `AddFace` | `static void` | `Mesh& mesh, const Vec3& position, const Vec3& normal, const Vec3& color` |
| `GetBlockColor` | `static Vec3` | `int type` |

## `voxel/block_registry.h`
```cpp
namespace vge {
```

### `struct BlockInfo`
| Member | Type |
|--------|------|
| `type` | `BlockType` |
| `solid` | `bool` |
| `opaque` | `bool` |
| `hardness` | `float` |
| `color` | `Vec3` |
| `emission` | `int` |

### `class BlockRegistry`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `RegisterBlock` | `void` | `BlockType type, const char* name, bool solid, bool opaque, float hardness, Vec3 color` |

## `voxel/chunk.h`
```cpp
namespace vge {
```

### `struct Chunk`
| Member | Type |
|--------|------|
| `dirty` | `bool` |
| `loaded` | `bool` |
| `modified` | `bool` |

## `voxel/chunk_manager.h`
```cpp
namespace vge {
```

### `class ChunkManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `void` | `World* world, WorldGenerator* generator` |
| `Update` | `void` | `const Vec3& playerPosition` |
| `LoadChunk` | `Chunk*` | `int x, int y, int z` |
| `UnloadChunk` | `void` | `int x, int y, int z` |
| `UnloadDistantChunks` | `void` | `int centerX, int centerY, int centerZ, int radius` |
| `Clear` | `void` | `` |

## `voxel/world.h`
```cpp
namespace vge {
```

### `struct ChunkCoord`

### `class World`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Clear` | `void` | `` |
| `GetChunk` | `Chunk*` | `int x, int y, int z` |
| `GetOrCreateChunk` | `Chunk*` | `int x, int y, int z` |
| `GetBlock` | `BlockType` | `int x, int y, int z` |
| `SetBlock` | `void` | `int x, int y, int z, BlockType type` |

## `voxel/world_generator.h`
```cpp
namespace vge {
```

### `class WorldGenerator`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `GenerateChunk` | `void` | `Chunk& chunk, int cx, int cy, int cz` |
| `SetSeed` | `void` | `unsigned int seed` |
| `GetNoise` | `float` | `float x, float y, float z` |
| `GetHeightNoise` | `float` | `float x, float z` |
| `GenerateTree` | `void` | `Chunk& chunk, int x, int y, int z` |

## `voxel/world_renderer.h`
```cpp
namespace vge {
```

### `class WorldRenderer`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `void` | `Shader* shaderProgram` |
| `RenderChunk` | `void` | `const Chunk* chunk, const Camera& camera` |
| `RenderWorld` | `void` | `const World& world, const Camera& camera` |
| `UpdateChunkMesh` | `void` | `const Chunk* chunk` |
| `Cleanup` | `void` | `` |
