#pragma once
#include "voxel/chunk.h"
#include "rendering/mesh.h"
#include "voxel/block_types.h"

namespace vge {

// Generates mesh data for chunks
class BlockMeshBuilder {
public:
    // Build a mesh for a single chunk
    static Mesh BuildChunkMesh(const Chunk& chunk);
    
    // Build mesh for multiple chunks (with face culling between chunks)
    static Mesh BuildChunkMesh(const Chunk& chunk, 
                               const Chunk* neighborXP, const Chunk* neighborXN,
                               const Chunk* neighborYP, const Chunk* neighborYN,
                               const Chunk* neighborZP, const Chunk* neighborZN);
    
    // Add a single block's faces to a mesh
    static void AddBlockFaces(Mesh& mesh, int x, int y, int z, 
                              BlockTypeID block, const Chunk& chunk,
                              const Chunk* neighbors[6]);
    
private:
    // Face vertices (position, normal, color, texCoord)
    static void AddFaceFront(Mesh& mesh, int x, int y, int z, const Vec3& color);
    static void AddFaceBack(Mesh& mesh, int x, int y, int z, const Vec3& color);
    static void AddFaceLeft(Mesh& mesh, int x, int y, int z, const Vec3& color);
    static void AddFaceRight(Mesh& mesh, int x, int y, int z, const Vec3& color);
    static void AddFaceTop(Mesh& mesh, int x, int y, int z, const Vec3& color);
    static void AddFaceBottom(Mesh& mesh, int x, int y, int z, const Vec3& color);
    
    // Check if a block face should be rendered (neighbor is air or transparent)
    static bool ShouldRenderFace(BlockTypeID neighborBlock);
    
    // Get block color from registry
    static Vec3 GetBlockColor(BlockTypeID block);
};

} // namespace vge
