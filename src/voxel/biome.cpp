#include "biome.h"
#include "world_generator.h"
#include <cmath>

namespace vge {

// Simple noise function (placeholder - would use proper Perlin/Simplex noise)
float Noise2D(int x, int z, int seed) {
    int n = x + z * 57 + seed * 131;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float Noise2DSmooth(float x, float z, int seed) {
    int ix = (int)x;
    int iz = (int)z;
    float fx = x - ix;
    float fz = z - iz;
    
    float a = Noise2D(ix, iz, seed);
    float b = Noise2D(ix + 1, iz, seed);
    float c = Noise2D(ix, iz + 1, seed);
    float d = Noise2D(ix + 1, iz + 1, seed);
    
    float u = fx * fx * (3.0f - 2.0f * fx);
    float v = fz * fz * (3.0f - 2.0f * fz);
    
    return a * (1.0f - u) * (1.0f - v) + b * u * (1.0f - v) + c * (1.0f - u) * v + d * u * v;
}

BiomeType Biome::GetBiomeType(float temperature, float humidity) {
    if (temperature < 0.2f) {
        return BiomeType::Snow;
    } else if (temperature < 0.4f) {
        return BiomeType::Plains;
    } else if (humidity > 0.7f) {
        return BiomeType::Forest;
    } else if (humidity < 0.2f) {
        return BiomeType::Desert;
    } else {
        return BiomeType::Plains;
    }
}

float Biome::GetBaseHeight(BiomeType type) {
    switch (type) {
        case BiomeType::Plains: return 64.0f;
        case BiomeType::Forest: return 68.0f;
        case BiomeType::Desert: return 62.0f;
        case BiomeType::Snow: return 70.0f;
        default: return 64.0f;
    }
}

float Biome::GetHeightVariation(BiomeType type) {
    switch (type) {
        case BiomeType::Plains: return 3.0f;
        case BiomeType::Forest: return 8.0f;
        case BiomeType::Desert: return 5.0f;
        case BiomeType::Snow: return 12.0f;
        default: return 5.0f;
    }
}

BlockType Biome::GetSurfaceBlock(BiomeType type) {
    switch (type) {
        case BiomeType::Plains: return BlockType::Grass;
        case BiomeType::Forest: return BlockType::Grass;
        case BiomeType::Desert: return BlockType::Sand;
        case BiomeType::Snow: return BlockType::Snow;
        default: return BlockType::Grass;
    }
}

BlockType Biome::GetSubsurfaceBlock(BiomeType type) {
    switch (type) {
        case BiomeType::Desert: return BlockType::Sand;
        default: return BlockType::Dirt;
    }
}

void Biome::GenerateChunkColumn(Chunk* chunk, int x, int z, float worldX, float worldZ, int seed) {
    // Sample biome parameters
    float temperature = Noise2DSmooth(worldX * 0.01f, worldZ * 0.01f, seed) * 0.5f + 0.5f;
    float humidity = Noise2DSmooth(worldX * 0.01f + 100, worldZ * 0.01f + 100, seed) * 0.5f + 0.5f;
    
    BiomeType biome = GetBiomeType(temperature, humidity);
    
    // Generate height
    float baseHeight = GetBaseHeight(biome);
    float variation = GetHeightVariation(biome);
    float heightNoise = Noise2DSmooth(worldX * 0.05f, worldZ * 0.05f, seed + 1);
    float height = baseHeight + heightNoise * variation;
    
    int groundHeight = (int)height;
    
    BlockType surfaceBlock = GetSurfaceBlock(biome);
    BlockType subsurfaceBlock = GetSubsurfaceBlock(biome);
    
    // Fill column
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        int worldY = chunk->GetChunkY() * CHUNK_SIZE + y;
        
        if (worldY > groundHeight) {
            // Air
            chunk->SetBlock(x, y, z, BlockType::Air);
        } else if (worldY == groundHeight) {
            // Surface
            chunk->SetBlock(x, y, z, surfaceBlock);
        } else if (worldY > groundHeight - 4) {
            // Subsurface (dirt/sand)
            chunk->SetBlock(x, y, z, subsurfaceBlock);
        } else if (worldY > 0) {
            // Stone
            chunk->SetBlock(x, y, z, BlockType::Stone);
        } else {
            // Bedrock at bottom
            chunk->SetBlock(x, y, z, BlockType::Bedrock);
        }
    }
    
    // Add trees in forests
    if (biome == BiomeType::Forest && Noise2D((int)worldX, (int)worldZ, seed + 2) > 0.7f) {
        int treeHeight = 4 + (int)(Noise2D((int)worldX, (int)worldZ, seed + 3) * 3);
        for (int h = 1; h <= treeHeight && (groundHeight + h) < CHUNK_SIZE; ++h) {
            int ty = groundHeight + h - chunk->GetChunkY() * CHUNK_SIZE;
            if (ty >= 0 && ty < CHUNK_SIZE) {
                chunk->SetBlock(x, ty, z, BlockType::Wood);
            }
        }
    }
}

} // namespace vge