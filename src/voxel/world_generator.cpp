#include "world_generator.h"
#include "world.h"
#include "block_registry.h"
#include <cmath>
#include <cstdlib>

namespace vge {

float WorldGenerator::Noise(int x, int z, int seed) {
    int n = x * 1619 + z * 31337 + seed * 1013;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float WorldGenerator::SmoothNoise(int x, int z, int seed) {
    float corners = (Noise(x - 1, z - 1, seed) + Noise(x + 1, z - 1, seed) +
                     Noise(x - 1, z + 1, seed) + Noise(x + 1, z + 1, seed)) / 16.0f;
    float sides = (Noise(x - 1, z, seed) + Noise(x + 1, z, seed) +
                   Noise(x, z - 1, seed) + Noise(x, z + 1, seed)) / 8.0f;
    float center = Noise(x, z, seed) / 4.0f;
    return corners + sides + center;
}

float WorldGenerator::Interpolate(float a, float b, float t) {
    return a + (b - a) * t;
}

void WorldGenerator::GenerateChunk(Chunk& chunk, int chunkX, int chunkY, int chunkZ) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID grass = registry.GetBlockId("grass");
    BlockTypeID dirt = registry.GetBlockId("dirt");
    BlockTypeID stone = registry.GetBlockId("stone");
    BlockTypeID bedrock = registry.GetBlockId("bedrock");
    BlockTypeID air = registry.GetBlockId("air");

    if (grass == BLOCK_AIR) grass = 1;
    if (dirt == BLOCK_AIR) dirt = 2;
    if (stone == BLOCK_AIR) stone = 3;
    if (bedrock == BLOCK_AIR) bedrock = stone;

    // Simple terrain generation for a single chunk
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            int worldX = chunkX * CHUNK_SIZE + x;
            int worldZ = chunkZ * CHUNK_SIZE + z;

            // Use noise for height variation
            float noise1 = SmoothNoise(worldX, worldZ, seed);
            float noise2 = SmoothNoise(worldX / 2, worldZ / 2, seed + 1) * 0.5f;
            float heightValue = (noise1 + noise2) / 1.5f;
            int terrainHeight = static_cast<int>(heightValue * 4) + 2;

            for (int y = 0; y < CHUNK_SIZE; y++) {
                int worldY = chunkY * CHUNK_SIZE + y;

                if (worldY < 0) {
                    chunk.SetBlock(x, y, z, bedrock);
                } else if (worldY < terrainHeight - 2) {
                    chunk.SetBlock(x, y, z, stone);
                } else if (worldY < terrainHeight) {
                    chunk.SetBlock(x, y, z, dirt);
                } else if (worldY == terrainHeight) {
                    chunk.SetBlock(x, y, z, grass);
                } else {
                    chunk.SetBlock(x, y, z, air);
                }
            }
        }
    }

    chunk.loaded = true;
    chunk.SetDirty(true);
}

void WorldGenerator::GenerateTree(World& world, int x, int y, int z) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID wood = registry.GetBlockId("wood");
    BlockTypeID leaves = registry.GetBlockId("leaves");
    
    // Trunk
    int trunkHeight = 4 + (std::abs(Noise(x, z, 42)) * 3);
    for (int i = 0; i < trunkHeight; i++) {
        world.SetBlock(x, y + i, z, wood);
    }
    
    // Leaves
    int leafStart = y + trunkHeight - 2;
    for (int ly = leafStart; ly < leafStart + 3; ly++) {
        int radius = (ly == leafStart + 2) ? 1 : 2;
        for (int lx = x - radius; lx <= x + radius; lx++) {
            for (int lz = z - radius; lz <= z + radius; lz++) {
                if (lx != x || lz != z || ly >= y + trunkHeight) {
                    world.SetBlock(lx, ly, lz, leaves);
                }
            }
        }
    }
}

