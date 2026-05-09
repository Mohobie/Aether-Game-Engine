#pragma once
#include <unordered_map>
#include "rendering/mesh.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "math/mat4.h"

namespace vge {

class Chunk;
class World;

class WorldRenderer {
private:
    std::unordered_map<const Chunk*, Mesh> chunkMeshes;
    Shader* shader;
    
public:
    WorldRenderer();
    ~WorldRenderer();
    
    void Initialize(Shader* shaderProgram);
    void RenderChunk(const Chunk* chunk, const Camera& camera);
    void RenderWorld(const World& world, const Camera& camera);
    void UpdateChunkMesh(const Chunk* chunk);
    void Cleanup();
};

} // namespace vge