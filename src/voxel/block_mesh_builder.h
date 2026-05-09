#pragma once
#include <vector>
#include "rendering/mesh.h"

namespace vge {

class Chunk;

class BlockMeshBuilder {
public:
    static void BuildChunkMesh(const Chunk* chunk, Mesh& mesh);
    static void AddCube(Mesh& mesh, const Vec3& position, const Vec3& color);
    static void AddFace(Mesh& mesh, const Vec3& position, const Vec3& normal, const Vec3& color);
};

} // namespace vge