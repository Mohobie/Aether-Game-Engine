#include "deferred_rendering.h"
#include <iostream>
#include <cmath>
#include <random>

namespace vge {

// ============================================
// GBuffer Implementation
// ============================================

GBuffer::GBuffer()
    : fbo(0)
    , depthTexture(0)
    , width(0)
    , height(0)
    , initialized(false) {}

GBuffer::~GBuffer() {
    Shutdown();
}

void GBuffer::Initialize(int w, int h) {
    width = w;
    height = h;
    
    std::cout << "[GBuffer] Initializing (" << w << "x" << h << ")" << std::endl;
    
    // Setup attachment configurations
    attachments.resize(static_cast<int>(GBufferAttachment::Count));
    
    // Position attachment (RGBA16F for HDR positions)
    attachments[static_cast<int>(GBufferAttachment::Position)] = {
        0, 0x881A, 0x1908, 0x1406, "Position", true  // GL_RGBA16F, GL_RGBA, GL_FLOAT
    };
    
    // Normal attachment (RGBA16F for HDR normals)
    attachments[static_cast<int>(GBufferAttachment::Normal)] = {
        0, 0x881A, 0x1908, 0x1406, "Normal", true
    };
    
    // Albedo attachment (RGBA8)
    attachments[static_cast<int>(GBufferAttachment::Albedo)] = {
        0, 0x8058, 0x1908, 0x1401, "Albedo", true  // GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE
    };
    
    // Emissive attachment (RGBA16F)
    attachments[static_cast<int>(GBufferAttachment::Emissive)] = {
        0, 0x881A, 0x1908, 0x1406, "Emissive", true
    };
    
    // Depth attachment
    depthTexture = 0;
    
    std::cout << "[GBuffer] Created " << attachments.size() << " color attachments" << std::endl;
    initialized = true;
}

void GBuffer::Shutdown() {
    if (!initialized) return;
    
    attachments.clear();
    initialized = false;
    std::cout << "[GBuffer] Shutdown" << std::endl;
}

void GBuffer::Resize(int w, int h) {
    if (!initialized) return;
    
    Shutdown();
    Initialize(w, h);
}

void GBuffer::BindForGeometryPass() {
    std::cout << "[GBuffer] Binding for geometry pass" << std::endl;
    // Would bind FBO and set draw buffers
}

void GBuffer::BindForLightingPass() {
    std::cout << "[GBuffer] Binding for lighting pass (read-only)" << std::endl;
    // Would bind textures for reading
}

void GBuffer::BindTexture(GBufferAttachment attachment, unsigned int slot) const {
    int index = static_cast<int>(attachment);
    if (index >= 0 && static_cast<size_t>(index) < attachments.size()) {
        std::cout << "[GBuffer] Binding " << attachments[index].name 
                  << " to slot " << slot << std::endl;
    }
}

unsigned int GBuffer::GetTexture(GBufferAttachment attachment) const {
    int index = static_cast<int>(attachment);
    if (index >= 0 && static_cast<size_t>(index) < attachments.size()) {
        return attachments[index].textureID;
    }
    return 0;
}

void GBuffer::CopyDepthTo(unsigned int targetFBO) {
    std::cout << "[GBuffer] Copying depth to target FBO" << std::endl;
    // Would use glBlitFramebuffer
    (void)targetFBO;
}

void GBuffer::PrintInfo() const {
    std::cout << "=== GBuffer Info ===" << std::endl;
    std::cout << "Resolution: " << width << "x" << height << std::endl;
    for (const auto& att : attachments) {
        std::cout << "  " << att.name << ": " << (att.enabled ? "enabled" : "disabled") << std::endl;
    }
    std::cout << "===================" << std::endl;
}

// ============================================
// Deferred Renderer Implementation
// ============================================

DeferredRenderer::DeferredRenderer()
    : lightingFBO(0)
    , lightingTexture(0)
    , screenQuadVAO(0)
    , screenQuadVBO(0)
    , geometryShader(0)
    , lightingShader(0)
    , stencilShader(0)
    , useStencilOptimization(true)
    , useTiledDeferred(false)
    , tileSize(16) {}

DeferredRenderer::~DeferredRenderer() {
    Shutdown();
}

void DeferredRenderer::Initialize(int width, int height) {
    std::cout << "[DeferredRenderer] Initializing" << std::endl;
    
    // Initialize G-Buffer
    gbuffer.Initialize(width, height);
    
    // Create lighting FBO
    std::cout << "[DeferredRenderer] Creating lighting accumulation buffer" << std::endl;
    // Would create FBO with HDR texture
    
    // Create screen quad
    std::cout << "[DeferredRenderer] Creating screen quad" << std::endl;
    // Would create VAO/VBO for fullscreen quad
    
    std::cout << "[DeferredRenderer] Initialization complete" << std::endl;
}

void DeferredRenderer::Shutdown() {
    gbuffer.Shutdown();
    std::cout << "[DeferredRenderer] Shutdown" << std::endl;
}

void DeferredRenderer::Resize(int width, int height) {
    gbuffer.Resize(width, height);
    // Would resize lighting FBO
}

void DeferredRenderer::BeginGeometryPass() {
    std::cout << "[DeferredRenderer] Beginning geometry pass" << std::endl;
    gbuffer.BindForGeometryPass();
    // Would clear G-Buffer and set state
}

void DeferredRenderer::EndGeometryPass() {
    std::cout << "[DeferredRenderer] Ending geometry pass" << std::endl;
    // Would unbind FBO
}

void DeferredRenderer::BeginLightingPass() {
    std::cout << "[DeferredRenderer] Beginning lighting pass" << std::endl;
    // Would bind lighting FBO and set additive blending
}

void DeferredRenderer::RenderDirectionalLight(const Vec3& direction, const Vec3& color, float intensity) {
    std::cout << "[DeferredRenderer] Rendering directional light" << std::endl;
    std::cout << "  Direction: (" << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
    std::cout << "  Color: (" << color.x << ", " << color.y << ", " << color.z << ")" << std::endl;
    std::cout << "  Intensity: " << intensity << std::endl;
    // Would render fullscreen quad with directional light shader
}

void DeferredRenderer::RenderPointLight(const Vec3& position, const Vec3& color, float intensity, float radius) {
    std::cout << "[DeferredRenderer] Rendering point light" << std::endl;
    std::cout << "  Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    std::cout << "  Radius: " << radius << std::endl;
    
    if (useStencilOptimization) {
        std::cout << "  Using stencil optimization" << std::endl;
        // Would render light volume with stencil test
    } else {
        // Would render fullscreen quad
    }
    (void)color; (void)intensity;
}

void DeferredRenderer::RenderSpotLight(const Vec3& position, const Vec3& direction, const Vec3& color,
                                       float intensity, float radius, float angle) {
    std::cout << "[DeferredRenderer] Rendering spot light" << std::endl;
    std::cout << "  Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    std::cout << "  Angle: " << angle << " degrees" << std::endl;
    
    if (useStencilOptimization) {
        std::cout << "  Using stencil optimization (cone volume)" << std::endl;
    }
    (void)direction; (void)color; (void)intensity; (void)radius;
}

void DeferredRenderer::EndLightingPass() {
    std::cout << "[DeferredRenderer] Ending lighting pass" << std::endl;
    // Would unbind FBO and reset blend state
}

unsigned int DeferredRenderer::GetGBufferTexture(GBufferAttachment attachment) const {
    return gbuffer.GetTexture(attachment);
}

void DeferredRenderer::VisualizeGBuffer() {
    std::cout << "[DeferredRenderer] Visualizing G-Buffer attachments" << std::endl;
    // Would render each attachment to a portion of the screen for debugging
}

// ============================================
// Light Volume Implementation
// ============================================

LightVolume::LightVolume()
    : sphereVAO(0)
    , sphereVBO(0)
    , sphereVertexCount(0)
    , coneVAO(0)
    , coneVBO(0)
    , coneVertexCount(0) {}

LightVolume::~LightVolume() {
    Shutdown();
}

void LightVolume::Initialize() {
    std::cout << "[LightVolume] Initializing geometry" << std::endl;
    
    // Generate sphere for point lights
    std::cout << "[LightVolume] Generating sphere (low-poly)" << std::endl;
    sphereVertexCount = 240; // Low-poly sphere
    // Would generate sphere vertices
    
    // Generate cone for spot lights
    std::cout << "[LightVolume] Generating cone" << std::endl;
    coneVertexCount = 32; // Cone base
    // Would generate cone vertices
}

void LightVolume::Shutdown() {
    std::cout << "[LightVolume] Shutdown" << std::endl;
}

void LightVolume::RenderSphere(const Vec3& position, float radius) {
    std::cout << "[LightVolume] Rendering sphere at (" << position.x << ", " << position.y << ", " << position.z 
              << ") radius=" << radius << std::endl;
    // Would render sphere with translation and scale
}

void LightVolume::RenderCone(const Vec3& position, const Vec3& direction, float angle, float radius) {
    std::cout << "[LightVolume] Rendering cone at (" << position.x << ", " << position.y << ", " << position.z 
              << ") angle=" << angle << " radius=" << radius << std::endl;
    // Would render cone with proper orientation
}

// ============================================
// Tiled Deferred Implementation
// ============================================

TiledDeferred::TiledDeferred()
    : tileSize(16)
    , tileCountX(0)
    , tileCountY(0)
    , tileComputeShader(0)
    , tileSSBO(0) {}

TiledDeferred::~TiledDeferred() {
    Shutdown();
}

void TiledDeferred::Initialize(int screenWidth, int screenHeight, int size) {
    tileSize = size;
    tileCountX = (screenWidth + tileSize - 1) / tileSize;
    tileCountY = (screenHeight + tileSize - 1) / tileSize;
    
    tiles.resize(tileCountX * tileCountY);
    
    std::cout << "[TiledDeferred] Initialized with " << tileCountX << "x" << tileCountY 
              << " tiles (" << tileSize << "x" << tileSize << " each)" << std::endl;
    std::cout << "[TiledDeferred] Total tiles: " << tiles.size() << std::endl;
}

void TiledDeferred::Shutdown() {
    tiles.clear();
    std::cout << "[TiledDeferred] Shutdown" << std::endl;
}

void TiledDeferred::BuildLightLists(const std::vector<class PointLight>& lights, const Mat4& viewProjection) {
    std::cout << "[TiledDeferred] Building light lists for " << lights.size() << " lights" << std::endl;
    
    // Clear previous lists
    for (auto& tile : tiles) {
        tile.lightIndices.clear();
    }
    
    // For each light, determine which tiles it affects
    for (size_t i = 0; i < lights.size(); ++i) {
        // Simple approach: test light bounding sphere against tile frustums
        // Would use proper frustum-sphere intersection
        
        // For now, add to all tiles (simplified)
        for (auto& tile : tiles) {
            tile.lightIndices.push_back(static_cast<int>(i));
        }
    }
    
    // Print stats
    int totalLightRefs = 0;
    for (const auto& tile : tiles) {
        totalLightRefs += tile.lightIndices.size();
    }
    std::cout << "[TiledDeferred] Average lights per tile: " 
              << (totalLightRefs / static_cast<float>(tiles.size())) << std::endl;
    (void)viewProjection;
}

const std::vector<int>& TiledDeferred::GetLightsForTile(int tileX, int tileY) const {
    int index = tileY * tileCountX + tileX;
    if (index >= 0 && static_cast<size_t>(index) < tiles.size()) {
        return tiles[index].lightIndices;
    }
    static std::vector<int> empty;
    return empty;
}

// ============================================
// SSAO Deferred Implementation
// ============================================

SSAODeferred::SSAODeferred()
    : kernelSize(64)
    , radius(0.5f)
    , bias(0.025f)
    , intensity(1.0f)
    , ssaoFBO(0)
    , ssaoTexture(0)
    , blurFBO(0)
    , blurTexture(0)
    , noiseTextureID(0)
    , initialized(false) {}

SSAODeferred::~SSAODeferred() {
    Shutdown();
}

void SSAODeferred::Initialize(int width, int height) {
    std::cout << "[SSAODeferred] Initializing (" << width << "x" << height << ")" << std::endl;
    
    GenerateKernel();
    GenerateNoiseTexture();
    
    initialized = true;
}

void SSAODeferred::Shutdown() {
    initialized = false;
}

void SSAODeferred::GenerateKernel() {
    kernelSamples.clear();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (int i = 0; i < kernelSize; ++i) {
        Vec3 sample(
            dist(gen) * 2.0f - 1.0f,
            dist(gen) * 2.0f - 1.0f,
            dist(gen)
        );
        
        float scale = static_cast<float>(i) / kernelSize;
        scale = 0.1f + scale * scale * 0.9f;
        
        sample.x *= scale;
        sample.y *= scale;
        sample.z *= scale;
        
        kernelSamples.push_back(sample);
    }
    
    std::cout << "[SSAODeferred] Generated " << kernelSize << " kernel samples" << std::endl;
}

void SSAODeferred::GenerateNoiseTexture() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (int i = 0; i < 16; ++i) {
        Vec3 noise(
            dist(gen) * 2.0f - 1.0f,
            dist(gen) * 2.0f - 1.0f,
            0.0f
        );
        noiseTexture.push_back(noise);
    }
    
