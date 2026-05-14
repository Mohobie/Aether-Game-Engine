#include "water_generator.h"
#include "voxel/block_registry.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace vge {

WaterGenerator::WaterGenerator() {
    srand(time(nullptr));
}

WaterGenerator::~WaterGenerator() {
}

float WaterGenerator::RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

int WaterGenerator::RandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

void WaterGenerator::CarveLakeBed(World& world, const Vec3& center, float radius, int depth) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID dirtId = registry.GetBlockId("dirt");
    BlockTypeID sandId = registry.GetBlockId("sand");
    if (dirtId == BLOCK_AIR) dirtId = 2;
    if (sandId == BLOCK_AIR) sandId = 7;
    
    int r = static_cast<int>(radius);
    int cx = static_cast<int>(center.x);
    int cy = static_cast<int>(center.y);
    int cz = static_cast<int>(center.z);
    
    for (int x = cx - r; x <= cx + r; x++) {
        for (int z = cz - r; z <= cz + r; z++) {
            float dx = x - center.x;
            float dz = z - center.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            
            if (dist > radius) continue;
            
            // Depth varies based on distance from center
            float depthFactor = 1.0f - (dist / radius);
            int localDepth = static_cast<int>(depth * depthFactor);
            if (localDepth < 1) localDepth = 1;
            
            // Carve down from surface
            for (int d = 0; d < localDepth + 3; d++) {
                int y = cy - d;
                BlockTypeID block = world.GetBlock(x, y, z);
                if (block == BLOCK_AIR) continue;
                
                if (d < localDepth) {
                    // Bottom of lake - sand
                    world.SetBlock(x, y, z, sandId);
                } else if (d < localDepth + 2) {
                    // Sides - dirt
                    world.SetBlock(x, y, z, dirtId);
                }
            }
        }
    }
}

void WaterGenerator::FillWithWater(World& world, const Vec3& center, float radius, int waterLevel) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID waterId = registry.GetBlockId("water");
    if (waterId == BLOCK_AIR) waterId = 9; // Default water
    
    int r = static_cast<int>(radius);
    int cx = static_cast<int>(center.x);
    int cz = static_cast<int>(center.z);
    
    for (int x = cx - r; x <= cx + r; x++) {
        for (int z = cz - r; z <= cz + r; z++) {
            float dx = x - center.x;
            float dz = z - center.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            
            if (dist > radius) continue;
            
            // Fill from bottom up to water level
            for (int y = waterLevel - 3; y <= waterLevel; y++) {
                BlockTypeID block = world.GetBlock(x, y, z);
                if (block == BLOCK_AIR) {
                    world.SetBlock(x, y, z, waterId);
                }
            }
        }
    }
}

void WaterGenerator::CreateLake(World& world, const Vec3& center, float radius, int depth) {
    int waterLevel = static_cast<int>(center.y);
    CarveLakeBed(world, center, radius, depth);
    FillWithWater(world, center, radius, waterLevel);
}

void WaterGenerator::CarveRiverBed(World& world, const Vec3& start, const Vec3& end, float width) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID dirtId = registry.GetBlockId("dirt");
    BlockTypeID sandId = registry.GetBlockId("sand");
    if (dirtId == BLOCK_AIR) dirtId = 2;
    if (sandId == BLOCK_AIR) sandId = 7;
    
    Vec3 current = start;
    Vec3 direction = end - start;
    float length = direction.length();
    direction = direction.normalize();
    
    float step = 0.5f;
    float traveled = 0.0f;
    
    while (traveled < length) {
        int cx = static_cast<int>(current.x);
        int cy = static_cast<int>(current.y);
        int cz = static_cast<int>(current.z);
        int w = static_cast<int>(width);
        
        // Carve width around current position
        for (int x = cx - w; x <= cx + w; x++) {
            for (int z = cz - w; z <= cz + w; z++) {
                float dx = x - current.x;
                float dz = z - current.z;
                float dist = std::sqrt(dx * dx + dz * dz);
                
                if (dist > width) continue;
                
                // Carve down
                for (int d = 0; d < 4; d++) {
                    int y = cy - d;
                    BlockTypeID block = world.GetBlock(x, y, z);
                    if (block == BLOCK_AIR) continue;
                    
                    if (d < 2) {
                        world.SetBlock(x, y, z, sandId);
                    } else {
                        world.SetBlock(x, y, z, dirtId);
                    }
                }
            }
        }
        
        // Add some wandering
        Vec3 wander(
            RandomFloat(-0.2f, 0.2f),
            RandomFloat(-0.1f, 0.1f),
            RandomFloat(-0.2f, 0.2f)
        );
        
        current = current + (direction + wander).normalize() * step;
        traveled += step;
    }
}

