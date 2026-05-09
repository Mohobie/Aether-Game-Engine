#include "renderer.h"
#include <iostream>
namespace aether {
void Renderer::init() { std::cout << "[Renderer] Initialized" << std::endl; }
void Renderer::shutdown() { std::cout << "[Renderer] Shutdown" << std::endl; }
void Renderer::beginFrame() {}
void Renderer::endFrame() {}
void Renderer::renderMesh(const Mesh& mesh, const Mat4& transform) {}
void Renderer::setCamera(const Camera& camera) { currentCamera = camera; }
} // namespace aether
