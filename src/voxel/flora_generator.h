#pragma once
#include "voxel/world.h"
#include "math/vec3.h"
#include <string>

namespace vge {

// ============================================
// Flora Generator (Grass, Flowers, Cacti, Lily Pads, Mushrooms)
// ============================================
class FloraGenerator {
public:
    FloraGenerator();
    ~FloraGenerator();

    // Generate flora in world
    void GenerateFlora(World& world, int chunkRadius);
    
    // Create specific flora
    void CreateGrass(World& world, const Vec3& position);
    void CreateFlower(World& world, const Vec3& position, const std::string& type);
    void CreateCactus(World& world, const Vec3& position);
    void CreateTallGrass(World& world, const Vec3& position);
    void CreateLilyPad(World& world, const Vec3& position);
    void CreateMushroom(World& world, const Vec3& position, const std::string& type);

private:
    float RandomFloat(float min, float max);
    int RandomInt(int min, int max);
    
    // Get surface Y at position
    int FindSurfaceY(World& world, int x, int z);
};

} // namespace vge
