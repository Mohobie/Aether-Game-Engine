#pragma once

#include "math/vec3.h"
#include "math/mat4.h"
#include "frustum.h"
#include <vector>
#include <cstdint>

namespace vge {

// ============================================
// Occlusion Query (Hardware-Accelerated)
// ============================================
// Uses OpenGL occlusion queries to test if objects
// are visible behind other geometry

class OcclusionQuery {
private:
    uint32_t queryID;
    bool active;
    
public:
    OcclusionQuery();
    ~OcclusionQuery();
    
    // Initialize query object
    void Initialize();
    void Shutdown();
    
    // Begin occlusion test for an object
    void BeginQuery();
    
    // End occlusion test
    void EndQuery();
    
    // Check if any pixels passed (object is visible)
    bool IsVisible() const;
    
    // Get number of visible pixels
    uint32_t GetVisiblePixels() const;
    
    // Check if query result is available
    bool IsResultAvailable() const;
    
    // Wait for result (blocking)
    void WaitForResult() const;
    
    uint32_t GetID() const { return queryID; }
    bool IsActive() const { return active; }
};

// ============================================
// Occlusion Culling System
// ============================================
// Two-phase occlusion culling:
// Phase 1: Render large occluders (build depth buffer)
// Phase 2: Test smaller objects against depth buffer

struct Occluder {
    AABB bounds;
    float effectiveRadius;  // Size for LOD selection
    uint32_t queryID;       // Associated query
    bool isVisible;         // Last frame result
    int framesSinceVisible; // For temporal coherence
};

struct Occludee {
    AABB bounds;
    uint32_t queryID;
    bool isVisible;
    int framesSinceVisible;
    float priority;         // For query budget allocation
};

class OcclusionCullingSystem {
private:
    // Query pool
    std::vector<OcclusionQuery> queryPool;
    std::vector<bool> queryInUse;
    uint32_t maxQueries;
    
    // Occluders (large objects that block view)
    std::vector<Occluder> occluders;
    
    // Occludees (objects to test)
    std::vector<Occludee> occludees;
    
    // Temporal coherence
    int framesToKeepVisible;  // Keep rendering for N frames after visible
    
    // Query budget
    uint32_t maxQueriesPerFrame;
    uint32_t queriesUsedThisFrame;
    
    // Depth buffer for software fallback
    std::vector<float> depthBuffer;
    int depthBufferWidth;
    int depthBufferHeight;
    Mat4 lastViewProj;
    
    // Settings
    bool useHardwareQueries;
    bool useSoftwareFallback;
    bool useTemporalCoherence;
    
public:
    OcclusionCullingSystem();
    ~OcclusionCullingSystem();
    
    void Initialize(uint32_t maxQueries = 256);
    void Shutdown();
    
    // Configuration
    void SetMaxQueriesPerFrame(uint32_t max);
    void SetFramesToKeepVisible(int frames);
    void EnableHardwareQueries(bool enable);
    void EnableSoftwareFallback(bool enable);
    void EnableTemporalCoherence(bool enable);
    
    // Occluder management
    void ClearOccluders();
    void AddOccluder(const AABB& bounds, float effectiveRadius);
    void UpdateOccluder(int index, const AABB& bounds);
    
    // Occludee management
    void ClearOccludees();
    void AddOccludee(const AABB& bounds, float priority = 1.0f);
    
    // Two-phase culling
    void BeginOcclusionPass(const Mat4& viewProj);
    void RenderOccluders();  // Phase 1: Build depth buffer
    void TestOccludees();    // Phase 2: Test visibility
    void EndOcclusionPass();
    
    // Get visible objects after culling
    std::vector<int> GetVisibleOccludees() const;
    std::vector<int> GetVisibleOccluders() const;
    
    // Check if specific object is visible
    bool IsOccludeeVisible(int index) const;
    bool IsOccluderVisible(int index) const;
    
    // Stats
    uint32_t GetTotalQueries() const { return maxQueries; }
    uint32_t GetQueriesUsed() const { return queriesUsedThisFrame; }
    uint32_t GetVisibleCount() const;
    uint32_t GetCulledCount() const;
    