    std::cout << "[SSAODeferred] Generated noise texture (4x4)" << std::endl;
}

void SSAODeferred::Render(const GBuffer& gbuffer, const Mat4& projection) {
    if (!initialized) return;
    
    std::cout << "[SSAODeferred] Rendering using G-Buffer data" << std::endl;
    std::cout << "  Radius: " << radius << ", Bias: " << bias << std::endl;
    
    // Would use G-Buffer position and normal textures
    const_cast<GBuffer*>(&gbuffer)->BindTexture(GBufferAttachment::Position, 0);
    const_cast<GBuffer*>(&gbuffer)->BindTexture(GBufferAttachment::Normal, 1);
    
    // Render SSAO pass
    std::cout << "[SSAODeferred] SSAO pass complete" << std::endl;
    (void)projection;
}

void SSAODeferred::Blur() {
    std::cout << "[SSAODeferred] Blurring result" << std::endl;
}

// ============================================
// Screen Space Reflections Implementation
// ============================================

ScreenSpaceReflections::ScreenSpaceReflections()
    : maxSteps(64)
    , maxDistance(100.0f)
    , thickness(0.5f)
    , stride(1.0f)
    , ssrFBO(0)
    , ssrTexture(0)
    , hitPointTexture(0)
    , initialized(false) {}

