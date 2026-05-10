# Voxel Module API

**Files:** `voxel/block.h`, `voxel/block_registry.h`, `voxel/chunk.h`, `voxel/world.h`, `voxel/world_generator.h`, `voxel/world_renderer.h`, `voxel/biome.h`, `voxel/block_mesh_builder.h`, `voxel/chunk_manager.h`

---

## `voxel/block.h`

```cpp
namespace vge {
```

### `enum class BlockType : uint16_t`
| Value | Description |
|-------|-------------|
| `Air` | Empty space |
| `Stone` | Stone block |
| `Dirt` | Dirt block |
| `Grass` | Grass block |
| `Sand` | Sand block |
| `Water` | Water block |
| `Wood` | Wood block |
| `Leaves` | Leaves block |
| `Glass` | Glass block |
| `Planks` | Wooden planks |
| `CraftingTable` | Crafting table |
| `Furnace` | Furnace |
| `Torch` | Torch |
| `Glowstone` | Glowstone |
| `Bedrock` | Bedrock |
| `CoalOre` | Coal ore |
| `IronOre` | Iron ore |
| `GoldOre` | Gold ore |
| `DiamondOre` | Diamond ore |
| `Snow` | Snow block |
| `Ice` | Ice block |
| `Cactus` | Cactus |
| `Flower` | Flower |

### `struct Block`
| Member | Type | Description |
|--------|------|-------------|
| `type` | `BlockType` | Block type |
| `metadata` | `uint8_t` | Block metadata |

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `IsSolid` | `bool` | `void` | Is solid block |
| `IsTransparent` | `bool` | `void` | Is transparent |
| `IsLightSource` | `bool` | `void` | Emits light |
| `GetLightLevel` | `uint8_t` | `void` | Light level (0-15) |
| `GetHardness` | `float` | `void` | Mining hardness |
| `GetColor` | `Vec3` | `void` | Block color |

---

## `voxel/block_registry.h`

### `struct BlockProperties`
| Member | Type | Description |
|--------|------|-------------|
| `type` | `BlockType` | Block type |
| `name` | `std::string` | Display name |
| `solid` | `bool` | Is solid |
| `transparent` | `bool` | Is transparent |
| `lightLevel` | `uint8_t` | Light emission |
| `hardness` | `float` | Mining hardness |
| `color` | `Vec3` | Block color |
| `textureTop` | `std::string` | Top texture |
| `textureSide` | `std::string` | Side texture |
| `textureBottom` | `std::string` | Bottom texture |

### `class BlockRegistry`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `BlockRegistry&` | `void` | Singleton |
| `Register` | `void` | `const BlockProperties& props` | Register block |
| `Get` | `const BlockProperties*` | `BlockType type` | Get properties |
| `GetName` | `std::string` | `BlockType type` | Get name |
| `IsSolid` | `bool` | `BlockType type` | Check solid |
| `IsTransparent` | `bool` | `BlockType type` | Check transparent |
| `GetLightLevel` | `uint8_t` | `BlockType type` | Get light |
| `GetHardness` | `float` | `BlockType type` | Get hardness |
| `GetColor` | `Vec3` | `BlockType type` | Get color |
| `GetTextureTop` | `std::string` | `BlockType type` | Get top texture |
| `GetTextureSide` | `std::string` | `BlockType type` | Get side texture |
| `GetTextureBottom` | `std::string` | `BlockType type` | Get bottom texture |
| `GetAllTypes` | `std::vector<BlockType>` | `void` | All registered types |
| `LoadFromFile` | `void` | `const std::string& path` | Load registry |
| `SaveToFile` | `void` | `const std::string& path` | Save registry |

---

## `voxel/chunk.h`

### `struct ChunkCoord`
| Member | Type | Description |
|--------|------|-------------|
| `x` | `int32_t` | Chunk X coordinate |
| `z` | `int32_t` | Chunk Z coordinate |

