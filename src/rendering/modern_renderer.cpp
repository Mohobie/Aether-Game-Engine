#include "modern_renderer.h"
#include "core/logger.h"
#include "voxel/world.h"
#include "voxel/world_renderer.h"
#include "rendering/sky/sky_renderer.h"
#include "rendering/weather/weather_effects_renderer.h"
#include "rendering/sky/day_night_cycle.h"
#include "core/weather_system.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <iostream>

namespace vge {

// OpenGL extension function pointers
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffersPtr = nullptr;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebufferPtr = nullptr;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffersPtr = nullptr;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2DPtr = nullptr;
static PFNGLGENBUFFERSPROC glGenBuffersPtr = nullptr;
static PFNGLBINDBUFFERPROC glBindBufferPtr = nullptr;
static PFNGLBUFFERDATAPROC glBufferDataPtr = nullptr;
static PFNGLDELETEBUFFERSPROC glDeleteBuffersPtr = nullptr;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArraysPtr = nullptr;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArrayPtr = nullptr;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysPtr = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArrayPtr = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointerPtr = nullptr;

static void InitGLPointers() {
    static bool initialized = false;
    if (!initialized) {
        glGenFramebuffersPtr = (PFNGLGENFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glGenFramebuffers");
        glBindFramebufferPtr = (PFNGLBINDFRAMEBUFFERPROC)glXGetProcAddress((const GLubyte*)"glBindFramebuffer");
        glDeleteFramebuffersPtr = (PFNGLDELETEFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glDeleteFramebuffers");
        glFramebufferTexture2DPtr = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glXGetProcAddress((const GLubyte*)"glFramebufferTexture2D");
        glGenBuffersPtr = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glGenBuffers");
        glBindBufferPtr = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte*)"glBindBuffer");
        glBufferDataPtr = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte*)"glBufferData");
        glDeleteBuffersPtr = (PFNGLDELETEBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glDeleteBuffers");
        glGenVertexArraysPtr = (PFNGLGENVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte*)"glGenVertexArrays");
        glBindVertexArrayPtr = (PFNGLBINDVERTEXARRAYPROC)glXGetProcAddress((const GLubyte*)"glBindVertexArray");
        glDeleteVertexArraysPtr = (PFNGLDELETEVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte*)"glDeleteVertexArrays");
        glEnableVertexAttribArrayPtr = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress((const GLubyte*)"glEnableVertexAttribArray");
        glVertexAttribPointerPtr = (PFNGLVERTEXATTRIBPOINTERPROC)glXGetProcAddress((const GLubyte*)"glVertexAttribPointer");
        initialized = true;
    }
}

#define glGenFramebuffers glGenFramebuffersPtr
#define glBindFramebuffer glBindFramebufferPtr
#define glDeleteFramebuffers glDeleteFramebuffersPtr
#define glFramebufferTexture2D glFramebufferTexture2DPtr
#define glGenBuffers glGenBuffersPtr
#define glBindBuffer glBindBufferPtr
#define glBufferData glBufferDataPtr
#define glDeleteBuffers glDeleteBuffersPtr
#define glGenVertexArrays glGenVertexArraysPtr
#define glBindVertexArray glBindVertexArrayPtr
#define glDeleteVertexArrays glDeleteVertexArraysPtr
#define glEnableVertexAttribArray glEnableVertexAttribArrayPtr
#define glVertexAttribPointer glVertexAttribPointerPtr

ModernRenderer::ModernRenderer()
    : initialized(false)
    , width(1280)
    , height(720)
    , currentPath(RenderPath::Forward)
    , hdrFBO(0)
    , hdrTexture(0)
    , depthBuffer(0)
    , worldRenderer(nullptr)
    , skyRenderer(nullptr)
    , weatherRenderer(nullptr)
    , dayNightCycle(nullptr)
    , weatherSystem(nullptr)
    , lightingSystem(nullptr)
    , screenQuadVAO(0)
    , screenQuadVBO(0)
    , wireframeMode(false)
{
}

ModernRenderer::~ModernRenderer() {
    if (initialized) Shutdown();
}

