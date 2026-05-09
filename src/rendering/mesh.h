#pragma once

#include <vector>
#include "math/vec3.h"

namespace VoxelEngine {
    struct Vertex {
        Vec3 position;
        Vec3 normal;
        float u, v;
    };
    
    class Mesh {
    public:
        void AddVertex(const Vertex& v);
        void Upload();
        void Draw();
    private:
        std::vector<Vertex> vertices;
        unsigned int vao, vbo;
    };
}
