#include "mineshaft_generator.h"
#include "voxel/block_registry.h"
#include "voxel/cave_generator.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace vge {

MineshaftGenerator::MineshaftGenerator() {
    srand(time(nullptr));
}

MineshaftGenerator::~MineshaftGenerator() {
}

float MineshaftGenerator::RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

int MineshaftGenerator::RandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

void MineshaftGenerator::PlaceBlock(World& world, int x, int y, int z, const std::string& blockId) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID block = registry.GetBlockId(blockId);
    if (block != BLOCK_AIR) {
        world.SetBlock(x, y, z, block);
    }
}

bool MineshaftGenerator::IsUndergroundBlock(World& world, int x, int y, int z) {
    BlockRegistry& registry = BlockRegistry::GetInstance();
    BlockTypeID block = world.GetBlock(x, y, z);
    
    if (block == BLOCK_AIR) return false;
    if (block == registry.GetBlockId("bedrock")) return false;
    if (block == registry.GetBlockId("water")) return false;
    if (block == registry.GetBlockId("lava")) return false;
    
    return true;
}

bool MineshaftGenerator::IsValidMineshaftPosition(World& world, const Vec3& pos) {
    int y = static_cast<int>(pos.y);
    
    // Must be underground but above bedrock
    if (y < 5 || y > 45) return false;
    
    // Check surrounding area has enough solid blocks
    int solidCount = 0;
    int checkRadius = 3;
    int totalChecks = 0;
    
    for (int dx = -checkRadius; dx <= checkRadius; dx++) {
        for (int dy = -checkRadius; dy <= checkRadius; dy++) {
            for (int dz = -checkRadius; dz <= checkRadius; dz++) {
                int cx = static_cast<int>(pos.x) + dx;
                int cy = y + dy;
                int cz = static_cast<int>(pos.z) + dz;
                
                if (IsUndergroundBlock(world, cx, cy, cz)) {
                    solidCount++;
                }
                totalChecks++;
            }
        }
    }
    
    // Need at least 60% solid blocks
    return (static_cast<float>(solidCount) / totalChecks) > 0.6f;
}

void MineshaftGenerator::GenerateCorridor(World& world, const Vec3& start, const Vec3& end, float width, float height) {
    Vec3 current = start;
    Vec3 direction = end - start;
    float length = direction.length();
    
    if (length < 0.1f) return;
    
    direction = direction.normalize();
    float step = 0.5f;
    float traveled = 0.0f;
    
    int halfWidth = static_cast<int>(width / 2.0f);
    int ceilHeight = static_cast<int>(height);
    
    while (traveled < length) {
        int cx = static_cast<int>(current.x);
        int cy = static_cast<int>(current.y);
        int cz = static_cast<int>(current.z);
        
        // Carve out the corridor
        for (int dx = -halfWidth; dx <= halfWidth; dx++) {
            for (int dy = 0; dy <= ceilHeight; dy++) {
                for (int dz = -halfWidth; dz <= halfWidth; dz++) {
                    int bx = cx + dx;
                    int by = cy + dy;
                    int bz = cz + dz;
                    
                    // Only carve underground blocks
                    if (IsUndergroundBlock(world, bx, by, bz)) {
                        world.SetBlock(bx, by, bz, BLOCK_AIR);
                    }
                }
            }
        }
        
        current = current + direction * step;
        traveled += step;
    }
    
    // Carve end point
    int ex = static_cast<int>(end.x);
    int ey = static_cast<int>(end.y);
    int ez = static_cast<int>(end.z);
    
    for (int dx = -halfWidth; dx <= halfWidth; dx++) {
        for (int dy = 0; dy <= ceilHeight; dy++) {
            for (int dz = -halfWidth; dz <= halfWidth; dz++) {
                int bx = ex + dx;
                int by = ey + dy;
                int bz = ez + dz;
                
                if (IsUndergroundBlock(world, bx, by, bz)) {
                    world.SetBlock(bx, by, bz, BLOCK_AIR);
                }
            }
        }
    }
}

