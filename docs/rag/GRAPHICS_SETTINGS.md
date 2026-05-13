# Graphics Settings

## Quick Reference

```cpp
// Graphics quality presets
enum class GraphicsQuality {
    Low,      // Reduced particles, simple lighting
    Medium,   // Standard settings
    High,     // Full effects, shadows
    Ultra     // Max everything
};

// Apply settings
renderer.SetRenderDistance(12);        // Chunk render distance
renderer.SetShadowQuality(2);          // Shadow map resolution
renderer.SetParticleDensity(1.0f);     // Particle amount
renderer.SetAntiAliasing(true);        // FXAA
renderer.SetVSync(true);               // VSync on/off
```

## Features

### Quality Presets
| Setting | Low | Medium | High | Ultra |
|---------|-----|--------|------|-------|
| Render Distance | 6 | 10 | 14 | 20 |
| Shadows | Off | Low | High | Ultra |
| Particles | 50% | 75% | 100% | 150% |
| Anti-Aliasing | Off | FXAA | MSAA 4x | MSAA 8x |
| Texture Quality | Low | Medium | High | Ultra |

### Individual Settings
- **Render Distance:** 2-32 chunks
- **Shadow Quality:** Off/Low/Medium/High/Ultra
- **Particle Density:** 0-200%
- **VSync:** On/Off/Adaptive
- **Frame Rate Limit:** 30/60/120/Unlimited
- **Field of View:** 60-110 degrees
- **Brightness:** 0.5-1.5

## Implementation

```cpp
class GraphicsSettings {
    GraphicsQuality quality = GraphicsQuality::High;
    
public:
    void ApplyPreset(GraphicsQuality preset) {
        switch (preset) {
            case GraphicsQuality::Low:
                renderer.SetRenderDistance(6);
                renderer.SetShadows(false);
                renderer.SetParticleDensity(0.5f);
                break;
            case GraphicsQuality::High:
                renderer.SetRenderDistance(14);
                renderer.SetShadows(true);
                renderer.SetParticleDensity(1.0f);
                break;
            // ... etc
        }
    }
};
```

## Files
- `src/rendering/renderer.h`
- `src/ui/settings_menu.cpp`
