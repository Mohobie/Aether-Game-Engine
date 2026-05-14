# Post-Processing System

## Overview

The Aether Engine post-processing system provides a modular, extensible pipeline for applying visual effects after the main scene render. It uses a ping-pong buffer system to chain multiple effects efficiently.

## Architecture

### Core Classes

| Class | Purpose |
|-------|---------|
| `PostProcessEffect` | Base class for all effects |
| `PostProcessStack` | Manages effect chain and ping-pong buffers |
| `ToneMappingEffect` | HDR to LDR conversion |
| `BloomEffect` | Multi-pass bloom with mip chain |
| `FXAAEffect` | Fast approximate anti-aliasing |
| `SSAOEffect` | Screen-space ambient occlusion |

### Effect Pipeline

```
HDR Framebuffer → Effect 1 → Effect 2 → ... → Final Output
                      ↑________↓
                    (ping-pong buffers)
```

## Usage

### Adding Effects

```cpp
auto stack = std::make_unique<PostProcessStack>();
stack->Initialize(width, height);

// Add bloom
auto bloom = std::make_shared<BloomEffect>();
bloom->SetIntensity(0.5f);
bloom->SetThreshold(1.0f);
stack->AddEffect(bloom);

// Add tone mapping
auto toneMap = std::make_shared<ToneMappingEffect>();
toneMap->SetOperator(ToneMappingEffect::Operator::ACES);
stack->AddEffect(toneMap);
```

### Rendering

```cpp
// After scene render to HDR framebuffer
postProcess->Render(hdrTexture, 0, width, height);
// Result is blitted to screen (FBO 0)
```

## Effects Reference

### Tone Mapping

| Operator | Description | Use Case |
|----------|-------------|----------|
| ACES | Filmic, natural look | Default, cinematic |
| Reinhard | Simple, preserves highlights | Performance |
| Uncharted2 | Game-style, saturated | Games |
| Exposure | Simple multiplier | Debug |

```cpp
toneMap->SetOperator(ToneMappingEffect::Operator::ACES);
toneMap->SetExposure(1.2f);
toneMap->SetGamma(2.2f);
```

### Bloom

```cpp
bloom->SetIntensity(0.5f);    // Bloom strength
bloom->SetThreshold(1.0f);    // Minimum brightness
bloom->SetRadius(4.0f);       // Blur radius
```

Uses 5 mip levels for efficient multi-pass blur.

### FXAA

```cpp
fxaa->SetQuality(0.75f);       // Edge detection quality
fxaa->SetEdgeThreshold(0.166f); // Edge detection threshold
```

### SSAO

```cpp
ssao->SetRadius(0.5f);        // Sample radius
ssao->SetIntensity(1.0f);     // Occlusion strength
ssao->SetBias(0.025f);        // Depth bias
ssao->SetGBuffer(posTex, normalTex, depthTex);
```

Requires G-Buffer from deferred rendering.

## Integration with ModernRenderer

The `ModernRenderer` automatically initializes the post-process stack:

```cpp
ModernRenderer renderer;
renderer.Initialize(width, height);

// Toggle effects
renderer.SetPostProcessEnabled(true);

// The stack is applied automatically in EndFrame()
```

## Performance Considerations

| Effect | Cost | Notes |
|--------|------|-------|
| Tone Mapping | Very Low | Single pass |
| FXAA | Low | Single pass |
| Bloom | Medium | 5 mip passes |
| SSAO | High | 64 samples + blur |

**Recommendations:**
- Always use tone mapping with HDR rendering
- FXAA is cheaper than MSAA
- Bloom adds significant visual quality
- SSAO is expensive - consider only on desktop

## Future Enhancements

- [ ] Motion blur
- [ ] Depth of field
- [ ] Chromatic aberration
- [ ] Vignette
- [ ] Color grading (LUT)
- [ ] Temporal anti-aliasing (TAA)

## Implementation Details

### Ping-Pong Buffers

Two RGBA16F framebuffers alternate as source and destination:

```cpp
// Effect 0: Read from HDR, write to buffer[0]
// Effect 1: Read from buffer[0], write to buffer[1]
// Effect 2: Read from buffer[1], write to buffer[0]
// ...
// Final: Read from last buffer, write to screen
```

### Full-Screen Quad

All effects render using a shared fullscreen triangle strip:

```
(-1,1) --- (1,1)
   |          |
(-1,-1) --- (1,-1)
```

### Shader Uniforms

Each effect receives:
- `screenTexture`: Input color texture
- `inverseScreenSize`: 1.0 / resolution
- Effect-specific uniforms

## Files

- `src/rendering/post_processing.h` - Declarations
- `src/rendering/post_processing.cpp` - Implementations
- `src/rendering/modern_renderer.h` - Integration

## Status

✅ **Complete** - All core effects implemented and integrated

Last Updated: 2026-05-14
