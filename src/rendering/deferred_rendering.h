#pragma once
#include "math/vec3.h"
#include "math/vec2.h"
#include "math/mat4.h"
#include <vector>
#include <memory>
#include <string>

namespace vge {

// ============================================
// G-Buffer - Geometry Buffer for Deferred Rendering
// ============================================
// Stores per-pixel geometry data for deferred shading

// G-Buffer texture attachments
enum class GBufferAttachment {
    Position = 0,       // RGB: view-space position, A: linear depth
    Normal = 1,         // RGB: view-space normal, A: roughness
    Albedo = 2,         // RGB: base color, A: metallic
    Emissive = 3,       // RGB: emissive color, A: ambient occlusion
    Depth = 4,          // Depth buffer
    Count = 5
};

struct GBufferTexture {
    unsigned int textureID;
    unsigned int internalFormat;
    unsigned int format;
    unsigned int type;
    std::string name;
    bool enabled;
};

class GBuffer {
private:
    unsigned int fbo;
    unsigned int depthTexture;
    std::vector<GBufferTexture> attachments;
    
    int width;
    int height;
    bool initialized;
    
public:
    GBuffer();
    ~GBuffer();
    
    // Initialize with screen dimensions
    void Initialize(int width, int height);
    void Shutdown();
    void Resize(int width, int height);
    
    // Bind for geometry pass
    void BindForGeometryPass();
    
    // Bind for lighting pass (read-only)
    void BindForLightingPass();
    
    // Bind specific texture for reading
    void BindTexture(GBufferAttachment attachment, unsigned int slot) const;
    
    // Getters
    unsigned int GetFBO() const { return fbo; }
    unsigned int GetTexture(GBufferAttachment attachment) const;
    unsigned int GetDepthTexture() const { return depthTexture; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    
    // Copy depth to another FBO
    void CopyDepthTo(unsigned int targetFBO);
    
    // Debug
    void PrintInfo() const;
};

// ============================================
// Deferred Renderer
// ============================================
class DeferredRenderer {
private:
    GBuffer gbuffer;
    
    // Lighting pass FBO (accumulates lighting)
    unsigned int lightingFBO;
    unsigned int lightingTexture;
    
    // Screen quad for lighting pass
    unsigned int screenQuadVAO;
    unsigned int screenQuadVBO;
    
    // Shaders
    unsigned int geometryShader;
    unsigned int lightingShader;
    unsigned int stencilShader;
    
    // Settings
    bool useStencilOptimization;
    bool useTiledDeferred;
    int tileSize;
    
public:
    DeferredRenderer();
    ~DeferredRenderer();
    
    void Initialize(int width, int height);
    void Shutdown();
    void Resize(int width, int height);
    
    // Rendering phases
    void BeginGeometryPass();
    void EndGeometryPass();
    
    void BeginLightingPass();
    void RenderDirectionalLight(const Vec3& direction, const Vec3& color, float intensity);
    void RenderPointLight(const Vec3& position, const Vec3& color, float intensity, float radius);
    void RenderSpotLight(const Vec3& position, const Vec3& direction, const Vec3& color, 
                         float intensity, float radius, float angle);
    void EndLightingPass();
    
    // Get output
    unsigned int GetLightingTexture() const { return lightingTexture; }
    unsigned int GetGBufferTexture(GBufferAttachment attachment) const;
    
    // Settings
    void EnableStencilOptimization(bool enable) { useStencilOptimization = enable; }
    void EnableTiledDeferred(bool enable) { useTiledDeferred = enable; }
    
    // Debug
    void VisualizeGBuffer();
};

// ============================================
// Light Volumes (for stencil optimization)
// ============================================
class LightVolume {
private:
    unsigned int sphereVAO;
    unsigned int sphereVBO;
    int sphereVertexCount;
    
    unsigned int coneVAO;
    unsigned int coneVBO;
    int coneVertexCount;
    
public:
    LightVolume();
    ~LightVolume();
    
    void Initialize();
    void Shutdown();
    
    // Render sphere for point light
    void RenderSphere(const Vec3& position, float radius);
    
    // Render cone for spot light
    void RenderCone(const Vec3& position, const Vec3& direction, 
                    float angle, float radius);
};

// ============================================
// Tiled Deferred Shading
// ============================================
struct LightTile {
    std::vector<int> lightIndices;
};

class TiledDeferred {
private:
    int tileSize;
    int tileCountX;
    int tileCountY;
    std::vector<LightTile> tiles;
    
