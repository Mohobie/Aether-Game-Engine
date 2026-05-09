#include "voxel/world_renderer.h"
#include "voxel/chunk.h"
#include "voxel/world.h"
#include "rendering/camera.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include <iostream>

// Stub implementation when OpenGL is not available
namespace vge {

WorldRenderer::WorldRenderer() : shader(nullptr) {}

WorldRenderer::~WorldRenderer() {}

void WorldRenderer::Initialize(Shader* shaderProgram) {
    shader = shaderProgram;
    std::cout << "[WorldRenderer] Initialized" << std::endl;
}

void WorldRenderer::RenderChunk(const Chunk* chunk, const Camera& camera) {
    if (!chunk || chunk->IsEmpty()) return;
    
    // Would render chunk mesh here
}

void WorldRenderer::RenderWorld(const World& world, const Camera& camera) {
    // Would iterate and render all chunks
}

void WorldRenderer::UpdateChunkMesh(const Chunk* chunk) {
    if (!chunk) return;
    
    // Would rebuild chunk mesh
}

void WorldRenderer::Cleanup() {
    chunkMeshes.clear();
}

} // namespace vge