void WaterGenerator::FillRiverWithWater(World& world, const Vec3& start, const Vec3& end, float width, int waterLevel) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID waterId = registry.GetBlockId("water");
    if (waterId == BLOCK_AIR) waterId = 9;
    
    Vec3 current = start;
    Vec3 direction = end - start;
    float length = direction.length();
    direction = direction.normalize();
    
    float step = 0.5f;
    float traveled = 0.0f;
    
    while (traveled < length) {
        int cx = static_cast<int>(current.x);
        int cz = static_cast<int>(current.z);
        int w = static_cast<int>(width);
        
        for (int x = cx - w; x <= cx + w; x++) {
            for (int z = cz - w; z <= cz + w; z++) {
                float dx = x - current.x;
                float dz = z - current.z;
                float dist = std::sqrt(dx * dx + dz * dz);
                
                if (dist > width) continue;
                
                // Fill with water up to level
                for (int y = waterLevel - 2; y <= waterLevel; y++) {
                    BlockTypeID block = world.GetBlock(x, y, z);
                    if (block == BLOCK_AIR) {
                        world.SetBlock(x, y, z, waterId);
                    }
                }
            }
        }
        
        Vec3 wander(
            RandomFloat(-0.2f, 0.2f),
            0,
            RandomFloat(-0.2f, 0.2f)
        );
        
        current = current + (direction + wander).normalize() * step;
        traveled += step;
    }
}

void WaterGenerator::CreateRiver(World& world, const Vec3& start, const Vec3& end, float width) {
    int waterLevel = static_cast<int>(start.y);
    CarveRiverBed(world, start, end, width);
    FillRiverWithWater(world, start, end, width, waterLevel);
}

void WaterGenerator::GenerateLakes(World& world, int chunkRadius, int numLakes) {
    for (int i = 0; i < numLakes; i++) {
        Vec3 center(
            RandomFloat(-chunkRadius * CHUNK_SIZE, chunkRadius * CHUNK_SIZE),
            RandomFloat(8, 15),
            RandomFloat(-chunkRadius * CHUNK_SIZE, chunkRadius * CHUNK_SIZE)
        );
        
        float radius = RandomFloat(5, 15);
        int depth = RandomInt(2, 5);
        
        CreateLake(world, center, radius, depth);
    }
}

void WaterGenerator::GenerateRivers(World& world, int chunkRadius, int numRivers) {
    for (int i = 0; i < numRivers; i++) {
        Vec3 start(
            RandomFloat(-chunkRadius * CHUNK_SIZE, chunkRadius * CHUNK_SIZE),
            RandomFloat(12, 20),
            RandomFloat(-chunkRadius * CHUNK_SIZE, chunkRadius * CHUNK_SIZE)
        );
        
        Vec3 end(
            start.x + RandomFloat(-50, 50),
            start.y + RandomFloat(-5, -2),
            start.z + RandomFloat(-50, 50)
        );
        
        float width = RandomFloat(2, 4);
        
        CreateRiver(world, start, end, width);
    }
}

} // namespace vge
