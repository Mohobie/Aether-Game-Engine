#pragma once

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include <vector>
#include <memory>

namespace vge {

// Forward declarations
class Shader;
class Texture;

// ============================================
// Post-Processing Effect Base
// ============================================
class PostProcessEffect {
public:
    virtual ~PostProcessEffect() = default;
    
    virtual void Initialize(uint32_t width, uint32_t height) = 0;
    virtual void Shutdown() = 0;
    
    // Process input texture, write to output
    virtual void Render(uint32_t inputTexture, uint32_t outputFBO, 
                       uint32_t width, uint32_t height) = 0;
    
    virtual const char* GetName() const = 0;
    virtual bool IsEnabled() const { return enabled; }
    virtual void SetEnabled(bool value) { enabled = value; }
    
protected:
    bool enabled = true;
};

// ============================================
// Tone Mapping (ACES Filmic)
// ============================================
class ToneMappingEffect : public PostProcessEffect {
public:
    enum class Operator {
        ACES,           // ACES Filmic
        Reinhard,       // Simple Reinhard
        Uncharted2,     // Uncharted 2 style
        Exposure        // Simple exposure
    };
    
    void Initialize(uint32_t width, uint32_t height) override;
    void Shutdown() override;
    void Render(uint32_t inputTexture, uint32_t outputFBO,
               uint32_t width, uint32_t height) override;
    
    const char* GetName() const override { return "Tone Mapping"; }
    
    void SetOperator(Operator op) { toneOperator = op; }
    void SetExposure(float exp) { exposure = exp; }
    void SetGamma(float g) { gamma = g; }
    
private:
    Operator toneOperator = Operator::ACES;
    float exposure = 1.0f;
    float gamma = 2.2f;
    
    std::unique_ptr<Shader> shader;
    uint32_t vao = 0;
    uint32_t vbo = 0;
};

// ============================================
// Bloom
// ============================================
class BloomEffect : public PostProcessEffect {
public:
    void Initialize(uint32_t width, uint32_t height) override;
    void Shutdown() override;
    void Render(uint32_t inputTexture, uint32_t outputFBO,
               uint32_t width, uint32_t height) override;
    
    const char* GetName() const override { return "Bloom"; }
    
    void SetIntensity(float i) { intensity = i; }
    void SetThreshold(float t) { threshold = t; }
    void SetRadius(float r) { radius = r; }
    
private:
    float intensity = 0.5f;
    float threshold = 1.0f;
    float radius = 4.0f;
    
    struct BloomMip {
        uint32_t fbo = 0;
        uint32_t texture = 0;
        uint32_t width = 0;
        uint32_t height = 0;
    };
    
    std::vector<BloomMip> mips;
    std::unique_ptr<Shader> downsampleShader;
    std::unique_ptr<Shader> upsampleShader;
    std::unique_ptr<Shader> combineShader;
    
    uint32_t vao = 0;
    uint32_t vbo = 0;
    
    void Downsample(uint32_t sourceTexture);
    void Upsample();
};

// ============================================
// FXAA Anti-Aliasing
// ============================================
class FXAAEffect : public PostProcessEffect {
public:
    void Initialize(uint32_t width, uint32_t height) override;
    void Shutdown() override;
    void Render(uint32_t inputTexture, uint32_t outputFBO,
               uint32_t width, uint32_t height) override;
    
    const char* GetName() const override { return "FXAA"; }
    
    void SetQuality(float q) { quality = q; }
    void SetEdgeThreshold(float t) { edgeThreshold = t; }
    
private:
    float quality = 0.75f;
    float edgeThreshold = 0.166f;
    
    std::unique_ptr<Shader> shader;
    uint32_t vao = 0;
    uint32_t vbo = 0;
};

// ============================================
// SSAO (Screen-Space Ambient Occlusion)
// ============================================
class SSAOEffect : public PostProcessEffect {
public:
    void Initialize(uint32_t width, uint32_t height) override;
    void Shutdown() override;
    void Render(uint32_t inputTexture, uint32_t outputFBO,
               uint32_t width, uint32_t height) override;
    
    // SSAO needs G-Buffer
    void SetGBuffer(uint32_t positionTex, uint32_t normalTex, uint32_t depthTex);
    void SetViewMatrix(const Mat4& view) { viewMatrix = view; }
    void SetProjectionMatrix(const Mat4& proj) { projMatrix = proj; }
    
    const char* GetName() const override { return "SSAO"; }
    
    void SetRadius(float r) { radius = r; }
    void SetIntensity(float i) { intensity = i; }
    void SetBias(float b) { bias = b; }
    
private:
    float radius = 0.5f;
    float intensity = 1.0f;
    float bias = 0.025f;
    
    Mat4 viewMatrix;
    Mat4 projMatrix;
    
    uint32_t positionTexture = 0;
    uint32_t normalTexture = 0;
    uint32_t depthTexture = 0;
    
    uint32_t ssaoFBO = 0;
    uint32_t ssaoTexture = 0;
    uint32_t blurFBO = 0;
    uint32_t blurTexture = 0;
    
    std::unique_ptr<Shader> ssaoShader;
    std::unique_ptr<Shader> blurShader;
    
    std::vector<Vec3> kernel;
    uint32_t noiseTexture = 0;
    
    uint32_t vao = 0;
    uint32_t vbo = 0;
    
    void GenerateKernel();
    void GenerateNoise();
};

// ============================================
// Post-Processing Stack
// ============================================
class PostProcessStack {
public:
    PostProcessStack();
    ~PostProcessStack();
    
    void Initialize(uint32_t width, uint32_t height);
    void Shutdown();
    
    // Add effects
    void AddEffect(std::shared_ptr<PostProcessEffect> effect);
    void RemoveEffect(const std::string& name);
    
    // Get effect by name
    std::shared_ptr<PostProcessEffect> GetEffect(const std::string& name);
    
    // Render the stack
    void Render(uint32_t inputTexture, uint32_t finalOutputFBO,
               uint32_t width, uint32_t height);
    
    // Resize
    void Resize(uint32_t width, uint32_t height);
    
    // Get statistics
    uint32_t GetEffectCount() const { return static_cast<uint32_t>(effects.size()); }
    std::vector<std::string> GetEffectNames() const;
    
private:
    std::vector<std::shared_ptr<PostProcessEffect>> effects;
    
    // Ping-pong buffers
    uint32_t fbo[2] = {0, 0};
    uint32_t texture[2] = {0, 0};
    uint32_t currentWidth = 0;
    uint32_t currentHeight = 0;
    
    void CreateBuffers(uint32_t width, uint32_t height);
    void DestroyBuffers();
};

} // namespace vge
