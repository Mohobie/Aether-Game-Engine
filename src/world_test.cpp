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
        std::cerr << "[world_test] " << message << std::endl;
        return false;
    }
    return true;
}

} // namespace

int main() {
    const std::filesystem::path blocksPath = FindDefaultBlocksPath();
    if (!BlockRegistry::GetInstance().LoadFromFile(blocksPath.string())) {
        std::cerr << "[world_test] Failed to load block registry from " << blocksPath.string() << std::endl;
        return 1;
    }

    World world;
    world.SetSeed(12345);
    world.Initialize();
    world.GenerateTerrain(0, 0);

    bool ok = true;
    ok &= Expect(world.HasChunk(0, -1, 0), "Expected generated chunk at (0, -1, 0)");
    ok &= Expect(world.HasChunk(0, 0, 0), "Expected generated chunk at (0, 0, 0)");
    ok &= Expect(world.HasChunk(0, 1, 0), "Expected generated chunk at (0, 1, 0)");
    ok &= Expect(world.HasChunk(0, 0), "Expected generated column at (0, 0)");
    ok &= Expect(world.GetBlock(0, 0, 0) != BLOCK_AIR, "Expected terrain block at world origin");

    world.LoadChunkAsync(1, 1);
    ok &= Expect(world.HasChunk(1, 1), "Expected chunk column (1, 1) after LoadChunkAsync");

    world.Update(Vec3(0.0f, 0.0f, 0.0f));
    ok &= Expect(world.HasChunk(0, 0), "Expected spawn chunks to remain loaded after update");

    if (!ok) {
        std::cerr << "[world_test] FAILED" << std::endl;
        return 1;
    }

    std::cout << "[world_test] PASSED" << std::endl;
    return 0;
}
