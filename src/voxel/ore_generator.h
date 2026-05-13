#pragma once
#include "voxel/world.h"
#include "math/vec3.h"
#include <vector>
#include <string>

namespace vge {

// ============================================
// Ore Vein Generator
// ============================================
class OreGenerator {
public:
    OreGenerator();
    ~OreGenerator();

    // Generate ore veins in world
    void GenerateOres(World& world, int chunkRadius);
    
    // Generate a single ore vein
    void GenerateOreVein(World& world, const Vec3& center, const std::string& oreType, 
                         int size, float density);

private:
    float RandomFloat(float min, float max);
    int RandomInt(int min, int max);
    
    // Get ore block ID from type name
    std::string GetOreBlockId(const std::string& oreType);
    
    // Get valid Y range for ore type
    void GetOreDepthRange(const std::string& oreType, int& minY, int& maxY);
};

} // namespace vge
