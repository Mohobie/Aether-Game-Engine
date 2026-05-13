#pragma once
#include "voxel/world.h"
#include "math/vec3.h"
#include <vector>

namespace vge {

// ============================================
// Cave Generator
// ============================================
class CaveGenerator {
public:
    CaveGenerator();
    ~CaveGenerator();

    // Generate caves in world
    void GenerateCaves(World& world, int chunkRadius);
    
    // Generate a single cave tunnel
    void GenerateCaveTunnel(World& world, const Vec3& start, const Vec3& end, float radius);
    
    // Generate cave system starting from a point
    void GenerateCaveSystem(World& world, const Vec3& start, int numTunnels, float maxLength);

private:
    // Random number generation
    float RandomFloat(float min, float max);
    int RandomInt(int min, int max);
    
    // Carve a sphere at position
    void CarveSphere(World& world, const Vec3& center, float radius);
    
    // Check if position is valid for cave
    bool IsValidCavePosition(const Vec3& pos);
};

} // namespace vge
