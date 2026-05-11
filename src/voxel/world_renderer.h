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
    void UpdateChunkMeshWithNeighbors(const Chunk* chunk,
                                       const Chunk* neighborXP, const Chunk* neighborXN,
                                       const Chunk* neighborYP, const Chunk* neighborYN,
                                       const Chunk* neighborZP, const Chunk* neighborZN);
    
    bool HasMesh(const Chunk* chunk) const;
    size_t GetMeshCount() const;
    void RemoveChunkMesh(const Chunk* chunk);
    void Cleanup();
};

} // namespace vge
