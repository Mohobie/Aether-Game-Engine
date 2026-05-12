#include "mesh.h"
namespace vge {
void Mesh::addVertex(const Vertex& v) { vertices.push_back(v); }
void Mesh::addTriangle(uint32_t a, uint32_t b, uint32_t c) { indices.push_back(a); indices.push_back(b); indices.push_back(c); }
void Mesh::clear() { vertices.clear(); indices.clear(); }
bool Mesh::isEmpty() const { return vertices.empty(); }
} // namespace vge
