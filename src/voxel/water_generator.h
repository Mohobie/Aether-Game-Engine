#pragma once
#include "voxel/world.h"
#include "math/vec3.h"
#include <vector>

namespace vge {

// ============================================
// Water Generator (Rivers & Lakes)
// ============================================
class WaterGenerator {
public:
    WaterGenerator();
    ~WaterGenerator();

    // Generate lakes
    void GenerateLakes(World& world, int chunkRadius, int numLakes);
    
    // Generate rivers
    void GenerateRivers(World& world, int chunkRadius, int numRivers);
    
    // Create a lake at position
    void CreateLake(World& world, const Vec3& center, float radius, int depth);
    
    // Create a river from start to end
    void CreateRiver(World& world, const Vec3& start, const Vec3& end, float width);

private:
    float RandomFloat(float min, float max);
    int RandomInt(int min, int max);
    
    // Carve terrain for water
    void CarveLakeBed(World& world, const Vec3& center, float radius, int depth);
    void CarveRiverBed(World& world, const Vec3& start, const Vec3& end, float width);
    
    // Fill with water
    void FillWithWater(World& world, const Vec3& center, float radius, int waterLevel);
    void FillRiverWithWater(World& world, const Vec3& start, const Vec3& end, float width, int waterLevel);
};

} // namespace vge
