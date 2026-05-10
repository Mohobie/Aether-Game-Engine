#include "block_mesh_builder.h"
#include "block_registry.h"
#include "chunk.h"

namespace vge {

Mesh BlockMeshBuilder::BuildChunkMesh(const Chunk& chunk) {
    Mesh mesh;
    
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                BlockTypeID block = chunk.GetBlock(x, y, z);
                if (block == BLOCK_AIR) continue;
                
                Vec3 color = GetBlockColor(block);
                
                // Check each face - only render if neighbor is air or transparent
                // Front face (Z+)
                if (z == CHUNK_SIZE - 1 || chunk.GetBlock(x, y, z + 1) == BLOCK_AIR) {
                    AddFaceFront(mesh, x, y, z, color);
                }
                // Back face (Z-)
                if (z == 0 || chunk.GetBlock(x, y, z - 1) == BLOCK_AIR) {
                    AddFaceBack(mesh, x, y, z, color);
                }
                // Right face (X+)
                if (x == CHUNK_SIZE - 1 || chunk.GetBlock(x + 1, y, z) == BLOCK_AIR) {
                    AddFaceRight(mesh, x, y, z, color);
                }
                // Left face (X-)
                if (x == 0 || chunk.GetBlock(x - 1, y, z) == BLOCK_AIR) {
                    AddFaceLeft(mesh, x, y, z, color);
                }
                // Top face (Y+)
                if (y == CHUNK_SIZE - 1 || chunk.GetBlock(x, y + 1, z) == BLOCK_AIR) {
                    AddFaceTop(mesh, x, y, z, color);
                }
                // Bottom face (Y-)
                if (y == 0 || chunk.GetBlock(x, y - 1, z) == BLOCK_AIR) {
                    AddFaceBottom(mesh, x, y, z, color);
                }
            }
        }
    }
    
    return mesh;
}

Mesh BlockMeshBuilder::BuildChunkMesh(const Chunk& chunk,
                                      const Chunk* neighborXP, const Chunk* neighborXN,
                                      const Chunk* neighborYP, const Chunk* neighborYN,
                                      const Chunk* neighborZP, const Chunk* neighborZN) {
    Mesh mesh;
    const Chunk* neighbors[6] = {neighborXP, neighborXN, neighborYP, neighborYN, neighborZP, neighborZN};
    
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                BlockTypeID block = chunk.GetBlock(x, y, z);
                if (block == BLOCK_AIR) continue;
                
                AddBlockFaces(mesh, x, y, z, block, chunk, neighbors);
            }
        }
    }
    
    return mesh;
}

void BlockMeshBuilder::AddBlockFaces(Mesh& mesh, int x, int y, int z,
                                     BlockTypeID block, const Chunk& chunk,
                                     const Chunk* neighbors[6]) {
    Vec3 color = GetBlockColor(block);
    
    // Front face (Z+)
    BlockTypeID front = (z < CHUNK_SIZE - 1) ? chunk.GetBlock(x, y, z + 1) 
                                             : (neighbors[4] ? neighbors[4]->GetBlock(x, y, 0) : BLOCK_AIR);
    if (ShouldRenderFace(front)) AddFaceFront(mesh, x, y, z, color);
    
    // Back face (Z-)
    BlockTypeID back = (z > 0) ? chunk.GetBlock(x, y, z - 1)
                              : (neighbors[5] ? neighbors[5]->GetBlock(x, y, CHUNK_SIZE - 1) : BLOCK_AIR);
    if (ShouldRenderFace(back)) AddFaceBack(mesh, x, y, z, color);
    
    // Right face (X+)
    BlockTypeID right = (x < CHUNK_SIZE - 1) ? chunk.GetBlock(x + 1, y, z)
                                              : (neighbors[0] ? neighbors[0]->GetBlock(0, y, z) : BLOCK_AIR);
    if (ShouldRenderFace(right)) AddFaceRight(mesh, x, y, z, color);
    
    // Left face (X-)
    BlockTypeID left = (x > 0) ? chunk.GetBlock(x - 1, y, z)
                                : (neighbors[1] ? neighbors[1]->GetBlock(CHUNK_SIZE - 1, y, z) : BLOCK_AIR);
    if (ShouldRenderFace(left)) AddFaceLeft(mesh, x, y, z, color);
    
    // Top face (Y+)
    BlockTypeID top = (y < CHUNK_SIZE - 1) ? chunk.GetBlock(x, y + 1, z)
                                            : (neighbors[2] ? neighbors[2]->GetBlock(x, 0, z) : BLOCK_AIR);
    if (ShouldRenderFace(top)) AddFaceTop(mesh, x, y, z, color);
    
    // Bottom face (Y-)
    BlockTypeID bottom = (y > 0) ? chunk.GetBlock(x, y - 1, z)
                                  : (neighbors[3] ? neighbors[3]->GetBlock(x, CHUNK_SIZE - 1, z) : BLOCK_AIR);
    if (ShouldRenderFace(bottom)) AddFaceBottom(mesh, x, y, z, color);
}

bool BlockMeshBuilder::ShouldRenderFace(BlockTypeID neighborBlock) {
    if (neighborBlock == BLOCK_AIR) return true;
    const BlockDef& def = BlockRegistry::GetInstance().GetBlock(neighborBlock);
    return !def.IsOpaque(); // Render face if neighbor is transparent
}

Vec3 BlockMeshBuilder::GetBlockColor(BlockTypeID block) {
    if (block == BLOCK_AIR) return Vec3(0, 0, 0);
    return BlockRegistry::GetInstance().GetBlock(block).GetColor();
}