ScreenSpaceReflections::~ScreenSpaceReflections() {
    Shutdown();
}

void ScreenSpaceReflections::Initialize(int width, int height) {
    std::cout << "[SSR] Initializing (" << width << "x" << height << ")" << std::endl;
    std::cout << "[SSR] Max steps: " << maxSteps << ", Max distance: " << maxDistance << std::endl;
    initialized = true;
}

void ScreenSpaceReflections::Shutdown() {
    initialized = false;
}

void ScreenSpaceReflections::Render(const GBuffer& gbuffer, const Mat4& view, const Mat4& projection) {
    if (!initialized) return;
    
    std::cout << "[SSR] Rendering screen-space reflections" << std::endl;
    
    // Bind G-Buffer textures
    const_cast<GBuffer*>(&gbuffer)->BindTexture(GBufferAttachment::Position, 0);
    const_cast<GBuffer*>(&gbuffer)->BindTexture(GBufferAttachment::Normal, 1);
    const_cast<GBuffer*>(&gbuffer)->BindTexture(GBufferAttachment::Albedo, 2);
    const_cast<GBuffer*>(&gbuffer)->BindTexture(GBufferAttachment::Depth, 3);
    
    // Ray march in screen space
    std::cout << "[SSR] Ray marching with stride=" << stride << ", thickness=" << thickness << std::endl;
    
    std::cout << "[SSR] Reflection pass complete" << std::endl;
    (void)view; (void)projection;
}

