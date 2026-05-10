#include "post_processing.h"
#include <iostream>
#include <cmath>
#include <random>

namespace vge {

// ============================================
// Bloom Effect
// ============================================

BloomEffect::BloomEffect()
    : iterations(4)
    , threshold(0.8f)
    , intensity(0.5f)
    , spread(1.0f) {}

BloomEffect::~BloomEffect() {
    Shutdown();
}

void BloomEffect::Initialize(int width, int height) {
    std::cout << "[PostProcess] Initializing Bloom (" << width << "x" << height << ")" << std::endl;
    
    // Create ping-pong blur buffers
    for (int i = 0; i < 2; ++i) {
        // Would create FBO and texture here
        blurTextures.push_back(0);
        blurFBOs.push_back(0);
    }
}

void BloomEffect::Shutdown() {
    blurTextures.clear();
    blurFBOs.clear();
}

void BloomEffect::Render(unsigned int sourceTexture, unsigned int targetFBO, int width, int height) {
    // 1. Extract bright areas
    ExtractBright(sourceTexture, width, height);
    
    // 2. Blur bright areas
    BlurBrightAreas(width, height);
    
    // 3. Combine with original
    Combine(sourceTexture, targetFBO, width, height);
}

void BloomEffect::ExtractBright(unsigned int sourceTexture, int width, int height) {
    std::cout << "[Bloom] Extracting bright areas (threshold: " << threshold << ")" << std::endl;
    // Would render bright extraction shader
    (void)sourceTexture; (void)width; (void)height;
}

void BloomEffect::BlurBrightAreas(int width, int height) {
    std::cout << "[Bloom] Blurring with " << iterations << " iterations" << std::endl;
    // Would render Gaussian blur passes
    (void)width; (void)height;
}

void BloomEffect::Combine(unsigned int sourceTexture, unsigned int targetFBO, int width, int height) {
    std::cout << "[Bloom] Combining with intensity: " << intensity << std::endl;
    // Would render combine shader
    (void)sourceTexture; (void)targetFBO; (void)width; (void)height;
}

// ============================================
// SSAO Effect
// ============================================

SSAOEffect::SSAOEffect()
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

SSAOEffect::~SSAOEffect() {
    Shutdown();
}

void SSAOEffect::Initialize(int width, int height) {
    std::cout << "[PostProcess] Initializing SSAO (" << width << "x" << height << ")" << std::endl;
    
    GenerateKernel();
    GenerateNoiseTexture();
    
    // Would create FBOs and textures
    initialized = true;
}

void SSAOEffect::Shutdown() {
    initialized = false;
}

void SSAOEffect::GenerateKernel() {
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
        
        // Scale samples so they're more aligned to center
        float scale = static_cast<float>(i) / kernelSize;
        scale = 0.1f + scale * scale * 0.9f; // lerp(0.1f, 1.0f, scale^2)
        
        sample.x *= scale;
        sample.y *= scale;
        sample.z *= scale;
        
        kernelSamples.push_back(sample);
    }
    
    std::cout << "[SSAO] Generated " << kernelSize << " kernel samples" << std::endl;
}

void SSAOEffect::GenerateNoiseTexture() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // 4x4 noise texture
    for (int i = 0; i < 16; ++i) {
        Vec3 noise(
            dist(gen) * 2.0f - 1.0f,
            dist(gen) * 2.0f - 1.0f,
            0.0f
        );
        noiseTexture.push_back(noise);
    }
    
    std::cout << "[SSAO] Generated noise texture (4x4)" << std::endl;
}

void SSAOEffect::Render(unsigned int positionTexture, unsigned int normalTexture, 
                        const Mat4& projection, int width, int height) {
    if (!initialized) return;
    
    std::cout << "[SSAO] Rendering with radius: " << radius << ", bias: " << bias << std::endl;
    // Would render SSAO shader
    (void)positionTexture; (void)normalTexture; (void)projection; (void)width; (void)height;
}

void SSAOEffect::Blur(int width, int height) {
    std::cout << "[SSAO] Blurring result" << std::endl;
    // Would render blur shader
    (void)width; (void)height;
}

// ============================================
// Tone Mapping
// ============================================

ToneMappingEffect::ToneMappingEffect()
    : algorithm(Algorithm::ACES)
    , exposure(1.0f)
    , whitePoint(2.0f)
    , contrast(1.0f)
    , saturation(1.0f) {}

