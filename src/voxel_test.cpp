#include "voxel/world.h"
#include "voxel/block_registry.h"
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

bool Expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "[voxel_test] " << message << std::endl;
        return false;
    }
    return true;
}

} // namespace

int main() {
    const std::filesystem::path blocksPath = FindDefaultBlocksPath();
    if (!BlockRegistry::GetInstance().LoadFromFile(blocksPath.string())) {
        std::cerr << "[voxel_test] Failed to load block registry from " << blocksPath.string() << std::endl;
        return 1;
    }

    World world;
    world.Initialize();

    const BlockTypeID stone = BlockRegistry::GetInstance().GetBlockId("stone");
    const BlockTypeID wood = BlockRegistry::GetInstance().GetBlockId("wood");
    const BlockTypeID glass = BlockRegistry::GetInstance().GetBlockId("glass");
    const BlockTypeID dirt = BlockRegistry::GetInstance().GetBlockId("dirt");

    world.SetBlock(0, 0, 0, stone);
    world.SetBlock(31, 0, 31, wood);
    world.SetBlock(32, 0, 32, glass);
    world.SetBlock(-1, 0, -1, dirt);
    world.SetBlock(-32, 0, -32, stone);

    bool ok = true;
    ok &= Expect(world.GetBlock(0, 0, 0) == stone, "Expected block at origin to match");
    ok &= Expect(world.GetBlock(31, 0, 31) == wood, "Expected positive edge block to match");
    ok &= Expect(world.GetBlock(32, 0, 32) == glass, "Expected next-chunk block to match");
    ok &= Expect(world.GetBlock(-1, 0, -1) == dirt, "Expected negative block to match");
    ok &= Expect(world.GetBlock(-32, 0, -32) == stone, "Expected negative chunk-boundary block to match");
    ok &= Expect(world.HasChunk(0, 0, 0), "Expected origin chunk to exist");
    ok &= Expect(world.HasChunk(1, 0, 1), "Expected positive adjacent chunk to exist");
    ok &= Expect(world.HasChunk(-1, 0, -1), "Expected negative adjacent chunk to exist");

    if (!ok) {
        std::cerr << "[voxel_test] FAILED" << std::endl;
        return 1;
    }

    std::cout << "[voxel_test] PASSED" << std::endl;
    return 0;
}
