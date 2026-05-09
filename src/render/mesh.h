#pragma once
#include "core/math.h"
#include <vector>
namespace aether {
struct Vertex {
    Vec3 position;
    Vec2 texCoord;
    uint32_t color;
};
class Mesh {
public:
    void addVertex(const Vertex& v);
    void addTriangle(uint32_t a, uint32_t b, uint32_t c);
    void clear();
    bool isEmpty() const;
private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
} // namespace aether
