# Deferred Rendering

## Overview

The Aether Engine deferred rendering system separates geometry and lighting calculations for efficient handling of many dynamic lights.

## Architecture

### G-Buffer Layout

| Attachment | Format | Content |
|------------|--------|---------|
| Position | RGBA16F | View-space position + linear depth |
| Normal | RGBA16F | View-space normal + roughness |
| Albedo | RGBA8 | Base color + metallic |
| Emissive | RGBA8 | Emissive color + AO |
| Depth | D24S8 | Hardware depth buffer |

### Render Phases

```
Phase 1: Geometry Pass
  - Bind G-Buffer
  - Render opaque geometry
  - Write to all attachments

Phase 2: Lighting Pass
  - Bind lighting accumulation FBO
  - For each light:
    - Render fullscreen quad (directional) or light volume (point/spot)
    - Sample G-Buffer
    - Accumulate lighting
```

## Usage

### Switching to Deferred

```cpp
ModernRenderer renderer;
renderer.Initialize(width, height);
renderer.SetRenderPath(RenderPath::Deferred);
```

### Adding Lights

```cpp
LightingSystem lights;
Light dirLight;
dirLight.type = LightType::Directional;
dirLight.direction = Vec3(-0.5f, -1.0f, -0.3f);
dirLight.color = Vec3(1.0f, 0.95f, 0.8f);
dirLight.intensity = 1.0f;
lights.AddLight(dirLight);

renderer.SetLightingSystem(&lights);
```

## Integration with Post-Processing

Deferred rendering output goes through the same post-processing stack:

```
Geometry Pass → G-Buffer
                    ↓
Lighting Pass → HDR Texture
                    ↓
Post-Process Stack → Screen
```

## Performance

| Scenario | Forward | Deferred |
|----------|---------|----------|
| Few lights | Faster | Slower (G-Buffer overhead) |
| Many lights | Slower | Faster |
| Transparent objects | Native | Requires forward pass |
| MSAA | Native | Requires separate pass |

## Limitations

- Transparent objects require separate forward pass
- No MSAA on G-Buffer (use FXAA or TAA instead)
- Higher memory bandwidth
- All materials must use same lighting model

## Future Work

- [ ] Tiled deferred rendering (compute shader)
- [ ] Clustered shading
- [ ] Transparency support (depth peeling)
- [ ] Subsurface scattering
- [ ] Decal support

## Status

✅ **Complete** - G-Buffer and lighting pass implemented, integrated with ModernRenderer

Last Updated: 2026-05-14