const char* ToneMappingEffect::GetAlgorithmDefine(Algorithm algo) {
    switch (algo) {
        case Algorithm::Reinhard: return "TONE_MAPPING_REINHARD";
        case Algorithm::ReinhardExtended: return "TONE_MAPPING_REINHARD_EXT";
        case Algorithm::ACES: return "TONE_MAPPING_ACES";
        case Algorithm::Uncharted2: return "TONE_MAPPING_UNCHARTED2";
        case Algorithm::Filmic: return "TONE_MAPPING_FILMIC";
        default: return "TONE_MAPPING_ACES";
    }
}

void ToneMappingEffect::Render(unsigned int hdrTexture, unsigned int targetFBO, 
                               int width, int height, float deltaTime) {
    std::cout << "[ToneMapping] Algorithm: " << GetAlgorithmDefine(algorithm) 
              << ", Exposure: " << exposure << std::endl;
    // Would render tone mapping shader
    (void)hdrTexture; (void)targetFBO; (void)width; (void)height; (void)deltaTime;
}

// ============================================
// FXAA
// ============================================

FXAAEffect::FXAAEffect()
    : quality(0.75f)
    , threshold(0.166f)
    , thresholdMin(0.0833f) {}

void FXAAEffect::Render(unsigned int sourceTexture, unsigned int targetFBO, int width, int height) {
    std::cout << "[FXAA] Rendering with quality: " << quality << std::endl;
    // Would render FXAA shader
    (void)sourceTexture; (void)targetFBO; (void)width; (void)height;
}

// ============================================
// Color Grading
// ============================================

ColorGradingEffect::ColorGradingEffect()
    : lutTexture(0)
    , lutSize(32)
    , intensity(1.0f)
    , useLUT(false)
    , brightness(0.0f)
    , contrast(1.0f)
    , saturation(1.0f)
    , hueShift(0.0f)
    , tint(1.0f, 1.0f, 1.0f)
    , tintIntensity(0.0f)
    , redChannel(1.0f, 0.0f, 0.0f)
    , greenChannel(0.0f, 1.0f, 0.0f)
    , blueChannel(0.0f, 0.0f, 1.0f) {}

ColorGradingEffect::~ColorGradingEffect() {
    Shutdown();
}

void ColorGradingEffect::Initialize() {
    GenerateNeutralLUT();
}

void ColorGradingEffect::Shutdown() {
    // Would delete LUT texture
}

void ColorGradingEffect::LoadLUT(const std::string& filepath) {
    std::cout << "[ColorGrading] Loading LUT: " << filepath << std::endl;
    // Would load 3D LUT texture
    useLUT = true;
    (void)filepath;
}

void ColorGradingEffect::SetTint(const Vec3& color, float intensity) {
    tint = color;
    tintIntensity = intensity;
}

void ColorGradingEffect::Render(unsigned int sourceTexture, unsigned int targetFBO, int width, int height) {
    std::cout << "[ColorGrading] Brightness: " << brightness 
              << ", Contrast: " << contrast 
              << ", Saturation: " << saturation << std::endl;
    // Would render color grading shader
    (void)sourceTexture; (void)targetFBO; (void)width; (void)height;
}

void ColorGradingEffect::GenerateNeutralLUT(int size) {
    std::cout << "[ColorGrading] Generating neutral LUT (" << size << "x" << size << "x" << size << ")" << std::endl;
    lutSize = static_cast<float>(size);
    // Would generate neutral LUT texture
}

// ============================================
// Chromatic Aberration
// ============================================

ChromaticAberrationEffect::ChromaticAberrationEffect()
    : intensity(0.0f)
    , direction(1.0f, 0.0f) {}

void ChromaticAberrationEffect::Render(unsigned int sourceTexture, unsigned int targetFBO, int width, int height) {
    if (intensity <= 0.0f) return;
    
    std::cout << "[ChromaticAberration] Intensity: " << intensity << std::endl;
    // Would render chromatic aberration shader
    (void)sourceTexture; (void)targetFBO; (void)width; (void)height;
}

// ============================================
// Vignette
// ============================================

VignetteEffect::VignetteEffect()
    : intensity(0.4f)
    , smoothness(0.8f)
    , center(0.5f, 0.5f)
    , color(0.0f, 0.0f, 0.0f)
    , rounded(true) {}

void VignetteEffect::Render(unsigned int sourceTexture, unsigned int targetFBO, int width, int height) {
    if (intensity <= 0.0f) return;
    
    std::cout << "[Vignette] Intensity: " << intensity << ", Smoothness: " << smoothness << std::endl;
    // Would render vignette shader
    (void)sourceTexture; (void)targetFBO; (void)width; (void)height;
}

// ============================================
// Motion Blur
// ============================================

