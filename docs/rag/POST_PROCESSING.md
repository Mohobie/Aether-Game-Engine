# Post-Processing System

## Overview

The Post-Processing System provides a flexible, chainable effects pipeline for enhancing rendered frames. Effects are applied in sequence using ping-pong framebuffers.

## Architecture

### PostProcessEffect Base Class

All effects inherit from `PostProcessEffect`:

```cpp
class PostProcessEffect {
public:
    virtual void Initialize(uint32_t width, uint32_t height) = 0;
    virtual void Shutdown() = 0;
    virtual void Render(uint32_t inputTexture, uint32_t outputFBO, 
                       uint32_t width, uint32_t height) = 0;
    virtual const char* GetName() const = 0;
    virtual bool IsEnabled() const;
};
```

### Built-in Effects

| Effect | Description | Parameters |
|--------|-------------|------------|
| **ToneMapping** | HDR to LDR conversion | exposure, gamma, operator |
| **Bloom** | Glow around bright areas | intensity, threshold, radius |
| **FXAA** | Fast approximate anti-aliasing | quality, edgeThreshold |
| **SSAO** | Screen-space ambient occlusion | radius, intensity, bias |

## PostProcessStack

Manages effect chain with ping-pong buffers:

```cpp
PostProcessStack stack;
stack.Initialize(1920, 1080);

// Add effects
stack.AddEffect(std::make_shared<BloomEffect>());
stack.AddEffect(std::make_shared<ToneMappingEffect>());
stack.AddEffect(std::make_shared<FXAAEffect>());

// Render
stack.Render(sceneTexture, finalFBO, width, height);
```

## Effect Details

### Tone Mapping

Converts HDR to LDR with filmic response:

```cpp
auto toneMapping = std::make_shared<ToneMappingEffect>();
toneMapping->SetOperator(ToneMappingEffect::Operator::ACES);
toneMapping->SetExposure(1.0f);
toneMapping->SetGamma(2.2f);
```

**Operators:**
- `ACES` - Academy Color Encoding System (default)
- `Reinhard` - Simple global operator
- `Uncharted2` - Uncharted 2 filmic curve
- `Exposure` - Simple exposure adjustment

### Bloom

Multi-pass glow effect:

```cpp
auto bloom = std::make_shared<BloomEffect>();
bloom->SetIntensity(0.5f);
bloom->SetThreshold(1.0f);  // Only bloom values > 1.0
bloom->SetRadius(4.0f);     // Blur radius
```

**Algorithm:**
1. Downsample scene to multiple mip levels
2. Upsample with additive blending
3. Combine with original scene

### FXAA

Fast edge anti-aliasing:

```cpp
auto fxaa = std::make_shared<FXAAEffect>();
fxaa->SetQuality(0.75f);
fxaa->SetEdgeThreshold(0.166f);
```

### SSAO

Ambient occlusion from G-Buffer:

```cpp
auto ssao = std::make_shared<SSAOEffect>();
ssao->SetGBuffer(positionTex, normalTex, depthTex);
ssao->SetRadius(0.5f);
ssao->SetIntensity(1.0f);
ssao->SetBias(0.025f);
```

**Requires:**
- View-space position texture
- View-space normal texture
- Projection matrix

## Usage Example

```cpp
// Initialize
PostProcessStack postProcess;
postProcess.Initialize(windowWidth, windowHeight);

// Add effects in order
auto ssao = std::make_shared<SSAOEffect>();
ssao->SetGBuffer(gPosition, gNormal, gDepth);
postProcess.AddEffect(ssao);

auto bloom = std::make_shared<BloomEffect>();
bloom->SetIntensity(0.3f);
postProcess.AddEffect(bloom);

auto toneMapping = std::make_shared<ToneMappingEffect>();
toneMapping->SetExposure(1.2f);
postProcess.AddEffect(toneMapping);

auto fxaa = std::make_shared<FXAAEffect>();
postProcess.AddEffect(fxaa);

// In render loop
void Render() {
    // Render scene to HDR framebuffer
    RenderScene();
    
    // Apply post-processing
    postProcess.Render(hdrTexture, 0, width, height);  // 0 = screen
}

// On resize
void OnResize(uint32_t w, uint32_t h) {
    postProcess.Resize(w, h);
}
```

## Creating Custom Effects

```cpp
class VignetteEffect : public PostProcessEffect {
public:
    void Initialize(uint32_t width, uint32_t height) override {
        // Create shader and quad
    }
    
    void Render(uint32_t inputTexture, uint32_t outputFBO,
                uint32_t width, uint32_t height) override {
        // Bind shader, set uniforms, draw quad
    }
    
    const char* GetName() const override { return "Vignette"; }
};
```

## Performance Tips

1. **Order matters** - Put cheaper effects first
2. **Disable unused** - Toggle `SetEnabled(false)` for unused effects
3. **Resolution scaling** - Run expensive effects at half resolution
4. **SSAO before bloom** - SSAO is cheaper, bloom can hide artifacts

## Future Improvements

- [ ] Motion blur
- [ ] Depth of field (Bokeh)
- [ ] Color grading (LUT)
- [ ] Chromatic aberration
- [ ] Vignette
- [ ] Film grain
