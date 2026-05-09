#pragma once
#include "camera.h"
#include "mesh.h"
#include "shader.h"
namespace aether {
class Renderer {
public:
    void init();
    void shutdown();
    void beginFrame();
    void endFrame();
    void renderMesh(const Mesh& mesh, const Mat4& transform);
    void setCamera(const Camera& camera);
private:
    Camera currentCamera;
    Shader defaultShader;
};
} // namespace aether
