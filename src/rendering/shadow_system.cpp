#include "shadow_system.h"
#include "mesh.h"
#include "rendering/shader.h"
#include <iostream>
#include <cmath>

namespace vge {

// ============================================
// Shadow Map Implementation
// ============================================

ShadowMap::ShadowMap(int res) : framebufferID(0), textureID(0), resolution(res) {
    lightSpaceMatrix = Mat4(); // Identity
}

ShadowMap::~ShadowMap() {
    Shutdown();
}

bool ShadowMap::Initialize() {
    std::cout << "[ShadowMap] Creating " << resolution << "x" << resolution << " shadow map" << std::endl;
    
    // In a real implementation, this would:
    // 1. glGenFramebuffers(1, &framebufferID)
    // 2. glGenTextures(1, &textureID)
    // 3. Setup depth texture
    // 4. Attach to framebuffer
    
    // Stub: just mark as initialized
    return true;
}

void ShadowMap::Shutdown() {
    // Would delete OpenGL resources
    framebufferID = 0;
    textureID = 0;
}

void ShadowMap::BeginRender(const Vec3& lightPos, const Vec3& lightDir,
                            const Vec3& sceneCenter, float sceneRadius) {
    // Calculate light space matrix (orthographic projection for directional light)
    Vec3 up(0, 1, 0);
    if (std::abs(lightDir.y) > 0.99f) {
        up = Vec3(1, 0, 0);
    }
    
    // View matrix from light's perspective
    Vec3 lightTarget = sceneCenter;
    Vec3 lightEye = sceneCenter - lightDir * (sceneRadius * 2.0f);
    
    // Simple look-at matrix
    Vec3 zAxis = (lightEye - lightTarget).normalize();
    Vec3 xAxis = Vec3(
        up.y * zAxis.z - up.z * zAxis.y,
        up.z * zAxis.x - up.x * zAxis.z,
        up.x * zAxis.y - up.y * zAxis.x
    ).normalize();
    Vec3 yAxis = Vec3(
        zAxis.y * xAxis.z - zAxis.z * xAxis.y,
        zAxis.z * xAxis.x - zAxis.x * xAxis.z,
        zAxis.x * xAxis.y - zAxis.y * xAxis.x
    );
    
    // Orthographic projection
    float orthoSize = sceneRadius;
    float nearPlane = 1.0f;
    float farPlane = sceneRadius * 4.0f;
    
    // Build light space matrix (projection * view)
    // For now, store a simplified version
    lightSpaceMatrix = Mat4(); // Identity as placeholder
    
    // Would bind framebuffer and clear depth
    std::cout << "[ShadowMap] Rendering from light perspective" << std::endl;
}

void ShadowMap::EndRender() {
    // Would unbind framebuffer
}

void ShadowMap::BindTexture(int slot) {
    // Would glActiveTexture(GL_TEXTURE0 + slot)
    // Would glBindTexture(GL_TEXTURE_2D, textureID)
}

// ============================================
// Cascaded Shadow Map Implementation
// ============================================

CascadedShadowMap::CascadedShadowMap(int num, int res) 
    : numCascades(num), resolution(res) {
    cascades.resize(numCascades);
}

CascadedShadowMap::~CascadedShadowMap() {
    Shutdown();
}

bool CascadedShadowMap::Initialize() {
    std::cout << "[CascadedShadowMap] Creating " << numCascades << " cascades at " << resolution << "x" << resolution << std::endl;
    
    for (int i = 0; i < numCascades; ++i) {
        cascades[i].shadowMap = new ShadowMap(resolution);
        if (!cascades[i].shadowMap->Initialize()) {
            return false;
        }
    }
    
    return true;
}

void CascadedShadowMap::Shutdown() {
    for (auto& cascade : cascades) {
        if (cascade.shadowMap) {
            cascade.shadowMap->Shutdown();
            delete cascade.shadowMap;
            cascade.shadowMap = nullptr;
        }
    }
}

void CascadedShadowMap::CalculateSplits(float nearPlane, float farPlane) {
    // Practical split scheme: logarithmic for near, linear for far
    float lambda = 0.5f; // Blend between log and linear
    
    for (int i = 0; i < numCascades; ++i) {
        float p = static_cast<float>(i + 1) / numCascades;
        
        float logSplit = nearPlane * std::pow(farPlane / nearPlane, p);
        float linearSplit = nearPlane + (farPlane - nearPlane) * p;
        
        cascades[i].splitDistance = lambda * logSplit + (1.0f - lambda) * linearSplit;
    }
}

void CascadedShadowMap::BeginCascadeRender(int cascadeIndex, const Vec3& lightDir,
                                           const Mat4& cameraView, const Mat4& cameraProj) {
    if (cascadeIndex < 0 || cascadeIndex >= numCascades) return;
    
    Cascade& cascade = cascades[cascadeIndex];
    
    // Calculate cascade frustum corners in world space
    // Then fit light projection to those corners
    
    // Stub: just begin render on the shadow map
    Vec3 sceneCenter(0, 0, 0);
    float sceneRadius = 50.0f;
    cascade.shadowMap->BeginRender(Vec3(0, 0, 0), lightDir, sceneCenter, sceneRadius);
}

void CascadedShadowMap::EndCascadeRender() {
    // End render on current cascade
    if (!cascades.empty() && cascades[0].shadowMap) {
        cascades[0].shadowMap->EndRender();
    }
}

int CascadedShadowMap::GetCascadeIndex(float depth) const {
    for (int i = 0; i < numCascades; ++i) {
        if (depth < cascades[i].splitDistance) {
            return i;
        }
    }
    return numCascades - 1;
}

// ============================================
// Shadow System Implementation
// ============================================

ShadowSystem::ShadowSystem() 
    : shadowShader(nullptr)
    , enabled(true)
    , shadowBias(0.005f)
    , shadowStrength(0.8f) {
}

ShadowSystem::~ShadowSystem() {
    Shutdown();
}

bool ShadowSystem::Initialize() {
    std::cout << "[ShadowSystem] Initializing shadow system" << std::endl;
    
    // Create default cascaded shadow map for directional light
    directionalShadows = std::make_unique<CascadedShadowMap>(4, 2048);
    if (!directionalShadows->Initialize()) {
        return false;
    }
    
    return true;
}

void ShadowSystem::Shutdown() {
    pointShadowMaps.clear();
    directionalShadows.reset();
}

ShadowMap* ShadowSystem::CreateShadowMap(int resolution) {
    auto shadowMap = std::make_unique<ShadowMap>(resolution);
    if (shadowMap->Initialize()) {
        ShadowMap* ptr = shadowMap.get();
        pointShadowMaps.push_back(std::move(shadowMap));
        return ptr;
    }
    return nullptr;
}

CascadedShadowMap* ShadowSystem::CreateCascadedShadowMap(int numCascades, int resolution) {
    auto csm = std::make_unique<CascadedShadowMap>(numCascades, resolution);
    if (csm->Initialize()) {
        directionalShadows = std::move(csm);
        return directionalShadows.get();
    }
    return nullptr;
}

void ShadowSystem::RenderShadows(const std::vector<Light>& lights,
                                const std::vector<Mesh*>& opaqueMeshes,
                                const Vec3& cameraPos,
                                const Mat4& cameraView,
                                const Mat4& cameraProj) {
    if (!enabled) return;
    
    // Find directional light and render CSM
    for (const auto& light : lights) {
        if (light.type == LightType::Directional) {
            if (directionalShadows) {
                // Calculate splits based on camera frustum
                directionalShadows->CalculateSplits(0.1f, 100.0f);
                
                // Render each cascade
                for (int i = 0; i < directionalShadows->GetNumCascades(); ++i) {
                    directionalShadows->BeginCascadeRender(i, light.direction, cameraView, cameraProj);
                    
                    // Render all opaque meshes
                    for (auto* mesh : opaqueMeshes) {
                        // Would render mesh with shadow shader
                    }
                    
                    directionalShadows->EndCascadeRender();
                }
            }
            break; // Only one directional light for CSM
        }
    }
    
    // Render point light shadows
    for (const auto& light : lights) {
        if (light.type == LightType::Point) {
            // Would render point shadow maps (cube maps)
        }
    }
}

void ShadowSystem::BindShadowMaps(Shader& shader) {
    if (!enabled) return;
    
    // Bind cascaded shadow maps
    if (directionalShadows) {
        const auto& cascades = directionalShadows->GetCascades();
        for (size_t i = 0; i < cascades.size(); ++i) {
            if (cascades[i].shadowMap) {
                cascades[i].shadowMap->BindTexture(3 + i);
                shader.SetMat4("lightSpaceMatrices[" + std::to_string(i) + "]", 
                              cascades[i].lightSpaceMatrix.data);
            }
        }
        shader.SetInt("numCascades", static_cast<int>(cascades.size()));
    }
    
    // Set shadow parameters
    shader.SetFloat("shadowBias", shadowBias);
    shader.SetFloat("shadowStrength", shadowStrength);
}

int ShadowSystem::GetShadowMapCount() const {
    int count = 0;
    if (directionalShadows) {
        count += directionalShadows->GetNumCascades();
    }
    count += static_cast<int>(pointShadowMaps.size());
    return count;
}

} // namespace vge