bool ModernRenderer::Initialize(int w, int h) {
    InitGLPointers();
    
    width = w;
    height = h;
    
    InitializeOpenGL();
    InitializeFramebuffers();
    CreateScreenQuad();
    
    // Initialize post-processing
    postProcess = std::make_unique<PostProcessStack>();
    postProcess->Initialize(width, height);
    
    // Initialize deferred renderer
    deferredRenderer = std::make_unique<DeferredRenderer>();
    deferredRenderer->Initialize(width, height);
    
    // Initialize world renderer
    worldRenderer = new WorldRenderer();
    forwardShader = std::make_unique<Shader>();
    forwardShader->LoadFromSource(Shader::GetDefaultVertexShader(), 
                                   Shader::GetDefaultFragmentShader());
    worldRenderer->Initialize(forwardShader.get());
    
    Logger::Info("[ModernRenderer] Initialized with " + std::to_string(width) + "x" + std::to_string(height));
    initialized = true;
    return true;
}

void ModernRenderer::Shutdown() {
    if (worldRenderer) {
        delete worldRenderer;
        worldRenderer = nullptr;
    }
    
    postProcess.reset();
    deferredRenderer.reset();
    forwardShader.reset();
    
    ShutdownFramebuffers();
    DestroyScreenQuad();
    
    initialized = false;
    Logger::Info("[ModernRenderer] Shutdown");
}

void ModernRenderer::Resize(int w, int h) {
    width = w;
    height = h;
    
    ShutdownFramebuffers();
    InitializeFramebuffers();
    
    if (postProcess) {
        postProcess->Resize(width, height);
    }
    if (deferredRenderer) {
        deferredRenderer->Resize(width, height);
    }
}

void ModernRenderer::InitializeOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    Logger::Info("[ModernRenderer] OpenGL state initialized");
}

void ModernRenderer::InitializeFramebuffers() {
    // Create HDR framebuffer for rendering
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    
    // HDR color attachment (RGBA16F)
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTexture, 0);
    
    // Depth attachment
    glGenTextures(1, &depthBuffer);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    Logger::Info("[ModernRenderer] HDR framebuffer created");
}

void ModernRenderer::ShutdownFramebuffers() {
    if (hdrFBO) {
        glDeleteFramebuffers(1, &hdrFBO);
        hdrFBO = 0;
    }
    if (hdrTexture) {
        glDeleteTextures(1, &hdrTexture);
        hdrTexture = 0;
    }
    if (depthBuffer) {
        glDeleteTextures(1, &depthBuffer);
        depthBuffer = 0;
    }
}

void ModernRenderer::CreateScreenQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);
    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void ModernRenderer::DestroyScreenQuad() {
    if (screenQuadVAO) {
        glDeleteVertexArrays(1, &screenQuadVAO);
        screenQuadVAO = 0;
    }
    if (screenQuadVBO) {
        glDeleteBuffers(1, &screenQuadVBO);
        screenQuadVBO = 0;
    }
}

void ModernRenderer::BeginFrame() {
    if (dayNightCycle) {
        Vec3 skyColor = dayNightCycle->GetSkyTopColor();
        glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
    } else {
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    }
    
    // Clear HDR framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ModernRenderer::EndFrame() {
    // Apply post-processing and render to screen
    RenderPostProcess();
    
    // Reset state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
}

void ModernRenderer::RenderWorld(const World& world, const Camera& camera) {
    switch (currentPath) {
        case RenderPath::Forward:
            RenderForward(world, camera);
            break;
        case RenderPath::Deferred:
            RenderDeferred(world, camera);
            break;
        default:
            RenderForward(world, camera);
            break;
    }
}

void ModernRenderer::RenderForward(const World& world, const Camera& camera) {
    // Set up view-projection matrix
    Mat4 view = Mat4::LookAt(camera.GetPosition(), 
                              camera.GetPosition() + camera.GetForward(),
                              Vec3(0, 1, 0));
    Mat4 proj = Mat4::Perspective(70.0f * 3.14159f / 180.0f, 
                                   (float)width / (float)height, 
                                   0.1f, 1000.0f);
    Mat4 viewProj = proj * view;
    
    // Render world
    if (worldRenderer) {
        worldRenderer->Render(world, camera, viewProj);
    }
    
    // Render entities
    RenderEntities(camera);
}