    // Compute shader for tile classification
    unsigned int tileComputeShader;
    unsigned int tileSSBO;
    
public:
    TiledDeferred();
    ~TiledDeferred();
    
    void Initialize(int screenWidth, int screenHeight, int tileSize = 16);
    void Shutdown();
    
    // Build light list for each tile
    void BuildLightLists(const std::vector<class PointLight>& lights,
                         const Mat4& viewProjection);
    
    // Get lights for a tile
    const std::vector<int>& GetLightsForTile(int tileX, int tileY) const;
    
    // Settings
    void SetTileSize(int size) { tileSize = size; }
    int GetTileSize() const { return tileSize; }
};

// ============================================
// Screen-Space Ambient Occlusion (SSAO) - Updated for G-Buffer
// ============================================
class SSAODeferred {
private:
    int kernelSize;
    float radius;
    float bias;
    float intensity;
    
    std::vector<Vec3> kernelSamples;
    std::vector<Vec3> noiseTexture;
    
    unsigned int ssaoFBO;
    unsigned int ssaoTexture;
    unsigned int blurFBO;
    unsigned int blurTexture;
    unsigned int noiseTextureID;
    
    bool initialized;
    
public:
    SSAODeferred();
    ~SSAODeferred();
    
    void Initialize(int width, int height);
    void Shutdown();
    
    // Parameters
    void SetKernelSize(int size) { kernelSize = size; GenerateKernel(); }
    void SetRadius(float r) { radius = r; }
    void SetBias(float b) { bias = b; }
    void SetIntensity(float i) { intensity = i; }
    
    // Generate sample kernel
    void GenerateKernel();
    void GenerateNoiseTexture();
    
    // Render SSAO using G-Buffer data
    void Render(const GBuffer& gbuffer, const Mat4& projection);
    void Blur();
    
    // Get output texture
    unsigned int GetSSAOTexture() const { return blurTexture; }
    unsigned int GetRawSSAOTexture() const { return ssaoTexture; }
};

// ============================================
// Screen-Space Reflections (SSR)
// ============================================
class ScreenSpaceReflections {
private:
    int maxSteps;
    float maxDistance;
    float thickness;
    float stride;
    
    unsigned int ssrFBO;
    unsigned int ssrTexture;
    unsigned int hitPointTexture;
    
    bool initialized;
    
public:
    ScreenSpaceReflections();
    ~ScreenSpaceReflections();
    
    void Initialize(int width, int height);
    void Shutdown();
    
    // Parameters
    void SetMaxSteps(int steps) { maxSteps = steps; }
    void SetMaxDistance(float dist) { maxDistance = dist; }
    void SetThickness(float t) { thickness = t; }
    void SetStride(float s) { stride = s; }
    
    // Render SSR using G-Buffer
    void Render(const GBuffer& gbuffer, const Mat4& view, const Mat4& projection);
    
    // Get output
    unsigned int GetReflectionTexture() const { return ssrTexture; }
};

// ============================================
// Light Types
// ============================================
struct DirectionalLight {
    Vec3 direction;
    Vec3 color;
    float intensity;
    bool castShadows;
    Mat4 lightSpaceMatrix;
};

struct PointLight {
    Vec3 position;
    Vec3 color;
    float intensity;
    float radius;
    bool castShadows;
};

struct SpotLight {
    Vec3 position;
    Vec3 direction;
    Vec3 color;
    float intensity;
    float radius;
    float innerAngle;
    float outerAngle;
    bool castShadows;
    Mat4 lightSpaceMatrix;
};

// ============================================
// Light Manager
// ============================================
class LightManager {
private:
    std::vector<DirectionalLight> directionalLights;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;
    
    int maxDirectionalLights;
    int maxPointLights;
    int maxSpotLights;
    
public:
    LightManager();
    
    // Add lights
    void AddDirectionalLight(const DirectionalLight& light);
    void AddPointLight(const PointLight& light);
    void AddSpotLight(const SpotLight& light);
    
    // Remove lights
    void RemoveDirectionalLight(int index);
    void RemovePointLight(int index);
    void RemoveSpotLight(int index);
    
    // Getters
    const std::vector<DirectionalLight>& GetDirectionalLights() const;
    const std::vector<PointLight>& GetPointLights() const;
    const std::vector<SpotLight>& GetSpotLights() const;
    
    // Clear all
    void Clear();
    
    // Count
    int GetDirectionalLightCount() const { return directionalLights.size(); }
    int GetPointLightCount() const { return pointLights.size(); }
    int GetSpotLightCount() const { return spotLights.size(); }
};

} // namespace vge