### `class Chunk`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Chunk` | (ctor) | `const ChunkCoord& coord` | Constructor |
| `GetCoord` | `ChunkCoord` | `void` | Get coordinates |
| `GetBlock` | `Block` | `int x, int y, int z` | Get block at local pos |
| `SetBlock` | `void` | `int x, int y, int z, BlockType type` | Set block |
| `RemoveBlock` | `void` | `int x, int y, int z` | Remove block (Air) |
| `IsBlockSolid` | `bool` | `int x, int y, int z` | Check solid |
| `IsBlockTransparent` | `bool` | `int x, int y, int z` | Check transparent |
| `GetHighestBlock` | `int` | `int x, int z` | Get highest Y |
| `IsEmpty` | `bool` | `void` | Check empty |
| `IsModified` | `bool` | `void` | Check modified |
| `SetModified` | `void` | `bool modified` | Set modified |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `void` | `const std::string& data` | Deserialize |
| `GetBlockCount` | `size_t` | `void` | Non-air block count |

---

## `voxel/world.h`

### `class World`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `World` | (ctor) | `const std::string& name = "world"` | Constructor |
| `GetName` | `std::string` | `void` | Get world name |
| `GetBlock` | `Block` | `int x, int y, int z` | Get block at world pos |
| `SetBlock` | `void` | `int x, int y, int z, BlockType type` | Set block |
| `RemoveBlock` | `void` | `int x, int y, int z` | Remove block |
| `GetChunk` | `Chunk*` | `const ChunkCoord& coord` | Get chunk |
| `GetOrCreateChunk` | `Chunk*` | `const ChunkCoord& coord` | Get or create |
| `RemoveChunk` | `void` | `const ChunkCoord& coord` | Remove chunk |
| `ChunkExists` | `bool` | `const ChunkCoord& coord` | Check exists |
| `GetLoadedChunks` | `std::vector<ChunkCoord>` | `void` | All loaded |
| `Update` | `void` | `void` | Update world |
| `GetSeed` | `uint32_t` | `void` | Get world seed |
| `SetSeed` | `void` | `uint32_t seed` | Set seed |
| `GetTime` | `float` | `void` | Get time of day |
| `SetTime` | `void` | `float time` | Set time |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `void` | `const std::string& data` | Deserialize |
| `GetBlockLight` | `uint8_t` | `int x, int y, int z` | Get block light |
| `SetBlockLight` | `void` | `int x, int y, int z, uint8_t level` | Set block light |
| `GetSkyLight` | `uint8_t` | `int x, int y, int z` | Get sky light |
| `SetSkyLight` | `void` | `int x, int y, int z, uint8_t level` | Set sky light |

---

## `voxel/world_generator.h`

### `enum class BiomeType`
| Value | Description |
|-------|-------------|
| `Plains` | Flat grassland |
| `Forest` | Tree-covered |
| `Desert` | Sand dunes |
| `Mountains` | High terrain |
| `Ocean` | Deep water |
| `River` | Shallow water |
| `Snow` | Snow-covered |
| `Jungle` | Dense vegetation |
| `Swamp` | Wetlands |
| `Savanna` | Dry grassland |

### `class WorldGenerator`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `WorldGenerator` | (ctor) | `uint32_t seed = 0` | Constructor |
| `SetSeed` | `void` | `uint32_t seed` | Set seed |
| `GenerateChunk` | `void` | `Chunk& chunk` | Generate chunk |
| `GenerateTerrain` | `void` | `Chunk& chunk` | Generate terrain |
| `GenerateCaves` | `void` | `Chunk& chunk` | Generate caves |
| `GenerateOres` | `void` | `Chunk& chunk` | Generate ores |
| `GenerateStructures` | `void` | `Chunk& chunk` | Generate structures |
| `GetBiomeAt` | `BiomeType` | `int x, int z` | Get biome |
| `GetHeightAt` | `int` | `int x, int z` | Get ground height |
| `SetBiomeScale` | `void` | `float scale` | Set biome scale |
| `SetHeightScale` | `void` | `float scale` | Set height scale |
| `SetCaveDensity` | `void` | `float density` | Set cave density |
| `SetOreDensity` | `void` | `float density` | Set ore density |

---

## `voxel/world_renderer.h`

