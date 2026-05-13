#pragma once
#include "voxel/world.h"

namespace vge {

class WorldGenerator {
public:
    // Generate a large flat world with terrain features
    static void GenerateFlatWorld(World& world, int size);
    
    // Generate a hilly terrain world
    static void GenerateHillyWorld(World& world, int size);
    
    // Generate a world with trees and water
    static void GenerateForestWorld(World& world, int size);
    
    // Generate a desert world
    static void GenerateDesertWorld(World& world, int size);
    
private:
    // Simple pseudo-random number generator (deterministic)
    static float Noise(int x, int z, int seed);
    static float SmoothNoise(int x, int z, int seed);
    static float Interpolate(float a, float b, float t);
    
    // Helper functions
    static void GenerateTree(World& world, int x, int y, int z);
    static void GenerateHill(World& world, int centerX, int centerZ, int height, int radius);
};

} // namespace vge
