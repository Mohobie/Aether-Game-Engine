# Occlusion Culling System

## Overview

The Aether Engine occlusion culling system provides multiple techniques for efficient visibility determination, reducing overdraw and improving performance in complex scenes.

## Architecture

### CullingSystem (Main Interface)

```cpp
CullingSystem culling;
culling.setCamera(camera.GetPosition(), viewProj);
culling.setViewDistance(1000.0f);
CullingResult result = culling.cullAll();
```

### Integration with ModernRenderer

```cpp
ModernRenderer renderer;
renderer.SetCullingSystem(&culling);
renderer.EnableFrustumCulling(true);
renderer.EnableOcclusionCulling(false); // HW queries not yet active
```

## Culling Techniques

### 1. Frustum Culling ✅
- Extracts 6 planes from view-projection matrix
- Tests AABB against each plane
- Fast, always enabled

### 2. Distance Culling ✅
- LOD-based culling by distance
- Configurable LOD levels
- Hysteresis to prevent popping

### 3. Occlusion Culling (Planned)
- Hardware occlusion queries
- Two-phase: render occluders, test occludees
- Temporal coherence for stability

### 4. Hierarchical Z-Buffer (Planned)
- Mipmapped depth buffer
- Conservative AABB tests
- GPU-driven culling

## CullingResult

```cpp
struct CullingResult {
    std::vector<void*> visibleChunks;
    std::vector<void*> visibleEntities;
    std::vector<int> chunkLODLevels;
    std::vector<int> entityLODLevels;
    int totalChunksTested = 0;
    int totalEntitiesTested = 0;
    int chunksCulledFrustum = 0;
    int chunksCulledDistance = 0;
    int entitiesCulledFrustum = 0;
    int entitiesCulledDistance = 0;
};
```

## Performance

| Technique | Cost | Effectiveness |
|-----------|------|---------------|
| Frustum | Very Low | High (outdoor) |
| Distance | Low | Medium |
| Occlusion | Medium | Very High (indoor) |
| HZB | Low | High |

## Future Work

- [ ] Hardware occlusion query integration
- [ ] Hierarchical Z-Buffer implementation
- [ ] GPU-driven culling compute shader
- [ ] Portal/sector-based culling
- [ ] Potentially Visible Sets (PVS)

## Status

🟡 **Partial** - Frustum and distance culling active, occlusion culling framework ready

Last Updated: 2026-05-14