MotionBlurEffect::MotionBlurEffect()
    : intensity(1.0f)
    , sampleCount(8)
    , maxVelocity(16.0f)
    , firstFrame(true) {}

void MotionBlurEffect::Initialize() {
    firstFrame = true;
}

void MotionBlurEffect::Shutdown() {
}

void MotionBlurEffect::Render(unsigned int sourceTexture, unsigned int depthTexture,
                              unsigned int velocityTexture,
                              const Mat4& view, const Mat4& projection,
                              unsigned int targetFBO, int width, int height) {
    if (firstFrame) {
        firstFrame = false;
        previousViewProjection = projection.Multiply(view);
        return;
    }
    
    std::cout << "[MotionBlur] Samples: " << sampleCount << ", Intensity: " << intensity << std::endl;
    
    Mat4 currentViewProjection = projection.Multiply(view);
    // Mat4 invCurrentVP = currentViewProjection.inverse();
    
    // Would render motion blur shader
    (void)sourceTexture; (void)depthTexture; (void)velocityTexture; 
    (void)targetFBO; (void)width; (void)height;
    
    previousViewProjection = currentViewProjection;
}

void MotionBlurEffect::UpdateViewProjection(const Mat4& viewProj) {
    previousViewProjection = viewProj;
}

// ============================================
// Depth of Field
// ============================================

DepthOfFieldEffect::DepthOfFieldEffect()
    : focalDistance(10.0f)
    , focalLength(50.0f)
    , aperture(2.8f)
    , blurSize(4.0f)
    , bokehShape(BokehShape::Circular)
    , bokehTexture(0) {}

DepthOfFieldEffect::~DepthOfFieldEffect() {
    Shutdown();
}

void DepthOfFieldEffect::Initialize() {
    std::cout << "[PostProcess] Initializing Depth of Field" << std::endl;
    // Would generate bokeh texture
}

void DepthOfFieldEffect::Shutdown() {
    // Would delete bokeh texture
}

void DepthOfFieldEffect::Render(unsigned int sourceTexture, unsigned int depthTexture,
                                const Mat4& projection, unsigned int targetFBO,
                                int width, int height) {
    std::cout << "[DoF] Focal distance: " << focalDistance 
              << ", Aperture: f/" << aperture << std::endl;
    // Would render DoF shader
    (void)sourceTexture; (void)depthTexture; (void)projection; (void)targetFBO; (void)width; (void)height;
}

void DepthOfFieldEffect::AutoFocus(unsigned int depthTexture, const Vec2& screenPoint,
                                   const Mat4& invProjection, int width, int height) {
    std::cout << "[DoF] Auto-focusing at screen point (" << screenPoint.x << ", " << screenPoint.y << ")" << std::endl;
    // Would sample depth and calculate focal distance
    (void)depthTexture; (void)invProjection; (void)width; (void)height;
}

// ============================================
// Post-Process Stack
// ============================================

PostProcessStack::PostProcessStack()
    : hdrFBO(0)
    , hdrTexture(0)
    , depthTexture(0)
    , intermediateFBO(0)
    , intermediateTexture(0)
    , enableBloom(true)
    , enableSSAO(true)
    , enableToneMapping(true)
    , enableFXAA(true)
    , enableColorGrading(true)
    , enableChromaticAberration(false)
    , enableVignette(true)
    , enableMotionBlur(false)
    , enableDepthOfField(false)
    , width(0)
    , height(0)
    , initialized(false) {}

PostProcessStack::~PostProcessStack() {
    Shutdown();
}

void PostProcessStack::Initialize(int w, int h) {
    width = w;
    height = h;
    
    std::cout << "[PostProcess] Initializing stack (" << w << "x" << h << ")" << std::endl;
    
    // Create effects
    bloom = std::make_unique<BloomEffect>();
    ssao = std::make_unique<SSAOEffect>();
    toneMapping = std::make_unique<ToneMappingEffect>();
    fxaa = std::make_unique<FXAAEffect>();
    colorGrading = std::make_unique<ColorGradingEffect>();
    chromaticAberration = std::make_unique<ChromaticAberrationEffect>();
    vignette = std::make_unique<VignetteEffect>();
    motionBlur = std::make_unique<MotionBlurEffect>();
    depthOfField = std::make_unique<DepthOfFieldEffect>();
    
    // Initialize effects
    bloom->Initialize(w / 2, h / 2);  // Bloom at half resolution
    ssao->Initialize(w, h);
    colorGrading->Initialize();
    motionBlur->Initialize();
    depthOfField->Initialize();
    
    // Create HDR framebuffer
    std::cout << "[PostProcess] Creating HDR framebuffer" << std::endl;
    // Would create FBO with HDR color + depth textures
    
    initialized = true;
    
    std::cout << "[PostProcess] Stack initialized successfully" << std::endl;
}