### `class WorldRenderer`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `WorldRenderer` | (ctor) | `void` | Constructor |
| `Initialize` | `void` | `void` | Initialize |
| `Shutdown` | `void` | `void` | Shutdown |
| `Render` | `void` | `const World& world, const Camera& camera` | Render world |
| `RenderChunk` | `void` | `const Chunk& chunk` | Render chunk |
| `UpdateChunkMesh` | `void` | `Chunk& chunk` | Update mesh |
| `SetRenderDistance` | `void` | `int distance` | Set render distance |
| `GetRenderDistance` | `int` | `void` | Get render distance |
| `SetFogEnabled` | `void` | `bool enabled` | Toggle fog |
| `SetFogColor` | `void` | `const Vec3& color` | Set fog color |
| `SetFogDensity` | `void` | `float density` | Set fog density |
| `SetSkyColor` | `void` | `const Vec3& color` | Set sky color |
| `SetLightDirection` | `void` | `const Vec3& dir` | Set light direction |
| `SetAmbientLight` | `void` | `float intensity` | Set ambient |
| `GetRenderedChunks` | `int` | `void` | Count rendered |
| `GetDrawCalls` | `int` | `void` | Count draw calls |

---

## `voxel/biome.h`

### `struct BiomeProperties`
| Member | Type | Description |
|--------|------|-------------|
| `type` | `BiomeType` | Biome type |
| `name` | `std::string` | Display name |
| `surfaceBlock` | `BlockType` | Surface block |
| `subsurfaceBlock` | `BlockType` | Subsurface block |
| `undergroundBlock` | `BlockType` | Underground block |
| `treeDensity` | `float` | Tree spawn density |
| `vegetationDensity` | `float` | Vegetation density |
| `temperature` | `float` | Temperature range |
| `humidity` | `float` | Humidity range |
| `heightMin` | `int` | Min height |
| `heightMax` | `int` | Max height |
| `waterColor` | `Vec3` | Water color |
| `grassColor` | `Vec3` | Grass color |
| `foliageColor` | `Vec3` | Foliage color |

### `class BiomeRegistry`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `BiomeRegistry&` | `void` | Singleton |
| `Register` | `void` | `const BiomeProperties& biome` | Register biome |
| `Get` | `const BiomeProperties*` | `BiomeType type` | Get biome |
| `GetByTemperature` | `BiomeType` | `float temp, float humidity` | Find by climate |
| `GetAllBiomes` | `std::vector<BiomeType>` | `void` | All biomes |
| `LoadFromFile` | `void` | `const std::string& path` | Load |
| `SaveToFile` | `void` | `const std::string& path` | Save |

---

## `voxel/block_mesh_builder.h`

### `class BlockMeshBuilder`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `BlockMeshBuilder` | (ctor) | `void` | Constructor |
| `Clear` | `void` | `void` | Clear mesh |
| `AddBlockFace` | `void` | `int x, int y, int z, BlockType type, int face` | Add face |
| `AddBlock` | `void` | `int x, int y, int z, BlockType type` | Add full block |
| `AddBlockTop` | `void` | `int x, int y, int z, BlockType type` | Add top |
| `AddBlockBottom` | `void` | `int x, int y, int z, BlockType type` | Add bottom |
| `AddBlockSide` | `void` | `int x, int y, int z, BlockType type, int side` | Add side |
| `SetUVs` | `void` | `float u1, float v1, float u2, float v2` | Set UVs |
| `SetColor` | `void` | `const Vec3& color` | Set color |
| `SetLight` | `void` | `float light` | Set light |
| `GetMeshData` | `MeshData` | `void` | Get mesh data |
| `GetVertexCount` | `size_t` | `void` | Vertex count |
| `GetIndexCount` | `size_t` | `void` | Index count |

---

## `voxel/chunk_manager.h`

### `class ChunkManager`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `ChunkManager` | (ctor) | `World& world` | Constructor |
| `Update` | `void` | `const Vec3& playerPos` | Update around player |
| `LoadChunk` | `void` | `const ChunkCoord& coord` | Load chunk |
| `UnloadChunk` | `void` | `const ChunkCoord& coord` | Unload chunk |
| `IsChunkLoaded` | `bool` | `const ChunkCoord& coord` | Check loaded |
| `IsChunkLoading` | `bool` | `const ChunkCoord& coord` | Check loading |
| `GetLoadedChunks` | `std::vector<ChunkCoord>` | `void` | All loaded |
| `SetLoadDistance` | `void` | `int distance` | Set load distance |
| `GetLoadDistance` | `int` | `void` | Get load distance |
| `SetUnloadDistance` | `void` | `int distance` | Set unload distance |
| `GetChunkCount` | `size_t` | `void` | Loaded count |
| `GetLoadingCount` | `size_t` | `void` | Loading count |
| `WaitForAll` | `void` | `void` | Wait for loading |
| `Clear` | `void` | `void` | Unload all |
