#pragma once
#include "math/vec3.h"
#include "math/mat4.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include <vector>
#include <memory>

namespace vge {

// Shadow map for a single light
class ShadowMap {
private:
    uint32_t framebufferID;
    uint32_t textureID;
    int resolution;
    Mat4 lightSpaceMatrix;
    
public:
    ShadowMap(int resolution = 2048);
    ~ShadowMap();
    
    bool Initialize();
    void Shutdown();
    
    // Begin shadow rendering
    void BeginRender(const Vec3& lightPos, const Vec3& lightDir, 
                     const Vec3& sceneCenter, float sceneRadius);
    void EndRender();
    
    // Get shadow texture
    uint32_t GetTextureID() const { return textureID; }
    const Mat4& GetLightSpaceMatrix() const { return lightSpaceMatrix; }
    
    // Bind for reading in shader
    void BindTexture(int slot = 3);
    
    int GetResolution() const { return resolution; }
};

// Cascaded Shadow Maps for directional lights
struct Cascade {
    float splitDistance;
    Mat4 lightSpaceMatrix;
    ShadowMap* shadowMap;
};

class CascadedShadowMap {
private:
    std::vector<Cascade> cascades;
    int numCascades;
    int resolution;
    
public:
    CascadedShadowMap(int numCascades = 4, int resolution = 2048);
    ~CascadedShadowMap();
    
    bool Initialize();
    void Shutdown();
    
    // Calculate cascade splits based on camera frustum
    void CalculateSplits(float nearPlane, float farPlane);
    
    // Render each cascade
    void BeginCascadeRender(int cascadeIndex, const Vec3& lightDir,
                           const Mat4& cameraView, const Mat4& cameraProj);
    void EndCascadeRender();
    
    // Get cascade data for shader
    const std::vector<Cascade>& GetCascades() const { return cascades; }
    int GetNumCascades() const { return numCascades; }
    
    // Find which cascade a point belongs to
    int GetCascadeIndex(float depth) const;
};

// Shadow system manager
class ShadowSystem {
private:
    std::vector<std::unique_ptr<ShadowMap>> pointShadowMaps;
    std::unique_ptr<CascadedShadowMap> directionalShadows;
    
    Shader* shadowShader;
    bool enabled;
    float shadowBias;
    float shadowStrength;
    
public:
    ShadowSystem();
    ~ShadowSystem();
    
    bool Initialize();
    void Shutdown();
    
    // Create shadow map for a light
    ShadowMap* CreateShadowMap(int resolution = 2048);
    CascadedShadowMap* CreateCascadedShadowMap(int numCascades = 4, int resolution = 2048);
    
    // Render shadows
    void RenderShadows(const std::vector<Light>& lights, 
                      const std::vector<vge::Mesh*>& opaqueMeshes,
                      const Vec3& cameraPos,
                      const Mat4& cameraView,
                      const Mat4& cameraProj);
    
    // Apply shadows in main render
    void BindShadowMaps(Shader& shader);
    
    // Settings
    void SetEnabled(bool enable) { enabled = enable; }
    bool IsEnabled() const { return enabled; }
    void SetBias(float bias) { shadowBias = bias; }
    float GetBias() const { return shadowBias; }
    void SetStrength(float strength) { shadowStrength = strength; }
    float GetStrength() const { return shadowStrength; }
    
    // Stats
    int GetShadowMapCount() const;
};

} // namespace vge
