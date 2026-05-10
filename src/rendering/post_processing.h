#pragma once
#include "math/vec3.h"
#include "math/vec2.h"
#include "math/mat4.h"
#include <vector>
#include <memory>
#include <string>

namespace vge {

// ============================================
// Post-Processing Effects
// ============================================

// Bloom Effect - Glow around bright areas
class BloomEffect {
private:
    int iterations;
    float threshold;
    float intensity;
    float spread;
    
    // Ping-pong buffers for blur
    std::vector<unsigned int> blurTextures;
    std::vector<unsigned int> blurFBOs;
    
public:
    BloomEffect();
    ~BloomEffect();
    
    void Initialize(int width, int height);
    void Shutdown();
    
    // Parameters
    void SetThreshold(float thresh) { threshold = thresh; }
    void SetIntensity(float intens) { intensity = intens; }
    void SetIterations(int iter) { iterations = iter; }
    void SetSpread(float spread) { this->spread = spread; }
    
    float GetThreshold() const { return threshold; }
    float GetIntensity() const { return intensity; }
    int GetIterations() const { return iterations; }
    
    // Rendering
    void Render(unsigned int sourceTexture, unsigned int targetFBO, int width, int height);
    
    // Extract bright areas
    void ExtractBright(unsigned int sourceTexture, int width, int height);
    
    // Blur the bright areas
    void BlurBrightAreas(int width, int height);
    
    // Combine bloom with original
    void Combine(unsigned int sourceTexture, unsigned int targetFBO, int width, int height);
};

// SSAO - Screen Space Ambient Occlusion
class SSAOEffect {
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
    SSAOEffect();
    ~SSAOEffect();
    
    void Initialize(int width, int height);
    void Shutdown();
    
    // Parameters
    void SetKernelSize(int size) { kernelSize = size; GenerateKernel(); }
    void SetRadius(float r) { radius = r; }
    void SetBias(float b) { bias = b; }
    void SetIntensity(float i) { intensity = i; }
    
    int GetKernelSize() const { return kernelSize; }
    float GetRadius() const { return radius; }
    float GetBias() const { return bias; }
    
    // Generate sample kernel
    void GenerateKernel();
    void GenerateNoiseTexture();
    
    // Render SSAO
    void Render(
        unsigned int positionTexture,    // View-space positions
        unsigned int normalTexture,      // View-space normals
        const Mat4& projection,
        int width, int height
    );
    
    // Blur SSAO result
    void Blur(int width, int height);
    
    // Get output texture
    unsigned int GetSSAOTexture() const { return blurTexture; }
    unsigned int GetRawSSAOTexture() const { return ssaoTexture; }
};

// Tone Mapping - HDR to LDR conversion
class ToneMappingEffect {
public:
    enum class Algorithm {
        Reinhard,       // Simple Reinhard
        ReinhardExtended, // Extended Reinhard with white point
        ACES,           // Academy Color Encoding System
        Uncharted2,     // Uncharted 2 style
        Filmic          // Filmic tone mapping
    };
    
private:
    Algorithm algorithm;
    float exposure;
    float whitePoint;
    float contrast;
    float saturation;
    
public:
    ToneMappingEffect();
    
    // Parameters
    void SetAlgorithm(Algorithm algo) { algorithm = algo; }
    void SetExposure(float exp) { exposure = exp; }
    void SetWhitePoint(float wp) { whitePoint = wp; }
    void SetContrast(float c) { contrast = c; }
    void SetSaturation(float s) { saturation = s; }
    
    Algorithm GetAlgorithm() const { return algorithm; }
    float GetExposure() const { return exposure; }
    float GetWhitePoint() const { return whitePoint; }
    
    // Render tone mapping
    void Render(
        unsigned int hdrTexture,
        unsigned int targetFBO,
        int width, int height,
        float deltaTime = 0.016f  // For auto-exposure adaptation
    );
    
    // Get shader define for algorithm
    static const char* GetAlgorithmDefine(Algorithm algo);
};

// FXAA - Fast Approximate Anti-Aliasing
class FXAAEffect {
private:
    float quality;
    float threshold;
    float thresholdMin;
    
public:
    FXAAEffect();
    
    void SetQuality(float q) { quality = q; }
    void SetThreshold(float t) { threshold = t; }
    void SetThresholdMin(float t) { thresholdMin = t; }
    
