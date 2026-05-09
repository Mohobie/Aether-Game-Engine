#include "raycast.h"
#include "voxel/world.h"
#include "voxel/block.h"
#include <iostream>

namespace vge {

Raycast::Raycast() : maxDistance(100.0f) {}

RaycastHit Raycast::Cast(const Vec3& origin, const Vec3& direction, World& world) {
    RaycastHit hit;
    hit.hit = false;
    hit.distance = maxDistance;
    
    // Normalize direction
    Vec3 dir = direction.Normalized();
    
    // Step through the grid
    Vec3 pos = origin;
    float stepSize = 0.1f; // Small steps for accuracy
    
    for (float dist = 0; dist < maxDistance; dist += stepSize) {
        pos = origin + (dir * dist);
        
        // Convert to block coordinates
        int blockX = (int)pos.x;
        int blockY = (int)pos.y;
        int blockZ = (int)pos.z;
        
        // Check if solid block
        BlockType block = world.GetBlock(blockX, blockY, blockZ);
        if (block != BlockType::Air && BlockRegistry::GetInstance().GetBlock(block).IsSolid()) {
            hit.hit = true;
            hit.position = pos;
            hit.blockPosition = Vec3((float)blockX, (float)blockY, (float)blockZ);
            hit.blockType = block;
            hit.distance = dist;
            
            // Calculate normal (approximate based on which face was hit)
            Vec3 center(blockX + 0.5f, blockY + 0.5f, blockZ + 0.5f);
            Vec3 diff = pos - center;
            
            // Find dominant axis
            if (std::abs(diff.x) > std::abs(diff.y) && std::abs(diff.x) > std::abs(diff.z)) {
                hit.normal = Vec3(diff.x > 0 ? 1.0f : -1.0f, 0, 0);
            } else if (std::abs(diff.y) > std::abs(diff.z)) {
                hit.normal = Vec3(0, diff.y > 0 ? 1.0f : -1.0f, 0);
            } else {
                hit.normal = Vec3(0, 0, diff.z > 0 ? 1.0f : -1.0f);
            }
            
            return hit;
        }
    }
    
    return hit;
}

bool Raycast::PlaceBlock(const Vec3& origin, const Vec3& direction, World& world, BlockType type) {
    RaycastHit hit = Cast(origin, direction, world);
    
    if (!hit.hit) {
        return false; // No block to place against
    }
    
    // Place block adjacent to hit face
    Vec3 placePos = hit.blockPosition + hit.normal;
    world.SetBlock((int)placePos.x, (int)placePos.y, (int)placePos.z, type);
    
    return true;
}

bool Raycast::RemoveBlock(const Vec3& origin, const Vec3& direction, World& world) {
    RaycastHit hit = Cast(origin, direction, world);
    
    if (!hit.hit) {
        return false; // No block to remove
    }
    
    // Remove the block
    world.SetBlock((int)hit.blockPosition.x, (int)hit.blockPosition.y, (int)hit.blockPosition.z, BlockType::Air);
    
    return true;
}

} // namespace vge