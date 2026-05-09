#pragma once
#include "chunk.h"
#include "rendering/mesh.h"

namespace VoxelEngine {
    class BlockMeshBuilder {
    public:
        Mesh BuildChunkMesh(const Chunk& chunk);
        void AddFace(Mesh& mesh, const Vec3& pos, int face, BlockID block);
    private:
        static constexpr float BLOCK_SIZE = 1.0f;
    };
}
