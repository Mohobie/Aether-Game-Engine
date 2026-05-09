#include "rendering/mesh.h"
#include <iostream>

// Stub implementation when OpenGL is not available
namespace vge {

Mesh::Mesh() : vao(0), vbo(0), ebo(0), uploaded(false) {}

Mesh::~Mesh() {
    // Would delete OpenGL buffers
}

void Mesh::AddVertex(const Vertex& v) {
    vertices.push_back(v);
}

void Mesh::AddIndex(uint32_t i) {
    indices.push_back(i);
}

void Mesh::UploadToGPU() {
    if (uploaded) return;
    std::cout << "[Mesh] Stub - would upload " << vertices.size() << " vertices to GPU" << std::endl;
    uploaded = true;
}

void Mesh::Draw() const {
    if (!uploaded) return;
    // Would draw with OpenGL
}

void Mesh::Clear() {
    vertices.clear();
    indices.clear();
    uploaded = false;
}

} // namespace vge