void ModernRenderer::RenderDeferred(const World& world, const Camera& camera) {
    // Phase 1: Geometry pass
    deferredRenderer->BeginGeometryPass();
    
    // Render opaque geometry to G-Buffer
    if (worldRenderer) {
        Mat4 view = Mat4::LookAt(camera.GetPosition(),
                                  camera.GetPosition() + camera.GetForward(),
                                  Vec3(0, 1, 0));
        Mat4 proj = Mat4::Perspective(70.0f * 3.14159f / 180.0f,
                                       (float)width / (float)height,
                                       0.1f, 1000.0f);
        Mat4 viewProj = proj * view;
        worldRenderer->Render(world, camera, viewProj);
    }
    
    deferredRenderer->EndGeometryPass();
    
    // Phase 2: Lighting pass
    deferredRenderer->BeginLightingPass();
    
    // Render lights
    if (lightingSystem) {
        for (const auto& light : lightingSystem->GetLights()) {
            switch (light.type) {
                case LightType::Directional:
                    deferredRenderer->RenderDirectionalLight(
                        light.direction, light.color, light.intensity);
                    break;
                case LightType::Point:
                    deferredRenderer->RenderPointLight(
                        light.position, light.color, light.intensity, light.range);
                    break;
                case LightType::Spot:
                    deferredRenderer->RenderSpotLight(
                        light.position, light.direction, light.color,
                        light.intensity, light.range, light.spotAngle);
                    break;
            }
        }
    }
    
    deferredRenderer->EndLightingPass();
    
    // Copy result to HDR framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, deferredRenderer->GetLightingTexture());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void ModernRenderer::RenderPostProcess() {
    if (!postProcess) return;
    
    // Render post-processing stack
    postProcess->Render(hdrTexture, 0, width, height);
}

void ModernRenderer::RenderToScreen() {
    // Final blit to screen (if post-processing is disabled)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Render fullscreen quad with HDR texture
    // (Would use a simple copy shader)
}

void ModernRenderer::RenderSky(const Camera& camera) {
    if (skyRenderer) {
        skyRenderer->Render(camera);
    }
}

void ModernRenderer::RenderWeatherEffects(const Camera& camera) {
    if (weatherRenderer) {
        weatherRenderer->Render(camera);
    }
}

void ModernRenderer::RenderEntities(const Camera& camera) {
    // TODO: Render ECS entities
    (void)camera;
}

void ModernRenderer::SetPostProcessEnabled(bool enabled) {
    if (postProcess) {
        // Enable/disable all effects
        auto names = postProcess->GetEffectNames();
        for (const auto& name : names) {
            auto effect = postProcess->GetEffect(name);
            if (effect) {
                effect->SetEnabled(enabled);
            }
        }
    }
}

void ModernRenderer::SetRenderPath(RenderPath path) {
    currentPath = path;
    Logger::Info("[ModernRenderer] Switched to " + 
                 std::string(path == RenderPath::Deferred ? "deferred" : "forward") + 
                 " rendering");
}

void ModernRenderer::SetWireframe(bool enabled) {
    wireframeMode = enabled;
    if (enabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void ModernRenderer::VisualizeGBuffer() {
    if (currentPath == RenderPath::Deferred && deferredRenderer) {
        deferredRenderer->VisualizeGBuffer();
    }
}

void ModernRenderer::SetClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void ModernRenderer::SetViewport(int x, int y, int w, int h) {
    width = w;
    height = h;
    glViewport(x, y, w, h);
}

void ModernRenderer::RenderCrosshair(int screenW, int screenH) {
    // Legacy crosshair rendering
    (void)screenW;
    (void)screenH;
}

void ModernRenderer::RenderBlockHighlight(const Vec3& blockPos, const Camera& camera, int screenW, int screenH) {
    // Legacy block highlight rendering
    (void)blockPos;
    (void)camera;
    (void)screenW;
    (void)screenH;
}

} // namespace vge