    // Software fallback depth buffer
    void UpdateDepthBuffer(const std::vector<float>& depths, int width, int height, const Mat4& viewProj);
    bool TestAABBAgainstDepthBuffer(const AABB& bounds, const Mat4& viewProj) const;
    
private:
    // Query management
    int AcquireQuery();
    void ReleaseQuery(int index);
    void ReleaseAllQueries();
    
    // Software rasterization fallback
    void RasterizeOccluderToDepthBuffer(const Occluder& occluder);
    bool TestOccludeeAgainstDepthBuffer(const Occludee& occludee) const;
    
    // Temporal coherence
    void UpdateTemporalCoherence();
    bool ShouldTestObject(int occludeeIndex) const;
};

// ============================================
// Hierarchical Z-Buffer (HZB) for GPU Culling
// ============================================
// Builds mipmapped depth buffer for fast conservative tests

class HierarchicalZBuffer {
private:
    std::vector<std::vector<float>> mipChain;
    int baseWidth;
    int baseHeight;
    int numLevels;
    
public:
    HierarchicalZBuffer();
    ~HierarchicalZBuffer();
    
    void Initialize(int width, int height);
    void Shutdown();
    
    // Build HZB from depth buffer
    void BuildFromDepthBuffer(const std::vector<float>& depthBuffer, int width, int height);
    
    // Test AABB against HZB (conservative)
    bool TestAABB(const AABB& bounds, const Mat4& viewProj) const;
    
    // Get depth at specific mip level
    float GetDepth(int x, int y, int mipLevel) const;
    
    // Get mip dimensions
    int GetWidth(int mipLevel) const;
    int GetHeight(int mipLevel) const;
    int GetNumLevels() const { return numLevels; }
    
private:
    void BuildMipChain();
    float SampleDepth(int x, int y, int level) const;
};

// ============================================
// GPU-Driven Culling (Compute Shader)
// ============================================
// Uses compute shaders for massive parallel culling

struct CullData {
    Mat4 viewMatrix;
    Mat4 projMatrix;
    Mat4 viewProjMatrix;
    Vec3 cameraPosition;
    float cameraNear;
    float cameraFar;
    float fov;
    float aspectRatio;
    uint32_t numObjects;
    uint32_t frameIndex;
};

struct GPUDrawCommand {
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t baseVertex;
    uint32_t baseInstance;
    uint32_t objectID;  // For material lookup
};

class GPUCullingSystem {
private:
    // OpenGL compute shader
    uint32_t cullComputeShader;
    uint32_t cullProgram;
    
    // SSBOs
    uint32_t objectDataSSBO;
    uint32_t drawCommandsSSBO;
    uint32_t cullDataSSBO;
    
    // Object data
    struct GPUObjectData {
        Vec3 center;
        float radius;
        Vec3 aabbMin;
        float pad1;
        Vec3 aabbMax;
        float pad2;
    };
    std::vector<GPUObjectData> objectData;
    
    // Draw commands
    std::vector<GPUDrawCommand> drawCommands;
    
    // Indirect draw buffer
    uint32_t indirectDrawBuffer;
    
    // Settings
    bool initialized;
    uint32_t maxObjects;
    
public:
    GPUCullingSystem();
    ~GPUCullingSystem();
    
    void Initialize(uint32_t maxObjects = 65536);
    void Shutdown();
    
    // Object management
    void ClearObjects();
    void AddObject(const Vec3& center, float radius, const AABB& bounds, uint32_t indexCount, uint32_t firstIndex);
    void UpdateObject(int index, const Vec3& center, float radius);
    
    // Cull on GPU
    void PerformCulling(const CullData& cullData);
    
    // Get results
    uint32_t GetVisibleCount() const;
    std::vector<GPUDrawCommand> GetVisibleCommands() const;
    
    // Indirect draw
    uint32_t GetIndirectDrawBuffer() const { return indirectDrawBuffer; }
    void ExecuteIndirectDraw() const;
    
    // Stats
    uint32_t GetTotalObjects() const { return static_cast<uint32_t>(objectData.size()); }
    bool IsInitialized() const { return initialized; }
    
private:
    // Compute shader source
    static const char* GetCullComputeShaderSource();
    
    // Buffer management
    void UpdateObjectDataBuffer();
    void UpdateDrawCommandsBuffer();
    void ReadBackResults();
};

} // namespace vge
