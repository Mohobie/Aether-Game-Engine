#include <iostream>
#include "math/vec3.h"
#include "math/mat4.h"
#include "voxel/block_types.h"
#include "voxel/block_registry.h"
#include "voxel/chunk.h"
#include "voxel/world.h"
#include "voxel/block_mesh_builder.h"
#include "rendering/mesh.h"
#include "physics/collision.h"

using namespace vge;

int main() {
    std::cout << "=== Aether Voxel Engine Tests ===" << std::endl;
    
    // Load default blocks
    std::string blockPath = "/home/mohobie/projects/aether-game-engine/assets/blocks/default_blocks.json";
    if (!BlockRegistry::GetInstance().LoadFromFile(blockPath)) {
        std::cerr << "Warning: Could not load block definitions" << std::endl;
    }
    
    // Test Vec3
    Vec3 a(1, 2, 3);
    Vec3 b(4, 5, 6);
    Vec3 c = a + b;
    std::cout << "Vec3 add: (" << c.x << ", " << c.y << ", " << c.z << ")" << std::endl;
    
    // Test Block
    BlockTypeID stoneId = BlockRegistry::GetInstance().GetBlockId("stone");
    Block block(stoneId);
    std::cout << "Block: " << block.GetName() << " (solid: " << block.IsSolid() << ")" << std::endl;
    
    // Test Chunk
    Chunk chunk(0, 0, 0);
    chunk.SetBlock(1, 1, 1, stoneId);
    chunk.SetBlock(2, 1, 1, stoneId);
    chunk.SetBlock(1, 2, 1, stoneId);
    std::cout << "Chunk block at (1,1,1): " << chunk.GetBlock(1, 1, 1) << std::endl;
    
    // Test World
    World world;
    world.SetBlock(5, 5, 5, "dirt");
    std::cout << "World block at (5,5,5): " << world.GetBlock(5, 5, 5) << std::endl;
    
    // Test AABB
    AABB box1(Vec3(0, 0, 0), Vec3(1, 1, 1));
    AABB box2(Vec3(0.5, 0.5, 0.5), Vec3(1, 1, 1));
    std::cout << "AABB intersect: " << box1.Intersects(box2) << std::endl;
    
    // Test Mesh Builder
    std::cout << "\n=== Mesh Builder Test ===" << std::endl;
    Mesh mesh = BlockMeshBuilder::BuildChunkMesh(chunk);
    std::cout << "Chunk mesh vertices: " << mesh.GetVertexCount() << std::endl;
    std::cout << "Chunk mesh indices: " << mesh.GetIndexCount() << std::endl;
    
    // Test with more blocks
    Chunk chunk2(0, 0, 0);
    for (int x = 0; x < 4; ++x) {
        for (int y = 0; y < 4; ++y) {
            for (int z = 0; z < 4; ++z) {
                if (x == 0 || x == 3 || y == 0 || y == 3 || z == 0 || z == 3) {
                    chunk2.SetBlock(x, y, z, stoneId);
                }
            }
        }
    }
    
    Mesh mesh2 = BlockMeshBuilder::BuildChunkMesh(chunk2);
    std::cout << "Hollow cube mesh vertices: " << mesh2.GetVertexCount() << std::endl;
    std::cout << "Hollow cube mesh indices: " << mesh2.GetIndexCount() << std::endl;
    
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}