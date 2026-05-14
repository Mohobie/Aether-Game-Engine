#include "core/save_system.h"
#include "voxel/block.h"
#include "voxel/block_registry.h"
#include "voxel/chunk.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include <cstdio>
#include <filesystem>
#include <iostream>

using namespace vge;

namespace {

std::filesystem::path FindDefaultBlocksPath() {
    const std::filesystem::path local = "assets/blocks/default_blocks.json";
    if (std::filesystem::exists(local)) {
        return local;
    }

    const std::filesystem::path parent = "../assets/blocks/default_blocks.json";
    if (std::filesystem::exists(parent)) {
        return parent;
    }

    return local;
}

} // namespace

int main() {
    std::cout << "=== Save/Load Test ===" << std::endl;

    World world;
    WorldGenerator generator;
    generator.SetSeed(54321);

    const std::filesystem::path blocksPath = FindDefaultBlocksPath();
    if (!BlockRegistry::GetInstance().LoadFromFile(blocksPath.string())) {
        std::cerr << "[save_test] Failed to load block registry from " << blocksPath.string() << std::endl;
        return 1;
    }

    std::cout << "\nGenerating world..." << std::endl;
    for (int x = -1; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                Chunk* chunk = world.GetOrCreateChunk(x, y, z);
                generator.GenerateChunk(*chunk, x, y, z);
            }
        }
    }

    world.SetBlock(0, 35, 0, BlockRegistry::GetInstance().GetBlockId("wood"));

    const BlockTypeID before = world.GetBlock(0, 35, 0);
    std::cout << "Block at (0, 35, 0) before save: " << static_cast<int>(before) << std::endl;

    Chunk* chunk = world.GetChunk(0, 1, 0);
    if (chunk) {
        const BlockTypeID localBlock = chunk->GetBlock(0, 3, 0);
        std::cout << "Block in chunk (0,1,0) at local (0,3,0): " << static_cast<int>(localBlock) << std::endl;
    }

    const std::filesystem::path savePath =
        std::filesystem::temp_directory_path() / "voxel_test_save.bin";
    std::cout << "\nSaving world..." << std::endl;
    if (SaveSystem::SaveWorld(world, savePath.string())) {
        std::cout << "Save successful!" << std::endl;
    } else {
        std::cout << "Save failed!" << std::endl;
        return 1;
    }

    World world2;
    std::cout << "\nLoading world..." << std::endl;
    if (SaveSystem::LoadWorld(world2, savePath.string())) {
        std::cout << "Load successful!" << std::endl;
    } else {
        std::cout << "Load failed!" << std::endl;
        return 1;
    }

    const BlockTypeID after = world2.GetBlock(0, 35, 0);
    std::cout << "Block at (0, 35, 0) after load: " << static_cast<int>(after) << std::endl;

    if (before == after) {
        std::cout << "\n[save_test] PASSED" << std::endl;
    } else {
        std::cout << "\n[save_test] FAILED" << std::endl;
        return 1;
    }

    std::filesystem::remove(savePath);

    std::cout << "\n=== Save/Load Test Complete ===" << std::endl;
    return 0;
}
