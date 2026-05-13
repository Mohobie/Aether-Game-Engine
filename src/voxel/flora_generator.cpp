#include "flora_generator.h"
#include "voxel/block_registry.h"
#include <cstdlib>

namespace vge {

FloraGenerator::FloraGenerator() {
    srand(time(nullptr));
}

FloraGenerator::~FloraGenerator() {
}

float FloraGenerator::RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

int FloraGenerator::RandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

int FloraGenerator::FindSurfaceY(World& world, int x, int z) {
    for (int y = 20; y > 0; y--) {
        BlockTypeID block = world.GetBlock(x, y, z);
        if (block != BLOCK_AIR) {
            return y + 1;
        }
    }
    return 5;
}

void FloraGenerator::CreateGrass(World& world, const Vec3& position) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID grassId = registry.GetBlockId("grass_plant");
    if (grassId == BLOCK_AIR) grassId = registry.GetBlockId("flower"); // Fallback
    
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    int z = static_cast<int>(position.z);
    
    world.SetBlock(x, y, z, grassId);
}

void FloraGenerator::CreateFlower(World& world, const Vec3& position, const std::string& type) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID flowerId = registry.GetBlockId(type);
    if (flowerId == BLOCK_AIR) flowerId = registry.GetBlockId("flower");
    
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    int z = static_cast<int>(position.z);
    
    world.SetBlock(x, y, z, flowerId);
}

void FloraGenerator::CreateCactus(World& world, const Vec3& position) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID cactusId = registry.GetBlockId("cactus");
    if (cactusId == BLOCK_AIR) return;
    
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    int z = static_cast<int>(position.z);
    
    int height = RandomInt(2, 4);
    for (int i = 0; i < height; i++) {
        world.SetBlock(x, y + i, z, cactusId);
    }
}

void FloraGenerator::CreateTallGrass(World& world, const Vec3& position) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID tallGrassId = registry.GetBlockId("tall_grass");
    if (tallGrassId == BLOCK_AIR) tallGrassId = registry.GetBlockId("grass_plant");
    
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    int z = static_cast<int>(position.z);
    
    int height = RandomInt(1, 3);
    for (int i = 0; i < height; i++) {
        world.SetBlock(x, y + i, z, tallGrassId);
    }
}

void FloraGenerator::GenerateFlora(World& world, int chunkRadius) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID grassBlockId = registry.GetBlockId("grass");
    BlockTypeID sandId = registry.GetBlockId("sand");
    
    for (int cx = -chunkRadius; cx < chunkRadius; cx++) {
        for (int cz = -chunkRadius; cz < chunkRadius; cz++) {
            for (int i = 0; i < 10; i++) { // 10 flora attempts per chunk
                int x = cx * CHUNK_SIZE + RandomInt(0, CHUNK_SIZE - 1);
                int z = cz * CHUNK_SIZE + RandomInt(0, CHUNK_SIZE - 1);
                int y = FindSurfaceY(world, x, z);
                
                // Check what's below
                BlockTypeID below = world.GetBlock(x, y - 1, z);
                
                if (below == grassBlockId) {
                    // Grass or flowers on grass
                    float roll = RandomFloat(0, 1);
                    if (roll < 0.6f) {
                        CreateGrass(world, Vec3(x, y, z));
                    } else if (roll < 0.8f) {
                        CreateFlower(world, Vec3(x, y, z), "flower");
                    } else {
                        CreateTallGrass(world, Vec3(x, y, z));
                    }
                } else if (below == sandId) {
                    // Cactus on sand (desert)
                    if (RandomFloat(0, 1) < 0.3f) {
                        CreateCactus(world, Vec3(x, y, z));
                    }
                }
            }
        }
    }
}

} // namespace vge
