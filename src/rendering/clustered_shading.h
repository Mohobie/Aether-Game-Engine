#pragma once

#include "math/vec3.h"
#include "math/mat4.h"
#include "rendering/light_system.h"
#include <vector>
#include <array>
#include <memory>

namespace vge {

// ============================================
// Clustered Shading System
// ============================================
// Assigns lights to 3D clusters for efficient deferred shading
// Reduces light evaluation from O(num_lights) to O(lights_in_cluster)

// Cluster grid dimensions
constexpr uint32_t CLUSTER_GRID_X = 16;
constexpr uint32_t CLUSTER_GRID_Y = 9;
constexpr uint32_t CLUSTER_GRID_Z = 24;
constexpr uint32_t MAX_LIGHTS_PER_CLUSTER = 128;
constexpr uint32_t MAX_TOTAL_LIGHTS = 1024;

// ============================================
// Cluster AABB
// ============================================
struct ClusterAABB {
    Vec3 minBounds;
    Vec3 maxBounds;
    
    bool IntersectsSphere(const Vec3& center, float radius) const;
    bool IntersectsAABB(const Vec3& aabbMin, const Vec3& aabbMax) const;
};

// ============================================
// Light Assignment
// ============================================
struct ClusterLightList {
    uint32_t count;
    std::array<uint32_t, MAX_LIGHTS_PER_CLUSTER> lightIndices;
    
    ClusterLightList() : count(0) {}
    
    void AddLight(uint32_t index) {
        if (count < MAX_LIGHTS_PER_CLUSTER) {
            lightIndices[count++] = index;
        }
    }
    
    void Clear() { count = 0; }
};

// ============================================
// Clustered Shading System
// ============================================
class ClusteredShadingSystem {
public:
    ClusteredShadingSystem();
    ~ClusteredShadingSystem();
    
    // Initialize with screen dimensions
    void Initialize(uint32_t screenWidth, uint32_t screenHeight);
    void Shutdown();
    
    // Update clusters for current frame
    void UpdateClusters(const Mat4& viewMatrix, const Mat4& projMatrix,
                       float nearPlane, float farPlane);
    
    // Assign lights to clusters
    void AssignLights(const std::vector<Light>& lights);
    
    // Get light list for a cluster
    const ClusterLightList& GetClusterLights(uint32_t x, uint32_t y, uint32_t z) const;
    const ClusterLightList& GetClusterLights(uint32_t flatIndex) const;
    
    // Get cluster index from screen position and depth
    uint32_t GetClusterIndex(uint32_t screenX, uint32_t screenY, float viewSpaceZ) const;
    
    // Get cluster AABB
    const ClusterAABB& GetClusterAABB(uint32_t x, uint32_t y, uint32_t z) const;
    
    // Statistics
    uint32_t GetTotalClusters() const { return CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z; }
    uint32_t GetActiveClusters() const;
    float GetAverageLightsPerCluster() const;
    uint32_t GetMaxLightsInCluster() const;
    
    // GPU resources
    void BuildLightListTexture();  // Build SSBO/light list for GPU
    uint32_t GetLightListBuffer() const { return lightListBuffer; }
    uint32_t GetClusterDataBuffer() const { return clusterDataBuffer; }
    
private:
    bool initialized;
    uint32_t screenWidth, screenHeight;
    
    // Cluster grid
    std::array<ClusterAABB, CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z> clusters;
    std::array<ClusterLightList, CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z> clusterLights;
    
    // GPU buffers
    uint32_t lightListBuffer;      // SSBO: light indices per cluster
    uint32_t clusterDataBuffer;    // SSBO: cluster AABBs
    uint32_t lightDataBuffer;      // SSBO: light data
    
    // Depth slicing
    float nearPlane, farPlane;
    float sliceScale, sliceBias;
    
    void BuildClusterGrid(const Mat4& invProj);
    uint32_t GetSliceFromDepth(float viewSpaceZ) const;
    void FlattenClusterIndex(uint32_t x, uint32_t y, uint32_t z, uint32_t& out) const;
};

} // namespace vge