void MineshaftGenerator::GenerateSupports(World& world, const Vec3& start, const Vec3& end, float width, float height) {
    Vec3 direction = end - start;
    float length = direction.length();
    
    if (length < 0.1f) return;
    
    direction = direction.normalize();
    
    // Place supports every 4-6 blocks
    float supportSpacing = 4.0f + RandomFloat(0, 2);
    float traveled = 0.0f;
    
    int halfWidth = static_cast<int>(width / 2.0f);
    int ceilHeight = static_cast<int>(height);
    
    while (traveled < length) {
        Vec3 pos = start + direction * traveled;
        int x = static_cast<int>(pos.x);
        int y = static_cast<int>(pos.y);
        int z = static_cast<int>(pos.z);
        
        // Place wooden posts at corners
        PlaceBlock(world, x - halfWidth, y, z - halfWidth, "wood");
        PlaceBlock(world, x - halfWidth, y + 1, z - halfWidth, "wood");
        PlaceBlock(world, x + halfWidth, y, z - halfWidth, "wood");
        PlaceBlock(world, x + halfWidth, y + 1, z - halfWidth, "wood");
        PlaceBlock(world, x - halfWidth, y, z + halfWidth, "wood");
        PlaceBlock(world, x - halfWidth, y + 1, z + halfWidth, "wood");
        PlaceBlock(world, x + halfWidth, y, z + halfWidth, "wood");
        PlaceBlock(world, x + halfWidth, y + 1, z + halfWidth, "wood");
        
        // Place crossbeams at ceiling
        for (int dx = -halfWidth; dx <= halfWidth; dx++) {
            PlaceBlock(world, x + dx, y + ceilHeight, z - halfWidth, "wood");
            PlaceBlock(world, x + dx, y + ceilHeight, z + halfWidth, "wood");
        }
        for (int dz = -halfWidth; dz <= halfWidth; dz++) {
            PlaceBlock(world, x - halfWidth, y + ceilHeight, z + dz, "wood");
            PlaceBlock(world, x + halfWidth, y + ceilHeight, z + dz, "wood");
        }
        
        traveled += supportSpacing;
    }
}

void MineshaftGenerator::GenerateRails(World& world, const Vec3& start, const Vec3& end) {
    Vec3 direction = end - start;
    float length = direction.length();
    
    if (length < 0.1f) return;
    
    direction = direction.normalize();
    float step = 1.0f;
    float traveled = 0.0f;
    
    while (traveled < length) {
        Vec3 pos = start + direction * traveled;
        int x = static_cast<int>(pos.x);
        int y = static_cast<int>(pos.y);
        int z = static_cast<int>(pos.z);
        
        // Place rail on floor
        PlaceBlock(world, x, y, z, "rail");
        
        traveled += step;
    }
}

void MineshaftGenerator::PlaceLootChest(World& world, int x, int y, int z) {
    PlaceBlock(world, x, y, z, "chest");
}

void MineshaftGenerator::PlaceCaveSpiderSpawner(World& world, int x, int y, int z) {
    PlaceBlock(world, x, y, z, "mob_spawner");
}

void MineshaftGenerator::GenerateRoom(World& world, const Vec3& center, int width, int height, int depth) {
    int cx = static_cast<int>(center.x);
    int cy = static_cast<int>(center.y);
    int cz = static_cast<int>(center.z);
    
    int halfW = width / 2;
    int halfD = depth / 2;
    
    // Carve out room
    for (int dx = -halfW; dx <= halfW; dx++) {
        for (int dy = 0; dy <= height; dy++) {
            for (int dz = -halfD; dz <= halfD; dz++) {
                int bx = cx + dx;
                int by = cy + dy;
                int bz = cz + dz;
                
                if (IsUndergroundBlock(world, bx, by, bz)) {
                    world.SetBlock(bx, by, bz, BLOCK_AIR);
                }
            }
        }
    }
    
    // Place supports at corners
    PlaceBlock(world, cx - halfW, cy, cz - halfD, "wood");
    PlaceBlock(world, cx - halfW, cy + 1, cz - halfD, "wood");
    PlaceBlock(world, cx + halfW, cy, cz - halfD, "wood");
    PlaceBlock(world, cx + halfW, cy + 1, cz - halfD, "wood");
    PlaceBlock(world, cx - halfW, cy, cz + halfD, "wood");
    PlaceBlock(world, cx - halfW, cy + 1, cz + halfD, "wood");
    PlaceBlock(world, cx + halfW, cy, cz + halfD, "wood");
    PlaceBlock(world, cx + halfW, cy + 1, cz + halfD, "wood");
    
    // Ceiling beams
    for (int dx = -halfW; dx <= halfW; dx++) {
        PlaceBlock(world, cx + dx, cy + height, cz - halfD, "wood");
        PlaceBlock(world, cx + dx, cy + height, cz + halfD, "wood");
    }
    for (int dz = -halfD; dz <= halfD; dz++) {
        PlaceBlock(world, cx - halfW, cy + height, cz + dz, "wood");
        PlaceBlock(world, cx + halfW, cy + height, cz + dz, "wood");
    }
}

void MineshaftGenerator::GenerateLadderShaft(World& world, const Vec3& top, int depth) {
    int tx = static_cast<int>(top.x);
    int ty = static_cast<int>(top.y);
    int tz = static_cast<int>(top.z);
    
    for (int d = 0; d < depth; d++) {
        int y = ty - d;
        
        // Carve 2x2 shaft
        for (int dx = 0; dx <= 1; dx++) {
            for (int dz = 0; dz <= 1; dz++) {
                if (IsUndergroundBlock(world, tx + dx, y, tz + dz)) {
                    world.SetBlock(tx + dx, y, tz + dz, BLOCK_AIR);
                }
            }
        }
        
        // Place ladder on one wall
        PlaceBlock(world, tx, y, tz, "ladder");
    }
}

