#pragma once
#include "voxel/world.h"
#include "math/vec3.h"
#include <vector>
#include <string>

namespace vge {

// ============================================
// Mineshaft Generator
// ============================================
// Generates underground mineshaft structures with:
// - Wooden support beams
// - Rail tracks
// - Loot chests
// - Cave spider spawners
// - Integration with existing cave generation
// ============================================

struct MineshaftSegment {
    Vec3 start;
    Vec3 end;
    float width;
    float height;
    bool hasRails;
    bool hasSupports;
};

class MineshaftGenerator {
public:
    MineshaftGenerator();
    ~MineshaftGenerator();

    // Generate mineshafts in world (integrates with cave generation)
    void GenerateMineshafts(World& world, int chunkRadius);
    
    // Generate a mineshaft system starting from a position
    void GenerateMineshaftSystem(World& world, const Vec3& start, int numSegments);
    
    // Generate a single corridor segment
    void GenerateCorridor(World& world, const Vec3& start, const Vec3& end, float width, float height);
    
    // Generate wooden supports in a corridor
    void GenerateSupports(World& world, const Vec3& start, const Vec3& end, float width, float height);
    
    // Generate rails along a corridor
    void GenerateRails(World& world, const Vec3& start, const Vec3& end);
    
    // Place a loot chest at position
    void PlaceLootChest(World& world, int x, int y, int z);
    
    // Place a cave spider spawner
    void PlaceCaveSpiderSpawner(World& world, int x, int y, int z);
    
    // Check if position is suitable for mineshaft (underground, not in water/lava)
    bool IsValidMineshaftPosition(World& world, const Vec3& pos);

private:
    // Random number generation
    float RandomFloat(float min, float max);
    int RandomInt(int min, int max);
    
    // Place block helper
    void PlaceBlock(World& world, int x, int y, int z, const std::string& blockId);
    
    // Check if block is a solid underground block (stone, dirt, gravel)
    bool IsUndergroundBlock(World& world, int x, int y, int z);
    
    // Generate a room/hub area
    void GenerateRoom(World& world, const Vec3& center, int width, int height, int depth);
    
    // Generate a ladder shaft (vertical connection)
    void GenerateLadderShaft(World& world, const Vec3& top, int depth);
    
    std::vector<MineshaftSegment> segments;
};

} // namespace vge
