#include "core/save_system.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "voxel/chunk.h"
#include "voxel/block.h"
#include "voxel/block_registry.h"
#include <iostream>
#include <cstdio>

using namespace vge;

int main() {
    std::cout << "=== Save/Load Test ===" << std::endl;
    
    // Create and generate a world
    World world;
    WorldGenerator generator;
    generator.SetSeed(54321);
    
    // Initialize block registry
    BlockRegistry::GetInstance().LoadFromFile("../assets/blocks/default_blocks.json");
    
    std::cout << "\nGenerating world..." << std::endl;
    for (int x = -1; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {  // Include y=1 for height 35
            for (int z = -1; z <= 1; z++) {
                Chunk* chunk = world.GetOrCreateChunk(x, y, z);
                generator.GenerateChunk(*chunk, x, y, z);
            }
        }
    }
    
    // Modify a block (world coordinates)
    // Set some blocks using BlockTypeID
    world.SetBlock(0, 35, 0, BlockRegistry::GetInstance().GetBlockId("wood"));
    
    // Verify - the block might be in chunk (0, 1, 0) since y=35 > 32
    BlockTypeID before = world.GetBlock(0, 35, 0);
    std::cout << "Block at (0, 35, 0) before save: " << (int)before << std::endl;
    
    // Also check chunk coordinates
    Chunk* chunk = world.GetChunk(0, 1, 0);
    if (chunk) {
        BlockTypeID localBlock = chunk->GetBlock(0, 3, 0); // y=35 in chunk (0,1,0) = local y=3
        std::cout << "Block in chunk (0,1,0) at local (0,3,0): " << (int)localBlock << std::endl;
    }
    
    // Save
    const char* savePath = "/tmp/voxel_test_save.bin";
    std::cout << "\nSaving world..." << std::endl;
    if (SaveSystem::SaveWorld(world, savePath)) {
        std::cout << "Save successful!" << std::endl;
    } else {
        std::cout << "Save failed!" << std::endl;
        return 1;
    }
    
    // Create new world and load
    World world2;
    std::cout << "\nLoading world..." << std::endl;
    if (SaveSystem::LoadWorld(world2, savePath)) {
        std::cout << "Load successful!" << std::endl;
    } else {
        std::cout << "Load failed!" << std::endl;
        return 1;
    }
    
    // Verify
    BlockTypeID after = world2.GetBlock(0, 35, 0);
    std::cout << "Block at (0, 35, 0) after load: " << (int)after << std::endl;
    
    if (before == after) {
        std::cout << "\n✅ Save/Load test PASSED!" << std::endl;
    } else {
        std::cout << "\n❌ Save/Load test FAILED!" << std::endl;
        return 1;
    }
    
    // Cleanup
    std::remove(savePath);
    
    std::cout << "\n=== Save/Load Test Complete ===" << std::endl;
    return 0;
}