void WorldGenerator::GenerateHill(World& world, int centerX, int centerZ, int height, int radius) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID grass = registry.GetBlockId("grass");
    BlockTypeID dirt = registry.GetBlockId("dirt");
    BlockTypeID stone = registry.GetBlockId("stone");
    
    for (int x = centerX - radius; x <= centerX + radius; x++) {
        for (int z = centerZ - radius; z <= centerZ + radius; z++) {
            float dist = std::sqrt((x - centerX) * (x - centerX) + (z - centerZ) * (z - centerZ));
            if (dist > radius) continue;
            
            int hillHeight = static_cast<int>(height * (1.0f - dist / radius));
            if (hillHeight < 0) hillHeight = 0;
            
            for (int y = 0; y <= hillHeight; y++) {
                BlockTypeID block;
                if (y == hillHeight) block = grass;
                else if (y >= hillHeight - 2) block = dirt;
                else block = stone;
                
                world.SetBlock(x, y, z, block);
            }
        }
    }
}

void WorldGenerator::GenerateFlatWorld(World& world, int size) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID grass = registry.GetBlockId("grass");
    BlockTypeID dirt = registry.GetBlockId("dirt");
    BlockTypeID stone = registry.GetBlockId("stone");
    BlockTypeID bedrock = registry.GetBlockId("bedrock");
    
    // Generate flat terrain
    for (int x = -size; x <= size; x++) {
        for (int z = -size; z <= size; z++) {
            // Bedrock layer
            world.SetBlock(x, -3, z, bedrock);
            
            // Stone layers
            for (int y = -2; y <= -1; y++) {
                world.SetBlock(x, y, z, stone);
            }
            
            // Dirt layer
            world.SetBlock(x, 0, z, dirt);
            
            // Grass top
            world.SetBlock(x, 1, z, grass);
        }
    }
    
    // Add some trees
    int numTrees = size / 3;
    for (int i = 0; i < numTrees; i++) {
        int tx = (Noise(i, 0, world.GetSeed()) * size);
        int tz = (Noise(0, i, world.GetSeed()) * size);
        GenerateTree(world, tx, 2, tz);
    }
}

void WorldGenerator::GenerateHillyWorld(World& world, int size) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID grass = registry.GetBlockId("grass");
    BlockTypeID dirt = registry.GetBlockId("dirt");
    BlockTypeID stone = registry.GetBlockId("stone");
    BlockTypeID bedrock = registry.GetBlockId("bedrock");
    
    int seed = world.GetSeed();
    
    // Generate hilly terrain
    for (int x = -size; x <= size; x++) {
        for (int z = -size; z <= size; z++) {
            // Calculate height using noise
            float noise1 = SmoothNoise(x, z, seed);
            float noise2 = SmoothNoise(x / 2, z / 2, seed + 1) * 0.5f;
            float noise3 = SmoothNoise(x / 4, z / 4, seed + 2) * 0.25f;
            
            float heightValue = (noise1 + noise2 + noise3) / 1.75f;
            int terrainHeight = static_cast<int>(heightValue * 8) + 2;
            
            if (terrainHeight < 0) terrainHeight = 0;
            
            // Bedrock at bottom
            world.SetBlock(x, -5, z, bedrock);
            
            // Fill from bottom up
            for (int y = -4; y <= terrainHeight; y++) {
                BlockTypeID block;
                if (y == terrainHeight) block = grass;
                else if (y >= terrainHeight - 2) block = dirt;
                else block = stone;
                
                world.SetBlock(x, y, z, block);
            }
        }
    }
    
    // Add trees on flat areas
    int numTrees = size / 2;
    for (int i = 0; i < numTrees; i++) {
        int tx = static_cast<int>(Noise(i * 7, 0, seed) * size * 0.8f);
        int tz = static_cast<int>(Noise(0, i * 7, seed) * size * 0.8f);
        
        // Find surface height
        int surfaceY = 0;
        for (int y = 20; y >= -5; y--) {
            if (world.GetBlock(tx, y, tz) != registry.GetBlockId("air")) {
                surfaceY = y;
                break;
            }
        }
        
        if (surfaceY > 1) {
            GenerateTree(world, tx, surfaceY + 1, tz);
        }
    }
}

