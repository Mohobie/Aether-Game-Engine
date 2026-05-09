#include "voxel/block_mesh_builder.h"
#include "voxel/chunk.h"
#include "voxel/block.h"
#include "voxel/block_registry.h"
#include "rendering/mesh.h"
#include <iostream>

namespace vge {

void BlockMeshBuilder::BuildChunkMesh(const Chunk* chunk, Mesh& mesh) {
    if (!chunk || chunk->IsEmpty()) return;
    
    int cx = chunk->GetChunkX();
    int cy = chunk->GetChunkY();
    int cz = chunk->GetChunkZ();
    
    // Iterate all blocks in chunk
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                BlockType block = chunk->GetBlock(x, y, z);
                if (block == BlockType::Air) continue;
                
                Vec3 pos(x, y, z);
                Vec3 color = GetBlockColor((int)block);
                
                // Check each face - only generate if adjacent to air
                // Left face (x-1)
                if (x == 0 || chunk->GetBlock(x-1, y, z) == BlockType::Air) {
                    AddFace(mesh, pos, Vec3(-1, 0, 0), color);
                }
                // Right face (x+1)
                if (x == CHUNK_SIZE-1 || chunk->GetBlock(x+1, y, z) == BlockType::Air) {
                    AddFace(mesh, pos, Vec3(1, 0, 0), color);
                }
                // Bottom face (y-1)
                if (y == 0 || chunk->GetBlock(x, y-1, z) == BlockType::Air) {
                    AddFace(mesh, pos, Vec3(0, -1, 0), color);
                }
                // Top face (y+1)
                if (y == CHUNK_SIZE-1 || chunk->GetBlock(x, y+1, z) == BlockType::Air) {
                    AddFace(mesh, pos, Vec3(0, 1, 0), color);
                }
                // Back face (z-1)
                if (z == 0 || chunk->GetBlock(x, y, z-1) == BlockType::Air) {
                    AddFace(mesh, pos, Vec3(0, 0, -1), color);
                }
                // Front face (z+1)
                if (z == CHUNK_SIZE-1 || chunk->GetBlock(x, y, z+1) == BlockType::Air) {
                    AddFace(mesh, pos, Vec3(0, 0, 1), color);
                }
            }
        }
    }
}

Vec3 BlockMeshBuilder::GetBlockColor(int type) {
    switch (type) {
        case 3: return Vec3(0.2f, 0.8f, 0.2f);    // Grass - Green
        case 2: return Vec3(0.6f, 0.4f, 0.2f);     // Dirt - Brown
        case 1: return Vec3(0.5f, 0.5f, 0.5f);     // Stone - Gray
        case 6: return Vec3(0.4f, 0.25f, 0.1f);   // Wood - Dark brown
        case 7: return Vec3(0.1f, 0.6f, 0.1f);    // Leaves - Dark green
        case 4: return Vec3(0.9f, 0.85f, 0.6f);    // Sand - Tan
        case 5: return Vec3(0.2f, 0.4f, 0.9f);    // Water - Blue
        case 14: return Vec3(0.2f, 0.2f, 0.2f);   // Bedrock - Dark gray
        default: return Vec3(1.0f, 0.0f, 1.0f);    // Magenta (unknown)
    }
}

void BlockMeshBuilder::AddFace(Mesh& mesh, const Vec3& position, const Vec3& normal, const Vec3& color) {
    // Calculate face vertices based on normal
    Vec3 v0, v1, v2, v3;
    
    if (normal.x != 0) {
        // Left or right face
        float x = position.x + (normal.x > 0 ? 1.0f : 0.0f);
        v0 = Vec3(x, position.y, position.z);
        v1 = Vec3(x, position.y + 1, position.z);
        v2 = Vec3(x, position.y + 1, position.z + 1);
        v3 = Vec3(x, position.y, position.z + 1);
    } else if (normal.y != 0) {
        // Bottom or top face
        float y = position.y + (normal.y > 0 ? 1.0f : 0.0f);
        v0 = Vec3(position.x, y, position.z);
        v1 = Vec3(position.x, y, position.z + 1);
        v2 = Vec3(position.x + 1, y, position.z + 1);
        v3 = Vec3(position.x + 1, y, position.z);
    } else {
        // Back or front face
        float z = position.z + (normal.z > 0 ? 1.0f : 0.0f);
        v0 = Vec3(position.x, position.y, z);
        v1 = Vec3(position.x + 1, position.y, z);
        v2 = Vec3(position.x + 1, position.y + 1, z);
        v3 = Vec3(position.x, position.y + 1, z);
    }
    
    // Add vertices (two triangles = one face)
    Vertex vert0, vert1, vert2, vert3;
    
    vert0.position = v0;
    vert0.normal = normal;
    vert0.color = color;
    vert0.texCoord = Vec2(0, 0);
    
    vert1.position = v1;
    vert1.normal = normal;
    vert1.color = color;
    vert1.texCoord = Vec2(1, 0);
    
    vert2.position = v2;
    vert2.normal = normal;
    vert2.color = color;
    vert2.texCoord = Vec2(1, 1);
    
    vert3.position = v3;
    vert3.normal = normal;
    vert3.color = color;
    vert3.texCoord = Vec2(0, 1);
    
    // Add triangles (counter-clockwise)
    uint32_t baseIndex = mesh.GetVertexCount();
    
    mesh.AddVertex(vert0);
    mesh.AddVertex(vert1);
    mesh.AddVertex(vert2);
    mesh.AddVertex(vert3);
    
    mesh.AddIndex(baseIndex + 0);
    mesh.AddIndex(baseIndex + 1);
    mesh.AddIndex(baseIndex + 2);
    
    mesh.AddIndex(baseIndex + 0);
    mesh.AddIndex(baseIndex + 2);
    mesh.AddIndex(baseIndex + 3);
}

void BlockMeshBuilder::AddCube(Mesh& mesh, const Vec3& position, const Vec3& color) {
    // Add all 6 faces
    AddFace(mesh, position, Vec3(-1, 0, 0), color); // Left
    AddFace(mesh, position, Vec3(1, 0, 0), color);  // Right
    AddFace(mesh, position, Vec3(0, -1, 0), color); // Bottom
    AddFace(mesh, position, Vec3(0, 1, 0), color);  // Top
    AddFace(mesh, position, Vec3(0, 0, -1), color); // Back
    AddFace(mesh, position, Vec3(0, 0, 1), color);    // Front
}

} // namespace vge