void PostProcessStack::Shutdown() {
    if (!initialized) return;
    
    bloom->Shutdown();
    ssao->Shutdown();
    colorGrading->Shutdown();
    motionBlur->Shutdown();
    depthOfField->Shutdown();
    
    // Would delete framebuffers
    
    initialized = false;
    std::cout << "[PostProcess] Stack shutdown" << std::endl;
}

void PostProcessStack::Resize(int w, int h) {
    if (!initialized) return;
    
    Shutdown();
    Initialize(w, h);
}

void PostProcessStack::BeginScene() {
    if (!initialized) return;
    
    // Bind HDR framebuffer for scene rendering
    std::cout << "[PostProcess] Begin scene rendering to HDR buffer" << std::endl;
    // Would bind hdrFBO
}

void PostProcessStack::EndScene() {
    if (!initialized) return;
    
    std::cout << "[PostProcess] End scene, applying effects..." << std::endl;
    
    unsigned int currentSource = hdrTexture;
    unsigned int currentTarget = intermediateFBO;
    
    // 1. SSAO (needs position/normal from G-buffer)
    if (enableSSAO) {
        ssao->Render(0, 0, Mat4(), width, height);  // Would use actual G-buffer textures
        ssao->Blur(width, height);
    }
    
    // 2. Depth of Field
    if (enableDepthOfField) {
        depthOfField->Render(currentSource, depthTexture, Mat4(), currentTarget, width, height);
        std::swap(currentSource, intermediateTexture);
    }
    
    // 3. Motion Blur
    if (enableMotionBlur) {
        motionBlur->Render(currentSource, depthTexture, 0, Mat4(), Mat4(), currentTarget, width, height);
        std::swap(currentSource, intermediateTexture);
    }
    
    // 4. Bloom
    if (enableBloom) {
        bloom->Render(currentSource, currentTarget, width, height);
        std::swap(currentSource, intermediateTexture);
    }
    
    // 5. Tone Mapping (HDR to LDR)
    if (enableToneMapping) {
        toneMapping->Render(currentSource, currentTarget, width, height);
        std::swap(currentSource, intermediateTexture);
    }
    
    // 6. Color Grading
    if (enableColorGrading) {
        colorGrading->Render(currentSource, currentTarget, width, height);
        std::swap(currentSource, intermediateTexture);
    }
    
    // 7. Chromatic Aberration
    if (enableChromaticAberration) {
        chromaticAberration->Render(currentSource, currentTarget, width, height);
        std::swap(currentSource, intermediateTexture);
    }
    
    // 8. Vignette
    if (enableVignette) {
        vignette->Render(currentSource, currentTarget, width, height);
        std::swap(currentSource, intermediateTexture);
    }
    
    // 9. FXAA (last step before screen)
    if (enableFXAA) {
        fxaa->Render(currentSource, 0, width, height);  // 0 = screen
    } else {
        // Blit to screen without FXAA
        std::cout << "[PostProcess] Blitting to screen" << std::endl;
    }
}

void PostProcessStack::Render(unsigned int sourceTexture, unsigned int targetFBO) {
    // Full pipeline for external use
    hdrTexture = sourceTexture;
    EndScene();
    (void)targetFBO;
}

void PostProcessStack::PrintSettings() const {
    std::cout << "=== Post-Process Stack Settings ===" << std::endl;
    std::cout << "Resolution: " << width << "x" << height << std::endl;
    std::cout << "Bloom: " << (enableBloom ? "ON" : "OFF") << std::endl;
    std::cout << "SSAO: " << (enableSSAO ? "ON" : "OFF") << std::endl;
    std::cout << "Tone Mapping: " << (enableToneMapping ? "ON" : "OFF") << std::endl;
    std::cout << "FXAA: " << (enableFXAA ? "ON" : "OFF") << std::endl;
    std::cout << "Color Grading: " << (enableColorGrading ? "ON" : "OFF") << std::endl;
    std::cout << "Chromatic Aberration: " << (enableChromaticAberration ? "ON" : "OFF") << std::endl;
    std::cout << "Vignette: " << (enableVignette ? "ON" : "OFF") << std::endl;
    std::cout << "Motion Blur: " << (enableMotionBlur ? "ON" : "OFF") << std::endl;
    std::cout << "Depth of Field: " << (enableDepthOfField ? "ON" : "OFF") << std::endl;
    std::cout << "===================================" << std::endl;
}

} // namespace vge