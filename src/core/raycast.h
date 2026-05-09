#pragma once
#include "math/vec3.h"
#include "voxel/block.h"

namespace vge {

class World;

struct RaycastHit {
    bool hit;
    Vec3 position;       // Exact hit position
    Vec3 blockPosition;  // Block coordinates
    Vec3 normal;         // Face normal
    float distance;      // Distance from origin
    BlockType blockType; // Type of block hit
    
    RaycastHit() : hit(false), distance(0), blockType(BlockType::Air) {}
};

class Raycast {
public:
    float maxDistance;
    
    Raycast();
    
    // Cast a ray and return hit info
    RaycastHit Cast(const Vec3& origin, const Vec3& direction, World& world);
    
    // Place a block at ray hit position + normal
    bool PlaceBlock(const Vec3& origin, const Vec3& direction, World& world, BlockType type);
    
    // Remove block at ray hit position
    bool RemoveBlock(const Vec3& origin, const Vec3& direction, World& world);
};

} // namespace vge