// Face generation helpers
void BlockMeshBuilder::AddFaceFront(Mesh& mesh, int x, int y, int z, const Vec3& color) {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fz = static_cast<float>(z);
    
    uint32_t base = mesh.GetVertexCount();
    mesh.AddVertex({Vec3(fx, fy, fz + 1), Vec3(0, 0, 1), color, Vec2(0, 0)});
    mesh.AddVertex({Vec3(fx + 1, fy, fz + 1), Vec3(0, 0, 1), color, Vec2(1, 0)});
    mesh.AddVertex({Vec3(fx + 1, fy + 1, fz + 1), Vec3(0, 0, 1), color, Vec2(1, 1)});
    mesh.AddVertex({Vec3(fx, fy + 1, fz + 1), Vec3(0, 0, 1), color, Vec2(0, 1)});
    
    mesh.AddIndex(base);
    mesh.AddIndex(base + 1);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base + 3);
}

void BlockMeshBuilder::AddFaceBack(Mesh& mesh, int x, int y, int z, const Vec3& color) {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fz = static_cast<float>(z);
    
    uint32_t base = mesh.GetVertexCount();
    mesh.AddVertex({Vec3(fx + 1, fy, fz), Vec3(0, 0, -1), color, Vec2(0, 0)});
    mesh.AddVertex({Vec3(fx, fy, fz), Vec3(0, 0, -1), color, Vec2(1, 0)});
    mesh.AddVertex({Vec3(fx, fy + 1, fz), Vec3(0, 0, -1), color, Vec2(1, 1)});
    mesh.AddVertex({Vec3(fx + 1, fy + 1, fz), Vec3(0, 0, -1), color, Vec2(0, 1)});
    
    mesh.AddIndex(base);
    mesh.AddIndex(base + 1);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base + 3);
}

void BlockMeshBuilder::AddFaceRight(Mesh& mesh, int x, int y, int z, const Vec3& color) {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fz = static_cast<float>(z);
    
    uint32_t base = mesh.GetVertexCount();
    mesh.AddVertex({Vec3(fx + 1, fy, fz + 1), Vec3(1, 0, 0), color, Vec2(0, 0)});
    mesh.AddVertex({Vec3(fx + 1, fy, fz), Vec3(1, 0, 0), color, Vec2(1, 0)});
    mesh.AddVertex({Vec3(fx + 1, fy + 1, fz), Vec3(1, 0, 0), color, Vec2(1, 1)});
    mesh.AddVertex({Vec3(fx + 1, fy + 1, fz + 1), Vec3(1, 0, 0), color, Vec2(0, 1)});
    
    mesh.AddIndex(base);
    mesh.AddIndex(base + 1);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base + 3);
}

void BlockMeshBuilder::AddFaceLeft(Mesh& mesh, int x, int y, int z, const Vec3& color) {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fz = static_cast<float>(z);
    
    uint32_t base = mesh.GetVertexCount();
    mesh.AddVertex({Vec3(fx, fy, fz), Vec3(-1, 0, 0), color, Vec2(0, 0)});
    mesh.AddVertex({Vec3(fx, fy, fz + 1), Vec3(-1, 0, 0), color, Vec2(1, 0)});
    mesh.AddVertex({Vec3(fx, fy + 1, fz + 1), Vec3(-1, 0, 0), color, Vec2(1, 1)});
    mesh.AddVertex({Vec3(fx, fy + 1, fz), Vec3(-1, 0, 0), color, Vec2(0, 1)});
    
    mesh.AddIndex(base);
    mesh.AddIndex(base + 1);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base + 3);
}

void BlockMeshBuilder::AddFaceTop(Mesh& mesh, int x, int y, int z, const Vec3& color) {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fz = static_cast<float>(z);
    
    uint32_t base = mesh.GetVertexCount();
    mesh.AddVertex({Vec3(fx, fy + 1, fz + 1), Vec3(0, 1, 0), color, Vec2(0, 0)});
    mesh.AddVertex({Vec3(fx + 1, fy + 1, fz + 1), Vec3(0, 1, 0), color, Vec2(1, 0)});
    mesh.AddVertex({Vec3(fx + 1, fy + 1, fz), Vec3(0, 1, 0), color, Vec2(1, 1)});
    mesh.AddVertex({Vec3(fx, fy + 1, fz), Vec3(0, 1, 0), color, Vec2(0, 1)});
    
    mesh.AddIndex(base);
    mesh.AddIndex(base + 1);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base + 3);
}

void BlockMeshBuilder::AddFaceBottom(Mesh& mesh, int x, int y, int z, const Vec3& color) {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    float fz = static_cast<float>(z);
    
    uint32_t base = mesh.GetVertexCount();
    mesh.AddVertex({Vec3(fx, fy, fz), Vec3(0, -1, 0), color, Vec2(0, 0)});
    mesh.AddVertex({Vec3(fx + 1, fy, fz), Vec3(0, -1, 0), color, Vec2(1, 0)});
    mesh.AddVertex({Vec3(fx + 1, fy, fz + 1), Vec3(0, -1, 0), color, Vec2(1, 1)});
    mesh.AddVertex({Vec3(fx, fy, fz + 1), Vec3(0, -1, 0), color, Vec2(0, 1)});
    
    mesh.AddIndex(base);
    mesh.AddIndex(base + 1);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base);
    mesh.AddIndex(base + 2);
    mesh.AddIndex(base + 3);
}

} // namespace vge