void WorldGenerator::GenerateForestWorld(World& world, int size) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID grass = registry.GetBlockId("grass");
    BlockTypeID dirt = registry.GetBlockId("dirt");
    BlockTypeID stone = registry.GetBlockId("stone");
    BlockTypeID water = registry.GetBlockId("water");
    BlockTypeID sand = registry.GetBlockId("sand");
    BlockTypeID bedrock = registry.GetBlockId("bedrock");
    
    int seed = world.GetSeed();
    
    // Generate terrain with water
    for (int x = -size; x <= size; x++) {
        for (int z = -size; z <= size; z++) {
            float noise1 = SmoothNoise(x, z, seed);
            float noise2 = SmoothNoise(x / 3, z / 3, seed + 1) * 0.7f;
            
            float heightValue = (noise1 + noise2) / 1.7f;
            int terrainHeight = static_cast<int>(heightValue * 6) + 1;
            
            // Water level at y=0
            int waterLevel = 0;
            
            // Bedrock
            world.SetBlock(x, -5, z, bedrock);
            
            if (terrainHeight < waterLevel) {
                // Underwater - sand at bottom, water above
                for (int y = -4; y < terrainHeight; y++) {
                    world.SetBlock(x, y, z, stone);
                }
                world.SetBlock(x, terrainHeight, z, sand);
                for (int y = terrainHeight + 1; y <= waterLevel; y++) {
                    world.SetBlock(x, y, z, water);
                }
            } else {
                // Above water
                for (int y = -4; y <= terrainHeight; y++) {
                    BlockTypeID block;
                    if (y == terrainHeight) block = grass;
                    else if (y >= terrainHeight - 2) block = dirt;
                    else block = stone;
                    
                    world.SetBlock(x, y, z, block);
                }
            }
        }
    }
    
    // Add lots of trees
    int numTrees = size;
    for (int i = 0; i < numTrees; i++) {
        int tx = static_cast<int>(Noise(i * 13, 0, seed) * size * 0.9f);
        int tz = static_cast<int>(Noise(0, i * 13, seed) * size * 0.9f);
        
        // Find surface height
        int surfaceY = 0;
        for (int y = 20; y >= -5; y--) {
            BlockTypeID block = world.GetBlock(tx, y, tz);
            if (block != registry.GetBlockId("air") && block != water) {
                surfaceY = y;
                break;
            }
        }
        
        if (surfaceY > 0) {
            GenerateTree(world, tx, surfaceY + 1, tz);
        }
    }
}

void WorldGenerator::GenerateDesertWorld(World& world, int size) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID sand = registry.GetBlockId("sand");
    BlockTypeID sandstone = registry.GetBlockId("stone"); // Use stone as sandstone
    BlockTypeID bedrock = registry.GetBlockId("bedrock");
    
    int seed = world.GetSeed();
    
    // Generate desert terrain with dunes
    for (int x = -size; x <= size; x++) {
        for (int z = -size; z <= size; z++) {
            float noise1 = SmoothNoise(x, z, seed);
            float noise2 = SmoothNoise(x / 2, z / 2, seed + 1) * 0.5f;
            
            float heightValue = (noise1 + noise2) / 1.5f;
            int terrainHeight = static_cast<int>(heightValue * 4);
            
            if (terrainHeight < 0) terrainHeight = 0;
            
            // Bedrock
            world.SetBlock(x, -5, z, bedrock);
            
            // Sandstone layers
            for (int y = -4; y < terrainHeight - 2; y++) {
                world.SetBlock(x, y, z, sandstone);
            }
            
            // Sand layers
            for (int y = terrainHeight - 2; y <= terrainHeight; y++) {
                world.SetBlock(x, y, z, sand);
            }
        }
    }
    
    // Add cactus-like structures (just tall sand pillars for now)
    int numCactus = size / 5;
    for (int i = 0; i < numCactus; i++) {
        int cx = static_cast<int>(Noise(i * 17, 0, seed) * size * 0.8f);
        int cz = static_cast<int>(Noise(0, i * 17, seed) * size * 0.8f);
        
        int surfaceY = 0;
        for (int y = 20; y >= -5; y--) {
            if (world.GetBlock(cx, y, cz) != registry.GetBlockId("air")) {
                surfaceY = y;
                break;
            }
        }
        
        if (surfaceY >= 0) {
            int height = 2 + static_cast<int>(std::abs(Noise(cx, cz, seed + 100)) * 3);
            for (int y = surfaceY + 1; y <= surfaceY + height; y++) {
                world.SetBlock(cx, y, cz, sand);
            }
        }
    }
}

} // namespace vge
