# Occlusion Culling System

## Overview

The Occlusion Culling System prevents rendering objects that are hidden behind other geometry. This provides significant performance improvements in complex scenes with many overlapping objects.

## Architecture

### Three-Tier Approach

| Tier | Method | Use Case |
|------|--------|----------|
| **Hardware Queries** | OpenGL occlusion queries | Primary method on modern GPUs |
| **Software Fallback** | CPU depth buffer testing | Fallback when queries unavailable |
| **Temporal Coherence** | Frame-to-frame coherence | Reduces query overhead |

## Components

### OcclusionQuery

Hardware-accelerated pixel counting using OpenGL queries.

```cpp
OcclusionQuery query;
query.Initialize();

// Test object visibility
query.BeginQuery();
RenderBoundingBox(objectBounds);
query.EndQuery();

bool visible = query.IsVisible();
uint32_t pixels = query.GetVisiblePixels();
```

### OcclusionCullingSystem

Main system orchestrating occlusion culling.

```cpp
OcclusionCullingSystem occlusion;
occlusion.Initialize(256);  // 256 query objects

// Configure
occlusion.EnableHardwareQueries(true);
occlusion.EnableSoftwareFallback(true);
occlusion.EnableTemporalCoherence(true);
occlusion.SetMaxQueriesPerFrame(128);
occlusion.SetFramesToKeepVisible(3);

// Add occluders (large objects that block view)
occlusion.AddOccluder(largeBuildingBounds, 50.0f);
occlusion.AddOccluder(terrainBounds, 100.0f);

// Add occludees (objects to test)
for (auto& object : sceneObjects) {
    occlusion.AddOccludee(object.bounds, object.priority);
}

// Perform culling
occlusion.BeginOcclusionPass(viewProjMatrix);
occlusion.RenderOccluders();      // Phase 1: Build depth
occlusion.TestOccludees();         // Phase 2: Test visibility
occlusion.EndOcclusionPass();      // Phase 3: Read results

// Get visible objects
auto visible = occlusion.GetVisibleOccludees();
```

### Hierarchical Z-Buffer (HZB)

Mipmap-based depth buffer for fast conservative tests.

```cpp
HierarchicalZBuffer hzb;
hzb.Initialize(screenWidth, screenHeight);

// Build from depth buffer
hzb.BuildFromDepthBuffer(depthBuffer, width, height);

// Test AABB (very fast, conservative)
bool visible = hzb.TestAABB(objectBounds, viewProj);
```

### GPU Culling (Compute Shader)

Massively parallel culling using compute shaders.

```cpp
GPUCullingSystem gpuCull;
gpuCull.Initialize(65536);  // Max 65k objects

// Add objects
for (auto& obj : objects) {
    gpuCull.AddObject(obj.center, obj.radius, obj.bounds,
                     obj.mesh.indexCount, obj.mesh.firstIndex);
}

// Set cull data
CullData cullData;
cullData.viewMatrix = camera.GetViewMatrix();
cullData.projMatrix = camera.GetProjectionMatrix();
cullData.viewProjMatrix = camera.GetViewProjMatrix();
cullData.cameraPosition = camera.GetPosition();
cullData.numObjects = gpuCull.GetTotalObjects();

// Cull on GPU
gpuCull.PerformCulling(cullData);

// Execute indirect draw
uint32_t visibleCount = gpuCull.GetVisibleCount();
gpuCull.ExecuteIndirectDraw();
```

## Integration with CullingSystem

The occlusion culling integrates with the existing CullingSystem:

```cpp
CullingSystem culling;
culling.setOcclusionCulling(true);  // Enable occlusion culling

// Existing frustum + distance culling still work
culling.setCamera(position, viewProj);
culling.setViewDistance(1000.0f);

// Occlusion culling automatically applied
CullingResult result = culling.cullAll();
```

## Performance Tips

1. **Use large occluders** - Buildings, terrain work better than small objects
2. **Temporal coherence** - Keep rendering objects for 2-3 frames after visible
3. **Query budget** - Limit queries per frame to avoid GPU stalls
4. **HZB for particles** - Use HZB for particle systems (many small objects)
5. **GPU culling for crowds** - Use compute shaders for large crowds

## Stats

```cpp
// Get culling statistics
uint32_t totalQueries = occlusion.GetTotalQueries();
uint32_t queriesUsed = occlusion.GetQueriesUsed();
uint32_t visibleCount = occlusion.GetVisibleCount();
uint32_t culledCount = occlusion.GetCulledCount();

Logger::Info("Occlusion: " + std::to_string(visibleCount) + " visible, " +
             std::to_string(culledCount) + " culled (" +
             std::to_string(queriesUsed) + "/" +
             std::to_string(totalQueries) + " queries)");
```

## Future Improvements

- [ ] Async readback (avoid GPU stalls)
- [ ] Predicated rendering (skip draw calls entirely)
- [ ] Multi-frame query aggregation
- [ ] Occluder simplification (auto-generate LODs)
- [ ] Portal-based culling for indoor scenes
