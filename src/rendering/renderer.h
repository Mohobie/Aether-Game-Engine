#pragma once
#include "rendering/mesh.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "voxel/world.h"

namespace vge {

class FramebufferRenderer;

class Renderer {
private:
    bool initialized;
    int width;
    int height;
    FramebufferRenderer* fb_renderer;
    
    void RenderWorldASCII(const World& world, const Camera& camera);
    void RenderWorldFB(const World& world, const Camera& camera);
    uint32_t GetBlockColor(BlockType type);
    
public:
    Renderer();
    ~Renderer();
    
    bool Initialize();
    void Shutdown();
    
    void BeginFrame();
    void EndFrame();
    
    void SetClearColor(float r, float g, float b, float a);
    void SetViewport(int x, int y, int w, int h);
    
    void RenderMesh(const Mesh& mesh, const Shader& shader, const Camera& camera);
    void RenderWorld(const World& world, const Camera& camera);
};

} // namespace vge