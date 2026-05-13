#pragma once
#include "voxel/world.h"
#include "math/vec3.h"
#include <string>

namespace vge {

// ============================================
// Structure Generator
// ============================================
class StructureGenerator {
public:
    StructureGenerator();
    ~StructureGenerator();

    // Generate trees
    void GenerateTrees(World& world, int chunkRadius, int treeDensity);
    
    // Generate a single tree at position
    void CreateTree(World& world, const Vec3& position);
    
    // Generate a small house
    void CreateHouse(World& world, const Vec3& position);
    
    // Generate a dungeon
    void CreateDungeon(World& world, const Vec3& position);

private:
    float RandomFloat(float min, float max);
    int RandomInt(int min, int max);
    
    // Place block helper
    void PlaceBlock(World& world, int x, int y, int z, const std::string& blockId);
};

} // namespace vge