void MineshaftGenerator::GenerateMineshaftSystem(World& world, const Vec3& start, int numSegments) {
    if (!IsValidMineshaftPosition(world, start)) return;
    
    segments.clear();
    
    Vec3 currentPos = start;
    
    // Generate main corridor
    for (int i = 0; i < numSegments; i++) {
        // Random direction (mostly horizontal)
        Vec3 direction(
            RandomFloat(-1.0f, 1.0f),
            RandomFloat(-0.3f, 0.3f),  // Slight vertical variation
            RandomFloat(-1.0f, 1.0f)
        );
        direction = direction.normalize();
        
        float segmentLength = RandomFloat(8.0f, 20.0f);
        Vec3 endPos = currentPos + direction * segmentLength;
        
        // Clamp Y to valid range
        if (endPos.y < 8) endPos.y = 8;
        if (endPos.y > 40) endPos.y = 40;
        
        float width = RandomFloat(2.0f, 3.5f);
        float height = RandomFloat(2.5f, 3.5f);
        bool hasRails = (RandomFloat(0, 1) < 0.6f);  // 60% chance for rails
        bool hasSupports = (RandomFloat(0, 1) < 0.8f);  // 80% chance for supports
        
        // Store segment info
        MineshaftSegment seg;
        seg.start = currentPos;
        seg.end = endPos;
        seg.width = width;
        seg.height = height;
        seg.hasRails = hasRails;
        seg.hasSupports = hasSupports;
        segments.push_back(seg);
        
        // Generate corridor
        GenerateCorridor(world, currentPos, endPos, width, height);
        
        // Add supports
        if (hasSupports) {
            GenerateSupports(world, currentPos, endPos, width, height);
        }
        
        // Add rails
        if (hasRails) {
            GenerateRails(world, currentPos, endPos);
        }
        
        // Chance for room/hub
        if (RandomFloat(0, 1) < 0.2f) {
            int roomWidth = RandomInt(4, 7);
            int roomHeight = RandomInt(3, 5);
            int roomDepth = RandomInt(4, 7);
            GenerateRoom(world, endPos, roomWidth, roomHeight, roomDepth);
            
            // Place loot chest in room
            if (RandomFloat(0, 1) < 0.5f) {
                int chestX = static_cast<int>(endPos.x) + RandomInt(-1, 1);
                int chestY = static_cast<int>(endPos.y) + 1;
                int chestZ = static_cast<int>(endPos.z) + RandomInt(-1, 1);
                PlaceLootChest(world, chestX, chestY, chestZ);
            }
        }
        
        // Chance for cave spider spawner
        if (RandomFloat(0, 1) < 0.15f) {
            int spawnerX = static_cast<int>(endPos.x);
            int spawnerY = static_cast<int>(endPos.y) + 1;
            int spawnerZ = static_cast<int>(endPos.z);
            PlaceCaveSpiderSpawner(world, spawnerX, spawnerY, spawnerZ);
        }
        
        // Chance for ladder shaft
        if (RandomFloat(0, 1) < 0.1f && endPos.y > 12) {
            int shaftDepth = RandomInt(5, 10);
            GenerateLadderShaft(world, endPos, shaftDepth);
        }
        
        // Branching - 25% chance
        if (RandomFloat(0, 1) < 0.25f && i < numSegments - 1) {
            Vec3 branchDir(
                RandomFloat(-1.0f, 1.0f),
                RandomFloat(-0.2f, 0.2f),
                RandomFloat(-1.0f, 1.0f)
            );
            branchDir = branchDir.normalize();
            
            float branchLength = RandomFloat(5.0f, 12.0f);
            Vec3 branchEnd = currentPos + branchDir * branchLength;
            
            if (branchEnd.y < 8) branchEnd.y = 8;
            if (branchEnd.y > 40) branchEnd.y = 40;
            
            float branchWidth = RandomFloat(1.5f, 2.5f);
            float branchHeight = RandomFloat(2.0f, 3.0f);
            
            GenerateCorridor(world, currentPos, branchEnd, branchWidth, branchHeight);
            GenerateSupports(world, currentPos, branchEnd, branchWidth, branchHeight);
            
            if (RandomFloat(0, 1) < 0.4f) {
                GenerateRails(world, currentPos, branchEnd);
            }
        }
        
        currentPos = endPos;
    }
}

void MineshaftGenerator::GenerateMineshafts(World& world, int chunkRadius) {
    for (int cx = -chunkRadius; cx < chunkRadius; cx++) {
        for (int cz = -chunkRadius; cz < chunkRadius; cz++) {
            // Chance to spawn a mineshaft in this chunk area
            if (RandomFloat(0, 1) > 0.15f) continue;  // 15% chance per chunk
            
            Vec3 start(
                cx * CHUNK_SIZE + RandomFloat(5, CHUNK_SIZE - 5),
                RandomFloat(12, 35),  // Underground depth
                cz * CHUNK_SIZE + RandomFloat(5, CHUNK_SIZE - 5)
            );
            
            if (!IsValidMineshaftPosition(world, start)) continue;
            
            int numSegments = RandomInt(3, 8);
            GenerateMineshaftSystem(world, start, numSegments);
        }
    }
}

} // namespace vge
