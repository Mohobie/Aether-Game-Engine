#include "world_generator.h"
#include "chunk.h"
#include "block.h"
#include <cmath>
#include <random>
#include <iostream>

namespace vge {

WorldGenerator::WorldGenerator() : seed(12345) {
    rng.seed(seed);
}

void WorldGenerator::SetSeed(unsigned int s) {
    seed = s;
    rng.seed(s);
    std::cout << "[WorldGen] Seed set to " << s << std::endl;
}

float WorldGenerator::GetNoise(float x, float y, float z) {
    // Simple value noise using multiple octaves
    float result = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.05f;
    
    for (int i = 0; i < 4; i++) {
        result += amplitude * std::sin(x * frequency + seed) * 
                  std::cos(y * frequency + seed * 0.5f) * 
                  std::sin(z * frequency + seed * 0.25f);
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    
    return result;
}

float WorldGenerator::GetHeightNoise(float x, float z) {
    // 2D height noise for terrain generation
    float result = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.02f;
    
    for (int i = 0; i < 6; i++) {
        result += amplitude * std::sin(x * frequency + seed) * 
                  std::cos(z * frequency + seed * 0.7f);
        amplitude *= 0.5f;
        frequency *= 2.1f;
    }
    
    return result;
}

void WorldGenerator::GenerateChunk(Chunk& chunk, int cx, int cy, int cz) {
    int worldBaseX = cx * CHUNK_SIZE;
    int worldBaseY = cy * CHUNK_SIZE;
    int worldBaseZ = cz * CHUNK_SIZE;
    
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            // Get height at this x,z position
            float heightNoise = GetHeightNoise(worldBaseX + x, worldBaseZ + z);
            int terrainHeight = (int)(heightNoise * 8.0f + 32.0f); // Base height around 32
            
            for (int y = 0; y < CHUNK_SIZE; y++) {
                int worldY = worldBaseY + y;
                int index = x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
                
                if (worldY > terrainHeight) {
                    // Above terrain - air
                    chunk.blocks[index] = BlockType::Air;
                } else if (worldY == terrainHeight) {
                    // Surface - grass
                    chunk.blocks[index] = BlockType::Grass;
                } else if (worldY > terrainHeight - 4) {
                    // Dirt below grass
                    chunk.blocks[index] = BlockType::Dirt;
                } else if (worldY > 0) {
                    // Stone
                    chunk.blocks[index] = BlockType::Stone;
                } else {
                    // Bedrock at bottom
                    chunk.blocks[index] = BlockType::Bedrock;
                }
            }
        }
    }
    
    // Occasionally add trees on grass
    for (int x = 2; x < CHUNK_SIZE - 2; x += 4) {
        for (int z = 2; z < CHUNK_SIZE - 2; z += 4) {
            int worldY = (int)(GetHeightNoise(worldBaseX + x, worldBaseZ + z) * 8.0f + 32.0f);
            int localY = worldY - worldBaseY;
            
            if (localY >= 0 && localY < CHUNK_SIZE - 6) {
                int index = x * CHUNK_SIZE * CHUNK_SIZE + localY * CHUNK_SIZE + z;
                if (chunk.blocks[index] == BlockType::Grass) {
                    // 10% chance for tree
                    if ((rng() % 100) < 10) {
                        GenerateTree(chunk, x, localY + 1, z);
                    }
                }
            }
        }
    }
    
    chunk.loaded = true;
    chunk.dirty = true;
}

void WorldGenerator::GenerateTree(Chunk& chunk, int x, int y, int z) {
    // Simple tree generation
    int trunkHeight = 4 + (rng() % 3);
    
    // Trunk
    for (int i = 0; i < trunkHeight; i++) {
        if (y + i < CHUNK_SIZE) {
            int index = x * CHUNK_SIZE * CHUNK_SIZE + (y + i) * CHUNK_SIZE + z;
            chunk.blocks[index] = BlockType::Wood;
        }
    }
    
    // Leaves
    int leafStart = y + trunkHeight - 2;
    for (int lx = -2; lx <= 2; lx++) {
        for (int ly = 0; ly <= 2; ly++) {
            for (int lz = -2; lz <= 2; lz++) {
                int nx = x + lx;
                int ny = leafStart + ly;
                int nz = z + lz;
                
                if (nx >= 0 && nx < CHUNK_SIZE && 
                    ny >= 0 && ny < CHUNK_SIZE && 
                    nz >= 0 && nz < CHUNK_SIZE) {
                    // Skip corners for rounder look
                    if (std::abs(lx) == 2 && std::abs(lz) == 2) continue;
                    if (ly == 2 && (std::abs(lx) > 1 || std::abs(lz) > 1)) continue;
                    
                    int index = nx * CHUNK_SIZE * CHUNK_SIZE + ny * CHUNK_SIZE + nz;
                    if (chunk.blocks[index] == BlockType::Air) {
                        chunk.blocks[index] = BlockType::Leaves;
                    }
                }
            }
        }
    }
}

} // namespace vge
