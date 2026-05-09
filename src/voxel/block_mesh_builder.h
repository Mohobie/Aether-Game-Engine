#pragma once
#include <vector>
#include "math/vec3.h"

namespace vge {
struct Vertex {
    Vec3 position;
    Vec3 color;
};

class Chunk;
class BlockMeshBuilder {
public:
    static std::vector<Vertex> BuildChunkMesh(const Chunk* chunk);
};
}