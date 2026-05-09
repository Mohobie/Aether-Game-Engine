#pragma once
#include <vector>
#include <cstdint>
#include "math/vec3.h"

namespace vge {

struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec3 color;
    Vec2 texCoord;
};

class Mesh {
private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t vao, vbo, ebo;
    bool uploaded;
    
public:
    Mesh();
    ~Mesh();
    
    void AddVertex(const Vertex& v);
    void AddIndex(uint32_t i);
    void UploadToGPU();
    void Draw() const;
    void Clear();
};

} // namespace vge