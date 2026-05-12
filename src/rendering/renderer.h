#pragma once
#include "rendering/mesh.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "voxel/world.h"
#include <GL/gl.h>

namespace vge {

class FramebufferRenderer;
class SkyRenderer;
class WeatherEffectsRenderer;
class DayNightCycle;
class WeatherSystem;
class WorldRenderer;

// Modern OpenGL renderer with VAOs/VBOs
class Renderer {
private:
    bool initialized;
    int width;
    int height;
    
    // OpenGL resources
    uint32_t cubeVAO;
    uint32_t cubeVBO;
    uint32_t crosshairVAO;
    uint32_t crosshairVBO;
    uint32_t highlightVAO;
    uint32_t highlightVBO;
    
    // Sub-renderers
    FramebufferRenderer* fb_renderer;
    SkyRenderer* sky_renderer;
    WeatherEffectsRenderer* weather_renderer;
    DayNightCycle* day_night_cycle;
    WeatherSystem* weather_system;
    WorldRenderer* world_renderer;
    Shader* world_shader;
    
    void InitializeCubeBuffers();
    void InitializeCrosshairBuffers();
    void InitializeHighlightBuffers();
    void CleanupBuffers();
    
    void RenderWorldModern(const World& world, const Camera& camera);
    void RenderWorldASCII(const World& world, const Camera& camera);
    void RenderWorldFB(const World& world, const Camera& camera);
    void RenderWorldMesh(const World& world, const Camera& camera);
    uint32_t GetBlockColor(BlockTypeID type);
    
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
    
    // World renderer integration
    WorldRenderer* GetWorldRenderer() const { return world_renderer; }
    void UpdateChunkMesh(const Chunk* chunk);
    void UpdateChunkMeshWithNeighbors(const Chunk* chunk,
                                       const Chunk* neighborXP, const Chunk* neighborXN,
                                       const Chunk* neighborYP, const Chunk* neighborYN,
                                       const Chunk* neighborZP, const Chunk* neighborZN);
    
    // Weather/Sky integration
    void SetDayNightCycle(DayNightCycle* cycle) { day_night_cycle = cycle; }
    void SetWeatherSystem(WeatherSystem* weather) { weather_system = weather; }
    void RenderSky(const Camera& camera);
    void RenderWeatherEffects(const Camera& camera);
    
    // Crosshair and block highlight rendering
    void RenderCrosshair(int screenW, int screenH);
    void RenderBlockHighlight(const Vec3& blockPos, const Camera& camera, int screenW, int screenH);
    
    // Get screen dimensions
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    
    bool IsInitialized() const { return initialized; }
};

} // namespace vge
