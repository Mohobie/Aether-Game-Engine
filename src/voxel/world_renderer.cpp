#include "world_renderer.h"
#include "chunk_mesh_builder.h"
#include "block.h"
#include "chunk.h"
#include "world.h"
#include <iostream>

namespace vge {

WorldRenderer::WorldRenderer() : shader(nullptr) {}

WorldRenderer::~WorldRenderer() {}

void WorldRenderer::Initialize(Shader* shaderProgram) {
    shader = shaderProgram;
    std::cout << "[WorldRenderer] Initialized" << std::endl;
}

void WorldRenderer::RenderChunk(const Chunk* chunk, const Camera& camera) {
    if (!chunk || chunk->IsEmpty()) return;
    
    // Get or build mesh for this chunk
    auto it = chunkMeshes.find(chunk);
    if (it == chunkMeshes.end() || chunk->IsDirty()) {
        // Rebuild mesh
        Mesh mesh;
        BlockMeshBuilder::BuildChunkMesh(chunk, mesh);
        chunkMeshes[chunk] = std::move(mesh);
        chunk->SetDirty(false);
    }
    
    // Render the mesh
    const Mesh& mesh = chunkMeshes[chunk];
    
    // Set up shader uniforms
    if (shader) {
        Mat4 model = Mat4::Translate(Vec3(
            chunk->GetChunkX() * CHUNK_SIZE,
            chunk->GetChunkY() * CHUNK_SIZE,
            chunk->GetChunkZ() * CHUNK_SIZE
        ));
        
        shader->SetMat4("model", model);
        shader->SetMat4("view", camera.GetViewMatrix());
        shader->SetMat4("projection", camera.GetProjectionMatrix());
    }
    
    mesh.Draw();
}

void WorldRenderer::RenderWorld(const World& world, const Camera& camera) {
    // Render all loaded chunks
    // TODO: Implement world chunk iteration
    // For now, this is a stub that would iterate the world's chunk map
}

void WorldRenderer::UpdateChunkMesh(const Chunk* chunk) {
    if (!chunk) return;
    
    // Mark for rebuild
    auto it = chunkMeshes.find(chunk);
    if (it != chunkMeshes.end()) {
        chunkMeshes.erase(it);
    }
}

void WorldRenderer::Cleanup() {
    chunkMeshes.clear();
}

} // namespace vge