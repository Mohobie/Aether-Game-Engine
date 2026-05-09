#pragma once
#include "rendering/mesh.h"
#include "rendering/shader.h"
#include "rendering/camera.h"

namespace vge {
    class Renderer {
    private:
        bool initialized;
        int width;
        int height;
        
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
    };
}