#include "voxel/world_renderer.h"
#include "voxel/chunk.h"
#include "voxel/world.h"
#include "rendering/camera.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include <iostream>

// Software-based world renderer (no OpenGL required)
// Uses ASCII output for now, ready for GL upgrade

namespace vge {

WorldRenderer::WorldRenderer() : shader(nullptr) {}

WorldRenderer::~WorldRenderer() {}

void WorldRenderer::Initialize(Shader* shaderProgram) {
    shader = shaderProgram;
    std::cout << "[WorldRenderer] Initialized (software mode)" << std::endl;
}

void WorldRenderer::RenderChunk(const Chunk* chunk, const Camera& camera) {
    if (!chunk || chunk->IsEmpty()) return;
    
    // Would render chunk mesh using OpenGL
    // For now, just track dirty chunks
    if (chunk->IsDirty()) {
        std::cout << "[WorldRenderer] Chunk at (" << chunk->GetChunkX() << ", " 
                  << chunk->GetChunkY() << ", " << chunk->GetChunkZ() << ") needs mesh update" << std::endl;
    }
}

void WorldRenderer::RenderWorld(const World& world, const Camera& camera) {
    // Would iterate visible chunks and render them
    // For now, delegate to ASCII renderer in main engine
    // This will be replaced with OpenGL calls when available
}

void WorldRenderer::UpdateChunkMesh(const Chunk* chunk) {
    if (!chunk) return;
    
    // Would rebuild chunk mesh using BlockMeshBuilder
    // For now, just log
    std::cout << "[WorldRenderer] Updated mesh for chunk at ("
              << chunk->GetChunkX() << ", "
              << chunk->GetChunkY() << ", "
              << chunk->GetChunkZ() << ")" << std::endl;
}

void WorldRenderer::Cleanup() {
    chunkMeshes.clear();
    std::cout << "[WorldRenderer] Cleaned up chunk meshes" << std::endl;
}

} // namespace vge