    float GetQuality() const { return quality; }
    float GetThreshold() const { return threshold; }
    
    // Render FXAA
    void Render(
        unsigned int sourceTexture,
        unsigned int targetFBO,
        int width, int height
    );
};

// Color Grading - LUT-based color correction
class ColorGradingEffect {
private:
    unsigned int lutTexture;
    float lutSize;
    float intensity;
    bool useLUT;
    
    // Color adjustments
    float brightness;
    float contrast;
    float saturation;
    float hueShift;
    Vec3 tint;
    float tintIntensity;
    
    // Channel mixer
    Vec3 redChannel;
    Vec3 greenChannel;
    Vec3 blueChannel;
    
public:
    ColorGradingEffect();
    ~ColorGradingEffect();
    
    void Initialize();
    void Shutdown();
    
    // LUT
    void LoadLUT(const std::string& filepath);
    void SetLUTIntensity(float i) { intensity = i; }
    void EnableLUT(bool enable) { useLUT = enable; }
    
    // Color adjustments
    void SetBrightness(float b) { brightness = b; }
    void SetContrast(float c) { contrast = c; }
    void SetSaturation(float s) { saturation = s; }
    void SetHueShift(float h) { hueShift = h; }
    void SetTint(const Vec3& color, float intensity);
    
    // Channel mixer
    void SetRedChannel(const Vec3& mix) { redChannel = mix; }
    void SetGreenChannel(const Vec3& mix) { greenChannel = mix; }
    void SetBlueChannel(const Vec3& mix) { blueChannel = mix; }
    
    // Render color grading
    void Render(
        unsigned int sourceTexture,
        unsigned int targetFBO,
        int width, int height
    );
    
    // Generate neutral LUT
    void GenerateNeutralLUT(int size = 32);
};

// Chromatic Aberration - Lens color fringing
class ChromaticAberrationEffect {
private:
    float intensity;
    Vec2 direction;
    
public:
    ChromaticAberrationEffect();
    
    void SetIntensity(float i) { intensity = i; }
    void SetDirection(const Vec2& dir) { direction = dir; }
    
    float GetIntensity() const { return intensity; }
    
    void Render(
        unsigned int sourceTexture,
        unsigned int targetFBO,
        int width, int height
    );
};

// Vignette - Darkened edges
class VignetteEffect {
private:
    float intensity;
    float smoothness;
    Vec2 center;
    Vec3 color;
    bool rounded;
    
public:
    VignetteEffect();
    
    void SetIntensity(float i) { intensity = i; }
    void SetSmoothness(float s) { smoothness = s; }
    void SetCenter(const Vec2& c) { center = c; }
    void SetColor(const Vec3& c) { color = c; }
    void SetRounded(bool r) { rounded = r; }
    
    void Render(
        unsigned int sourceTexture,
        unsigned int targetFBO,
        int width, int height
    );
};

// Motion Blur - Camera movement blur
class MotionBlurEffect {
private:
    float intensity;
    int sampleCount;
    float maxVelocity;
    
    Mat4 previousViewProjection;
    bool firstFrame;
    
public:
    MotionBlurEffect();
    
    void SetIntensity(float i) { intensity = i; }
    void SetSampleCount(int count) { sampleCount = count; }
    void SetMaxVelocity(float max) { maxVelocity = max; }
    
    void Initialize();
    void Shutdown();
    
    void Render(
        unsigned int sourceTexture,
        unsigned int depthTexture,
        unsigned int velocityTexture,  // Optional: object velocity
        const Mat4& view,
        const Mat4& projection,
        unsigned int targetFBO,
        int width, int height
    );
    
    void UpdateViewProjection(const Mat4& viewProj);
};

// Depth of Field - Focus blur
class DepthOfFieldEffect {
public:
    enum class BokehShape {
        Circular,
        Hexagonal,
        Octagonal
    };
    
private:
    float focalDistance;
    float focalLength;
    float aperture;
    float blurSize;
    BokehShape bokehShape;
    
    // Bokeh textures for different shapes
    unsigned int bokehTexture;
    
public:
    DepthOfFieldEffect();
    ~DepthOfFieldEffect();
    
    void Initialize();
    void Shutdown();
    
