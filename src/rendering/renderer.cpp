#include "rendering/renderer.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/mesh.h"
#include <iostream>

// Stub implementation when OpenGL is not available
namespace vge {

Renderer::Renderer() : initialized(false), width(1280), height(720) {}

Renderer::~Renderer() {
    if (initialized) Shutdown();
}

bool Renderer::Initialize() {
    std::cout << "[Renderer] Stub - would initialize OpenGL" << std::endl;
    initialized = true;
    return true;
}

void Renderer::Shutdown() {
    std::cout << "[Renderer] Stub - shutting down" << std::endl;
    initialized = false;
}

void Renderer::BeginFrame() {
    // Stub - would clear screen
}

void Renderer::EndFrame() {
    // Stub - would swap buffers
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
    // Stub - would set OpenGL clear color
}

void Renderer::SetViewport(int x, int y, int w, int h) {
    width = w;
    height = h;
}

void Renderer::RenderMesh(const Mesh& mesh, const Shader& shader, const Camera& camera) {
    // Stub - would render mesh
}

} // namespace vge