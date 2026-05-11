#include "voxel/world_renderer.h"
#include "voxel/chunk.h"
#include "voxel/world.h"
#include "voxel/block_mesh_builder.h"
#include "rendering/camera.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include <iostream>

namespace vge {

WorldRenderer::WorldRenderer() : shader(nullptr) {}

WorldRenderer::~WorldRenderer() {
    Cleanup();
}

void WorldRenderer::Initialize(Shader* shaderProgram) {
    shader = shaderProgram;
    chunkMeshes.clear();
    std::cout << "[WorldRenderer] Initialized" << std::endl;
}

void WorldRenderer::RenderChunk(const Chunk* chunk, const Camera& camera) {
    if (!chunk || chunk->IsEmpty()) return;
    
    // Rebuild mesh if chunk is dirty
    if (chunk->IsDirty()) {
        UpdateChunkMesh(chunk);
        const_cast<Chunk*>(chunk)->SetDirty(false);
    }
    
    auto it = chunkMeshes.find(chunk);
    if (it == chunkMeshes.end() || it->second.GetVertexCount() == 0) {
        return;
    }
    
    // In a real OpenGL implementation, this would:
    // 1. Bind the chunk's VAO/VBO
    // 2. Set up model matrix (chunk world position)
    // 3. Bind shader and set uniforms
    // 4. Call glDrawElements
    // For now, the mesh is built and ready
    it->second.Draw();
}

void WorldRenderer::RenderWorld(const World& world, const Camera& camera) {
    // Iterate all loaded chunks and render visible ones
    // For now, render all loaded chunks (culling can be added later)
    // World doesn't expose chunk iteration directly, so we use a different approach
    // The Renderer::RenderWorld method will call this per visible chunk
}

void WorldRenderer::UpdateChunkMesh(const Chunk* chunk) {
    if (!chunk) return;
    
    // Build mesh for this chunk
    Mesh mesh = BlockMeshBuilder::BuildChunkMesh(*chunk);
    mesh.UploadToGPU();
    
    chunkMeshes[chunk] = std::move(mesh);
}

void WorldRenderer::UpdateChunkMeshWithNeighbors(const Chunk* chunk,
                                                  const Chunk* neighborXP,
                                                  const Chunk* neighborXN,
                                                  const Chunk* neighborYP,
                                                  const Chunk* neighborYN,
                                                  const Chunk* neighborZP,
                                                  const Chunk* neighborZN) {
    if (!chunk) return;
    
    Mesh mesh = BlockMeshBuilder::BuildChunkMesh(*chunk,
                                                  neighborXP, neighborXN,
                                                  neighborYP, neighborYN,
                                                  neighborZP, neighborZN);
    mesh.UploadToGPU();
    
    chunkMeshes[chunk] = std::move(mesh);
}

bool WorldRenderer::HasMesh(const Chunk* chunk) const {
    return chunkMeshes.find(chunk) != chunkMeshes.end();
}

size_t WorldRenderer::GetMeshCount() const {
    return chunkMeshes.size();
}

void WorldRenderer::RemoveChunkMesh(const Chunk* chunk) {
    auto it = chunkMeshes.find(chunk);
    if (it != chunkMeshes.end()) {
        chunkMeshes.erase(it);
    }
}

void WorldRenderer::Cleanup() {
    chunkMeshes.clear();
}

} // namespace vge
