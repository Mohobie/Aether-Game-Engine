#pragma once

#include "math/vec3.h"
#include "math/mat4.h"
#include "rendering/camera.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include "rendering/post_processing.h"
#include "rendering/deferred_rendering.h"
#include "rendering/light_system.h"
#include <GL/gl.h>
#include <memory>
#include <vector>

namespace vge {

class World;
class WorldRenderer;
class SkyRenderer;
class WeatherEffectsRenderer;
class DayNightCycle;
class WeatherSystem;

// ============================================
// Modern OpenGL Renderer
// ============================================
// Unified render pipeline supporting:
// - Forward rendering (opaque objects)
// - Deferred rendering (lighting)
// - Post-processing stack
// - HDR rendering

enum class RenderPath {
    Forward,      // Standard forward rendering
    Deferred,     // G-Buffer + deferred lighting
    ForwardPlus   // Tiled/clustered forward (future)
};

class ModernRenderer {
public:
    ModernRenderer();
    ~ModernRenderer();

    // Lifecycle
    bool Initialize(int width, int height);
    void Shutdown();
    void Resize(int width, int height);

    // Frame rendering
    void BeginFrame();
    void EndFrame();

    // Scene rendering
    void RenderWorld(const World& world, const Camera& camera);
    void RenderSky(const Camera& camera);
    void RenderWeatherEffects(const Camera& camera);
    void RenderEntities(const Camera& camera);

    // Post-processing
    void SetPostProcessEnabled(bool enabled);
    PostProcessStack* GetPostProcessStack() { return postProcess.get(); }

    // Lighting
    void SetLightingSystem(LightingSystem* lights) { lightingSystem = lights; }
    void SetDayNightCycle(DayNightCycle* cycle) { dayNightCycle = cycle; }
    void SetWeatherSystem(WeatherSystem* weather) { weatherSystem = weather; }

    // Render path
    void SetRenderPath(RenderPath path);
    RenderPath GetRenderPath() const { return currentPath; }

    // Debug
    void SetWireframe(bool enabled);
    void VisualizeGBuffer();

    // Getters
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool IsInitialized() const { return initialized; }

    // Legacy compatibility
    void SetClearColor(float r, float g, float b, float a);
    void SetViewport(int x, int y, int w, int h);
    void RenderCrosshair(int screenW, int screenH);
    void RenderBlockHighlight(const Vec3& blockPos, const Camera& camera, int screenW, int screenH);

private:
    bool initialized;
    int width, height;
    RenderPath currentPath;

    // OpenGL state
    GLuint hdrFBO;
    GLuint hdrTexture;
    GLuint depthBuffer;

    // Render paths
    std::unique_ptr<DeferredRenderer> deferredRenderer;
    std::unique_ptr<PostProcessStack> postProcess;

    // Sub-renderers
    WorldRenderer* worldRenderer;
    SkyRenderer* skyRenderer;
    WeatherEffectsRenderer* weatherRenderer;
    DayNightCycle* dayNightCycle;
    WeatherSystem* weatherSystem;
    LightingSystem* lightingSystem;

    // Shaders
    std::unique_ptr<Shader> forwardShader;
    std::unique_ptr<Shader> skyShader;

    // Screen quad for post-processing
    GLuint screenQuadVAO;
    GLuint screenQuadVBO;

    // Debug
    bool wireframeMode;

    void InitializeFramebuffers();
    void ShutdownFramebuffers();
    void CreateScreenQuad();
    void DestroyScreenQuad();

    void RenderForward(const World& world, const Camera& camera);
    void RenderDeferred(const World& world, const Camera& camera);
    void RenderPostProcess();
    void RenderToScreen();

    void InitializeOpenGL();
};

} // namespace vge