    void SetFocalDistance(float dist) { focalDistance = dist; }
    void SetFocalLength(float length) { focalLength = length; }
    void SetAperture(float aperture) { this->aperture = aperture; }
    void SetBlurSize(float size) { blurSize = size; }
    void SetBokehShape(BokehShape shape) { bokehShape = shape; }
    
    float GetFocalDistance() const { return focalDistance; }
    float GetFocalLength() const { return focalLength; }
    
    void Render(
        unsigned int sourceTexture,
        unsigned int depthTexture,
        const Mat4& projection,
        unsigned int targetFBO,
        int width, int height
    );
    
    // Auto-focus on point
    void AutoFocus(unsigned int depthTexture, const Vec2& screenPoint, 
                   const Mat4& invProjection, int width, int height);
};

// ============================================
// Post-Processing Stack
// ============================================
class PostProcessStack {
private:
    // Effects
    std::unique_ptr<BloomEffect> bloom;
    std::unique_ptr<SSAOEffect> ssao;
    std::unique_ptr<ToneMappingEffect> toneMapping;
    std::unique_ptr<FXAAEffect> fxaa;
    std::unique_ptr<ColorGradingEffect> colorGrading;
    std::unique_ptr<ChromaticAberrationEffect> chromaticAberration;
    std::unique_ptr<VignetteEffect> vignette;
    std::unique_ptr<MotionBlurEffect> motionBlur;
    std::unique_ptr<DepthOfFieldEffect> depthOfField;
    
    // Framebuffers
    unsigned int hdrFBO;
    unsigned int hdrTexture;
    unsigned int depthTexture;
    
    unsigned int intermediateFBO;
    unsigned int intermediateTexture;
    
    // Settings
    bool enableBloom;
    bool enableSSAO;
    bool enableToneMapping;
    bool enableFXAA;
    bool enableColorGrading;
    bool enableChromaticAberration;
    bool enableVignette;
    bool enableMotionBlur;
    bool enableDepthOfField;
    
    int width;
    int height;
    bool initialized;
    
public:
    PostProcessStack();
    ~PostProcessStack();
    
    // Initialize with screen size
    void Initialize(int w, int h);
    void Shutdown();
    void Resize(int w, int h);
    
    // Toggle effects
    void EnableBloom(bool enable) { enableBloom = enable; }
    void EnableSSAO(bool enable) { enableSSAO = enable; }
    void EnableToneMapping(bool enable) { enableToneMapping = enable; }
    void EnableFXAA(bool enable) { enableFXAA = enable; }
    void EnableColorGrading(bool enable) { enableColorGrading = enable; }
    void EnableChromaticAberration(bool enable) { enableChromaticAberration = enable; }
    void EnableVignette(bool enable) { enableVignette = enable; }
    void EnableMotionBlur(bool enable) { enableMotionBlur = enable; }
    void EnableDepthOfField(bool enable) { enableDepthOfField = enable; }
    
    bool IsBloomEnabled() const { return enableBloom; }
    bool IsSSAOEnabled() const { return enableSSAO; }
    bool IsToneMappingEnabled() const { return enableToneMapping; }
    bool IsFXAAEnabled() const { return enableFXAA; }
    
    // Get effect instances for configuration
    BloomEffect* GetBloom() { return bloom.get(); }
    SSAOEffect* GetSSAO() { return ssao.get(); }
    ToneMappingEffect* GetToneMapping() { return toneMapping.get(); }
    FXAAEffect* GetFXAA() { return fxaa.get(); }
    ColorGradingEffect* GetColorGrading() { return colorGrading.get(); }
    ChromaticAberrationEffect* GetChromaticAberration() { return chromaticAberration.get(); }
    VignetteEffect* GetVignette() { return vignette.get(); }
    MotionBlurEffect* GetMotionBlur() { return motionBlur.get(); }
    DepthOfFieldEffect* GetDepthOfField() { return depthOfField.get(); }
    
    // Begin scene rendering (renders to HDR buffer)
    void BeginScene();
    
    // End scene rendering, apply post-processing
    void EndScene();
    
    // Get HDR framebuffer for scene rendering
    unsigned int GetHDRFBO() const { return hdrFBO; }
    unsigned int GetHDRTexture() const { return hdrTexture; }
    unsigned int GetDepthTexture() const { return depthTexture; }
    
    // Full pipeline render (for external use)
    void Render(unsigned int sourceTexture, unsigned int targetFBO);
    
    // Debug
    void PrintSettings() const;
};

} // namespace vge