#include "block_mesh_builder.h"
#include "chunk.h"
#include "block.h"

namespace vge {
std::vector<Vertex> BlockMeshBuilder::BuildChunkMesh(const Chunk* chunk) {
    std::vector<Vertex> vertices;
    if (!chunk || chunk->IsEmpty()) return vertices;
    
    for (int x = 0; x < Chunk::CHUNK_SIZE; ++x) {
        for (int y = 0; y < Chunk::CHUNK_SIZE; ++y) {
            for (int z = 0; z < Chunk::CHUNK_SIZE; ++z) {
                BlockType block = chunk->GetBlock(x, y, z);
                if (block == BlockType::Air) continue;
                
                BlockProperties props = Block::GetProperties(block);
                Vec3 color = props.color;
                Vec3 pos(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                
                // Add face vertices (simplified - just one face for now)
                vertices.push_back({pos + Vec3(0, 0, 0), color});
                vertices.push_back({pos + Vec3(1, 0, 0), color});
                vertices.push_back({pos + Vec3(1, 1, 0), color});
                vertices.push_back({pos + Vec3(0, 0, 0), color});
                vertices.push_back({pos + Vec3(1, 1, 0), color});
                vertices.push_back({pos + Vec3(0, 1, 0), color});
            }
        }
    }
    return vertices;
}
}