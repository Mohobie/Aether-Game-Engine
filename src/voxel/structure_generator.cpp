#include "structure_generator.h"
#include "voxel/block_registry.h"
#include <cstdlib>
#include <cmath>

namespace vge {

StructureGenerator::StructureGenerator() {
    srand(time(nullptr));
}

StructureGenerator::~StructureGenerator() {
}

float StructureGenerator::RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

int StructureGenerator::RandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

void StructureGenerator::PlaceBlock(World& world, int x, int y, int z, const std::string& blockId) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID block = registry.GetBlockId(blockId);
    if (block != BLOCK_AIR) {
        world.SetBlock(x, y, z, block);
    }
}

void StructureGenerator::CreateTree(World& world, const Vec3& position) {
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    int z = static_cast<int>(position.z);
    
    int height = RandomInt(4, 6);
    
    // Trunk
    for (int i = 0; i < height; i++) {
        PlaceBlock(world, x, y + i, z, "wood");
    }
    
    // Leaves
    int leafStart = y + height - 2;
    for (int ly = leafStart; ly <= y + height + 1; ly++) {
        int radius = (ly == y + height + 1) ? 1 : 2;
        for (int lx = x - radius; lx <= x + radius; lx++) {
            for (int lz = z - radius; lz <= z + radius; lz++) {
                if (lx == x && lz == z && ly < y + height) continue; // Skip trunk
                PlaceBlock(world, lx, ly, lz, "leaves");
            }
        }
    }
}

void StructureGenerator::CreateHouse(World& world, const Vec3& position) {
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    int z = static_cast<int>(position.z);
    
    int width = 5;
    int depth = 5;
    int height = 4;
    
    // Floor
    for (int dx = 0; dx < width; dx++) {
        for (int dz = 0; dz < depth; dz++) {
            PlaceBlock(world, x + dx, y, z + dz, "planks");
        }
    }
    
    // Walls
    for (int dy = 1; dy <= height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            PlaceBlock(world, x + dx, y + dy, z, "planks");
            PlaceBlock(world, x + dx, y + dy, z + depth - 1, "planks");
        }
        for (int dz = 1; dz < depth - 1; dz++) {
            PlaceBlock(world, x, y + dy, z + dz, "planks");
            PlaceBlock(world, x + width - 1, y + dy, z + dz, "planks");
        }
    }
    
    // Roof
    for (int dx = -1; dx <= width; dx++) {
        for (int dz = -1; dz <= depth; dz++) {
            PlaceBlock(world, x + dx, y + height + 1, z + dz, "wood");
        }
    }
    
    // Door
    world.SetBlock(x + width / 2, y + 1, z, BLOCK_AIR);
    world.SetBlock(x + width / 2, y + 2, z, BLOCK_AIR);
    
    // Windows
    PlaceBlock(world, x + 1, y + 2, z, "glass");
    PlaceBlock(world, x + width - 2, y + 2, z, "glass");
}

void StructureGenerator::CreateDungeon(World& world, const Vec3& position) {
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    int z = static_cast<int>(position.z);
    
    int size = 7;
    int height = 4;
    
    // Floor
    for (int dx = 0; dx < size; dx++) {
        for (int dz = 0; dz < size; dz++) {
            PlaceBlock(world, x + dx, y, z + dz, "cobblestone");
        }
    }
    
    // Walls
    for (int dy = 1; dy <= height; dy++) {
        for (int dx = 0; dx < size; dx++) {
            PlaceBlock(world, x + dx, y + dy, z, "cobblestone");
            PlaceBlock(world, x + dx, y + dy, z + size - 1, "cobblestone");
        }
        for (int dz = 1; dz < size - 1; dz++) {
            PlaceBlock(world, x, y + dy, z + dz, "cobblestone");
            PlaceBlock(world, x + size - 1, y + dy, z + dz, "cobblestone");
        }
    }
    
    // Ceiling
    for (int dx = 0; dx < size; dx++) {
        for (int dz = 0; dz < size; dz++) {
            PlaceBlock(world, x + dx, y + height + 1, z + dz, "cobblestone");
        }
    }
    
    // Spawner in center
    PlaceBlock(world, x + size / 2, y + 1, z + size / 2, "mob_spawner");
    
    // Chests in corners
    PlaceBlock(world, x + 1, y + 1, z + 1, "chest");
    PlaceBlock(world, x + size - 2, y + 1, z + size - 2, "chest");
}

void StructureGenerator::GenerateTrees(World& world, int chunkRadius, int treeDensity) {
    for (int cx = -chunkRadius; cx < chunkRadius; cx++) {
        for (int cz = -chunkRadius; cz < chunkRadius; cz++) {
            for (int i = 0; i < treeDensity; i++) {
                Vec3 pos(
                    cx * CHUNK_SIZE + RandomFloat(2, CHUNK_SIZE - 2),
                    10, // Surface level - adjust based on terrain
                    cz * CHUNK_SIZE + RandomFloat(2, CHUNK_SIZE - 2)
                );
                
                // Find ground level
                for (int y = 20; y > 0; y--) {
                    if (world.GetBlock(static_cast<int>(pos.x), y, static_cast<int>(pos.z)) != BLOCK_AIR) {
                        pos.y = y + 1;
                        break;
                    }
                }
                
                CreateTree(world, pos);
            }
        }
    }
}

} // namespace vge
