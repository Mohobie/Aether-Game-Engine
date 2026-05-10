#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "voxel/chunk.h"
#include "voxel/block.h"
#include "voxel/block_registry.h"
#include "core/raycast.h"
#include "math/vec3.h"
#include <iostream>

using namespace vge;

int main() {
    std::cout << "=== World Generation Test ===" << std::endl;
    
    // Initialize block registry
    BlockRegistry::GetInstance().LoadFromFile("../assets/blocks/default_blocks.json");
    
    // Create world
    World world;
    
    // Create generator
    WorldGenerator generator;
    generator.SetSeed(12345);
    
    // Generate a few chunks
    std::cout << "\nGenerating chunks..." << std::endl;
    for (int x = -1; x <= 1; x++) {
        for (int z = -1; z <= 1; z++) {
            Chunk* chunk = world.GetOrCreateChunk(x, 0, z);
            generator.GenerateChunk(*chunk, x, 0, z);
            
            // Count blocks
            int solid = 0, air = 0, grass = 0, dirt = 0, stone = 0, wood = 0, leaves = 0;
            for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++) {
                BlockTypeID blockId = chunk->blocks[i];
                if (blockId == BlockRegistry::GetInstance().GetBlockId("air")) air++;
                else {
                    solid++;
                    if (blockId == BlockRegistry::GetInstance().GetBlockId("grass")) grass++;
                    else if (blockId == BlockRegistry::GetInstance().GetBlockId("dirt")) dirt++;
                    else if (blockId == BlockRegistry::GetInstance().GetBlockId("stone")) stone++;
                    else if (blockId == BlockRegistry::GetInstance().GetBlockId("wood")) wood++;
                    else if (blockId == BlockRegistry::GetInstance().GetBlockId("leaves")) leaves++;
                }
            }
            
            std::cout << "Chunk (" << x << ", 0, " << z << "): "
                      << "solid=" << solid << " air=" << air 
                      << " grass=" << grass << " dirt=" << dirt 
                      << " stone=" << stone << " wood=" << wood 
                      << " leaves=" << leaves << std::endl;
        }
    }
    
    // Test block placement
    std::cout << "\nTesting block placement..." << std::endl;
    // Set some blocks using BlockTypeID
    world.SetBlock(0, 35, 0, BlockRegistry::GetInstance().GetBlockId("wood"));
    BlockTypeID block = world.GetBlock(0, 35, 0);
    std::cout << "Block at (0, 35, 0): " << (int)block << " (expected " << (int)BlockRegistry::GetInstance().GetBlockId("wood") << ")" << std::endl;
    
    // Test raycast
    std::cout << "\nTesting raycast..." << std::endl;
    Raycast raycast;
    RaycastHit hit = raycast.Cast(Vec3(0, 40, 0), Vec3(0, -1, 0), world);
    if (hit.hit) {
        std::cout << "Ray hit at distance: " << hit.distance << std::endl;
        std::cout << "Hit position: (" << hit.position.x << ", " << hit.position.y << ", " << hit.position.z << ")" << std::endl;
    } else {
        std::cout << "Ray missed (expected to hit ground)" << std::endl;
    }
    
    std::cout << "\n=== World Generation Test Complete ===" << std::endl;
    return 0;
}
