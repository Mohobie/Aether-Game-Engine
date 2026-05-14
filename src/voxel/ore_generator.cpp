#include "ore_generator.h"
#include "voxel/block_registry.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace vge {

OreGenerator::OreGenerator() {
    srand(time(nullptr));
}

OreGenerator::~OreGenerator() {
}

float OreGenerator::RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

int OreGenerator::RandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

std::string OreGenerator::GetOreBlockId(const std::string& oreType) {
    if (oreType == "coal") return "coal_ore";
    if (oreType == "iron") return "iron_ore";
    if (oreType == "gold") return "gold_ore";
    if (oreType == "diamond") return "diamond_ore";
    if (oreType == "emerald") return "emerald_ore";
    if (oreType == "redstone") return "redstone_ore";
    if (oreType == "lapis") return "lapis_ore";
    return "coal_ore"; // Default
}

void OreGenerator::GetOreDepthRange(const std::string& oreType, int& minY, int& maxY) {
    if (oreType == "coal") { minY = 5; maxY = 128; }
    else if (oreType == "iron") { minY = 5; maxY = 64; }
    else if (oreType == "gold") { minY = 5; maxY = 32; }
    else if (oreType == "diamond") { minY = 5; maxY = 16; }
    else if (oreType == "emerald") { minY = 5; maxY = 32; }
    else if (oreType == "redstone") { minY = 5; maxY = 16; }
    else if (oreType == "lapis") { minY = 5; maxY = 32; }
    else { minY = 5; maxY = 64; }
}

void OreGenerator::GenerateOreVein(World& world, const Vec3& center, const std::string& oreType,
                                   int size, float density) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    std::string blockId = GetOreBlockId(oreType);
    BlockTypeID oreBlock = registry.GetBlockId(blockId);
    BlockTypeID stoneBlock = registry.GetBlockId("stone");
    
    if (oreBlock == BLOCK_AIR) return;
    if (stoneBlock == BLOCK_AIR) stoneBlock = 3; // Default stone
    
    int radius = size / 2;
    int placed = 0;
    int target = static_cast<int>(size * density);
    
    for (int x = -radius; x <= radius && placed < target; x++) {
        for (int y = -radius; y <= radius && placed < target; y++) {
            for (int z = -radius; z <= radius && placed < target; z++) {
                float dist = std::sqrt(x*x + y*y + z*z);
                if (dist > radius) continue;
                
                // Density falloff from center
                float chance = 1.0f - (dist / radius);
                if (RandomFloat(0, 1) > chance) continue;
                
                int wx = static_cast<int>(center.x) + x;
                int wy = static_cast<int>(center.y) + y;
                int wz = static_cast<int>(center.z) + z;
                
                // Only replace stone
                if (world.GetBlock(wx, wy, wz) == stoneBlock) {
                    world.SetBlock(wx, wy, wz, oreBlock);
                    placed++;
                }
            }
        }
    }
}

void OreGenerator::GenerateOres(World& world, int chunkRadius) {
    // Ore types with their generation parameters
    struct OreConfig {
        std::string type;
        int veinSize;
        float density;
        int veinsPerChunk;
    };
    
    std::vector<OreConfig> ores = {
        {"coal", 8, 0.7f, 20},
        {"iron", 6, 0.6f, 15},
        {"gold", 4, 0.5f, 4},
        {"diamond", 3, 0.4f, 3},
        {"emerald", 3, 0.3f, 2},
        {"redstone", 5, 0.5f, 8},
        {"lapis", 4, 0.5f, 3}
    };
    
    for (int cx = -chunkRadius; cx < chunkRadius; cx++) {
        for (int cz = -chunkRadius; cz < chunkRadius; cz++) {
            for (const auto& ore : ores) {
                int minY, maxY;
                GetOreDepthRange(ore.type, minY, maxY);
                
                for (int v = 0; v < ore.veinsPerChunk; v++) {
                    Vec3 center(
                        cx * CHUNK_SIZE + RandomFloat(0, CHUNK_SIZE),
                        RandomFloat(minY, maxY),
                        cz * CHUNK_SIZE + RandomFloat(0, CHUNK_SIZE)
                    );
                    
                    GenerateOreVein(world, center, ore.type, ore.veinSize, ore.density);
                }
            }
        }
    }
}

} // namespace vge
