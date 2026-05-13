#include "cave_generator.h"
#include "voxel/block_registry.h"
#include <cmath>
#include <cstdlib>

namespace vge {

CaveGenerator::CaveGenerator() {
    srand(time(nullptr));
}

CaveGenerator::~CaveGenerator() {
}

float CaveGenerator::RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

int CaveGenerator::RandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

void CaveGenerator::CarveSphere(World& world, const Vec3& center, float radius) {
    int r = static_cast<int>(radius);
    int cx = static_cast<int>(center.x);
    int cy = static_cast<int>(center.y);
    int cz = static_cast<int>(center.z);
    
    for (int x = cx - r; x <= cx + r; x++) {
        for (int y = cy - r; y <= cy + r; y++) {
            for (int z = cz - r; z <= cz + r; z++) {
                float dx = x - center.x;
                float dy = y - center.y;
                float dz = z - center.z;
                float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                
                if (dist <= radius) {
                    // Only carve stone, dirt, gravel - not bedrock or air
                    BlockTypeID block = world.GetBlock(x, y, z);
                    BlockRegistry& registry = BlockRegistry::GetInstance();
                    
                    if (block != BLOCK_AIR && block != registry.GetBlockId("bedrock")) {
                        world.SetBlock(x, y, z, BLOCK_AIR);
                    }
                }
            }
        }
    }
}

bool CaveGenerator::IsValidCavePosition(const Vec3& pos) {
    // Caves should be underground but above bedrock
    return pos.y > 2 && pos.y < 50;
}

void CaveGenerator::GenerateCaveTunnel(World& world, const Vec3& start, const Vec3& end, float radius) {
    Vec3 current = start;
    Vec3 direction = end - start;
    float length = direction.length();
    direction = direction.normalize();
    
    float step = 0.5f;
    float traveled = 0.0f;
    
    while (traveled < length) {
        // Vary radius slightly for natural look
        float currentRadius = radius + RandomFloat(-0.5f, 0.5f);
        currentRadius = std::max(1.0f, currentRadius);
        
        CarveSphere(world, current, currentRadius);
        
        // Add some wandering
        Vec3 wander(
            RandomFloat(-0.3f, 0.3f),
            RandomFloat(-0.2f, 0.2f),
            RandomFloat(-0.3f, 0.3f)
        );
        
        current = current + (direction + wander).normalize() * step;
        traveled += step;
    }
    
    // Carve end point
    CarveSphere(world, end, radius);
}

void CaveGenerator::GenerateCaveSystem(World& world, const Vec3& start, int numTunnels, float maxLength) {
    if (!IsValidCavePosition(start)) return;
    
    for (int i = 0; i < numTunnels; i++) {
        // Random end point
        Vec3 end(
            start.x + RandomFloat(-maxLength, maxLength),
            start.y + RandomFloat(-maxLength * 0.3f, maxLength * 0.3f),
            start.z + RandomFloat(-maxLength, maxLength)
        );
        
        if (!IsValidCavePosition(end)) continue;
        
        float radius = RandomFloat(1.5f, 3.0f);
        GenerateCaveTunnel(world, start, end, radius);
        
        // Branching - 30% chance to create a branch
        if (RandomFloat(0, 1) < 0.3f) {
            Vec3 branchStart = start + (end - start) * 0.5f;
            Vec3 branchEnd(
                branchStart.x + RandomFloat(-maxLength * 0.5f, maxLength * 0.5f),
                branchStart.y + RandomFloat(-maxLength * 0.2f, maxLength * 0.2f),
                branchStart.z + RandomFloat(-maxLength * 0.5f, maxLength * 0.5f)
            );
            
            if (IsValidCavePosition(branchEnd)) {
                float branchRadius = radius * 0.7f;
                GenerateCaveTunnel(world, branchStart, branchEnd, branchRadius);
            }
        }
    }
}

void CaveGenerator::GenerateCaves(World& world, int chunkRadius) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    
    for (int cx = -chunkRadius; cx < chunkRadius; cx++) {
        for (int cz = -chunkRadius; cz < chunkRadius; cz++) {
            // Multiple cave systems per chunk area
            int numSystems = RandomInt(1, 3);
            
            for (int s = 0; s < numSystems; s++) {
                Vec3 start(
                    cx * CHUNK_SIZE + RandomFloat(0, CHUNK_SIZE),
                    RandomFloat(5, 30),
                    cz * CHUNK_SIZE + RandomFloat(0, CHUNK_SIZE)
                );
                
                int numTunnels = RandomInt(3, 8);
                float maxLength = RandomFloat(10, 30);
                
                GenerateCaveSystem(world, start, numTunnels, maxLength);
            }
        }
    }
}

} // namespace vge