// ============================================
// Light Manager Implementation
// ============================================

LightManager::LightManager()
    : maxDirectionalLights(4)
    , maxPointLights(1024)
    , maxSpotLights(16) {}

void LightManager::AddDirectionalLight(const DirectionalLight& light) {
    if (directionalLights.size() < static_cast<size_t>(maxDirectionalLights)) {
        directionalLights.push_back(light);
        std::cout << "[LightManager] Added directional light. Total: " << directionalLights.size() << std::endl;
    } else {
        std::cout << "[LightManager] Warning: Max directional lights reached (" << maxDirectionalLights << ")" << std::endl;
    }
}

void LightManager::AddPointLight(const PointLight& light) {
    if (pointLights.size() < static_cast<size_t>(maxPointLights)) {
        pointLights.push_back(light);
        if (pointLights.size() % 100 == 0) {
            std::cout << "[LightManager] Added point light. Total: " << pointLights.size() << std::endl;
        }
    } else {
        std::cout << "[LightManager] Warning: Max point lights reached (" << maxPointLights << ")" << std::endl;
    }
}

void LightManager::AddSpotLight(const SpotLight& light) {
    if (spotLights.size() < static_cast<size_t>(maxSpotLights)) {
        spotLights.push_back(light);
        std::cout << "[LightManager] Added spot light. Total: " << spotLights.size() << std::endl;
    } else {
        std::cout << "[LightManager] Warning: Max spot lights reached (" << maxSpotLights << ")" << std::endl;
    }
}

void LightManager::RemoveDirectionalLight(int index) {
    if (index >= 0 && static_cast<size_t>(index) < directionalLights.size()) {
        directionalLights.erase(directionalLights.begin() + index);
    }
}

void LightManager::RemovePointLight(int index) {
    if (index >= 0 && static_cast<size_t>(index) < pointLights.size()) {
        pointLights.erase(pointLights.begin() + index);
    }
}

void LightManager::RemoveSpotLight(int index) {
    if (index >= 0 && static_cast<size_t>(index) < spotLights.size()) {
        spotLights.erase(spotLights.begin() + index);
    }
}

const std::vector<DirectionalLight>& LightManager::GetDirectionalLights() const {
    return directionalLights;
}

const std::vector<PointLight>& LightManager::GetPointLights() const {
    return pointLights;
}

const std::vector<SpotLight>& LightManager::GetSpotLights() const {
    return spotLights;
}

void LightManager::Clear() {
    directionalLights.clear();
    pointLights.clear();
    spotLights.clear();
    std::cout << "[LightManager] All lights cleared" << std::endl;
}

} // namespace vge