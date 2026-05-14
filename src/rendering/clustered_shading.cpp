#include "clustered_shading.h"
#include "core/logger.h"
#include <cmath>
#include <algorithm>

namespace vge {

// ============================================
// ClusterAABB
// ============================================

bool ClusterAABB::IntersectsSphere(const Vec3& center, float radius) const {
    // Find closest point on AABB to sphere center
    Vec3 closest(
        std::max(minBounds.x, std::min(center.x, maxBounds.x)),
        std::max(minBounds.y, std::min(center.y, maxBounds.y)),
        std::max(minBounds.z, std::min(center.z, maxBounds.z))
    );
    
    float distSq = (closest - center).LengthSquared();
    return distSq <= radius * radius;
}

bool ClusterAABB::IntersectsAABB(const Vec3& aabbMin, const Vec3& aabbMax) const {
    return (minBounds.x <= aabbMax.x && maxBounds.x >= aabbMin.x) &&
           (minBounds.y <= aabbMax.y && maxBounds.y >= aabbMin.y) &&
           (minBounds.z <= aabbMax.z && maxBounds.z >= aabbMin.z);
}

// ============================================
// ClusteredShadingSystem
// ============================================

ClusteredShadingSystem::ClusteredShadingSystem()
    : initialized(false)
    , screenWidth(1920)
    , screenHeight(1080)
    , lightListBuffer(0)
    , clusterDataBuffer(0)
    , lightDataBuffer(0)
    , nearPlane(0.1f)
    , farPlane(1000.0f)
    , sliceScale(0.0f)
    , sliceBias(0.0f)
{
    clusters.fill(ClusterAABB{});
    clusterLights.fill(ClusterLightList{});
}

ClusteredShadingSystem::~ClusteredShadingSystem() {
    Shutdown();
}

void ClusteredShadingSystem::Initialize(uint32_t width, uint32_t height) {
    screenWidth = width;
    screenHeight = height;
    
    // Create GPU buffers
    glGenBuffers(1, &lightListBuffer);
    glGenBuffers(1, &clusterDataBuffer);
    glGenBuffers(1, &lightDataBuffer);
    
    // Initialize cluster data buffer
    size_t clusterDataSize = clusters.size() * sizeof(ClusterAABB);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clusterDataBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, clusterDataSize, nullptr, GL_STATIC_DRAW);
    
    // Initialize light list buffer
    size_t lightListSize = clusters.size() * MAX_LIGHTS_PER_CLUSTER * sizeof(uint32_t);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightListBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lightListSize, nullptr, GL_DYNAMIC_DRAW);
    
    // Initialize light data buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightDataBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_TOTAL_LIGHTS * sizeof(LightGPUData), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    initialized = true;
    Logger::Info("[ClusteredShading] Initialized " + std::to_string(CLUSTER_GRID_X) + "x" +
                 std::to_string(CLUSTER_GRID_Y) + "x" + std::to_string(CLUSTER_GRID_Z) +
                 " cluster grid (" + std::to_string(GetTotalClusters()) + " total clusters)");
}

void ClusteredShadingSystem::Shutdown() {
    if (lightListBuffer) glDeleteBuffers(1, &lightListBuffer);
    if (clusterDataBuffer) glDeleteBuffers(1, &clusterDataBuffer);
    if (lightDataBuffer) glDeleteBuffers(1, &lightDataBuffer);
    
    lightListBuffer = 0;
    clusterDataBuffer = 0;
    lightDataBuffer = 0;
    initialized = false;
}

void ClusteredShadingSystem::UpdateClusters(const Mat4& viewMatrix, const Mat4& projMatrix,
                                           float near, float far) {
    if (!initialized) return;
    
    nearPlane = near;
    farPlane = far;
    
    // Calculate depth slicing parameters
    // Use exponential depth distribution for better precision near camera
    sliceScale = static_cast<float>(CLUSTER_GRID_Z) / std::log2(farPlane / nearPlane);
    sliceBias = static_cast<float>(CLUSTER_GRID_Z) * std::log2(nearPlane) / std::log2(farPlane / nearPlane);
    
    // Build cluster grid
    Mat4 invProj = projMatrix.Inverted();
    BuildClusterGrid(invProj);
    
    // Upload cluster data to GPU
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clusterDataBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, clusters.size() * sizeof(ClusterAABB), clusters.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ClusteredShadingSystem::BuildClusterGrid(const Mat4& invProj) {
    // Build AABB for each cluster in view space
    for (uint32_t z = 0; z < CLUSTER_GRID_Z; ++z) {
        // Calculate depth bounds for this slice
        float zNear = nearPlane * std::pow(farPlane / nearPlane, static_cast<float>(z) / CLUSTER_GRID_Z);
        float zFar = nearPlane * std::pow(farPlane / nearPlane, static_cast<float>(z + 1) / CLUSTER_GRID_Z);
        
        for (uint32_t y = 0; y < CLUSTER_GRID_Y; ++y) {
            for (uint32_t x = 0; x < CLUSTER_GRID_X; ++x) {
                // Calculate screen-space tile bounds
                float tileMinX = static_cast<float>(x) / CLUSTER_GRID_X * 2.0f - 1.0f;
                float tileMaxX = static_cast<float>(x + 1) / CLUSTER_GRID_X * 2.0f - 1.0f;
                float tileMinY = static_cast<float>(y) / CLUSTER_GRID_Y * 2.0f - 1.0f;
                float tileMaxY = static_cast<float>(y + 1) / CLUSTER_GRID_Y * 2.0f - 1.0f;
                
                // Transform to view space
                Vec4 corners[8];
                corners[0] = invProj * Vec4(tileMinX, tileMinY, -1.0f, 1.0f);
                corners[1] = invProj * Vec4(tileMaxX, tileMinY, -1.0f, 1.0f);
                corners[2] = invProj * Vec4(tileMinX, tileMaxY, -1.0f, 1.0f);
                corners[3] = invProj * Vec4(tileMaxX, tileMaxY, -1.0f, 1.0f);
                corners[4] = invProj * Vec4(tileMinX, tileMinY, 1.0f, 1.0f);
                corners[5] = invProj * Vec4(tileMaxX, tileMinY, 1.0f, 1.0f);
                corners[6] = invProj * Vec4(tileMinX, tileMaxY, 1.0f, 1.0f);
                corners[7] = invProj * Vec4(tileMaxX, tileMaxY, 1.0f, 1.0f);
                
                // Calculate AABB
                Vec3 minBounds(std::numeric_limits<float>::max());
                Vec3 maxBounds(std::numeric_limits<float>::lowest());
                
                for (int i = 0; i < 8; ++i) {
                    corners[i] /= corners[i].w;
                    minBounds.x = std::min(minBounds.x, corners[i].x);
                    minBounds.y = std::min(minBounds.y, corners[i].y);
                    minBounds.z = std::min(minBounds.z, corners[i].z);
                    maxBounds.x = std::max(maxBounds.x, corners[i].x);
                    maxBounds.y = std::max(maxBounds.y, corners[i].y);
                    maxBounds.z = std::max(maxBounds.z, corners[i].z);
                }
                
                // Scale by depth
                minBounds *= zNear;
                maxBounds *= zFar;
                
                uint32_t index = z * CLUSTER_GRID_Y * CLUSTER_GRID_X + y * CLUSTER_GRID_X + x;
                clusters[index].minBounds = minBounds;
                clusters[index].maxBounds = maxBounds;
            }
        }
    }
}

void ClusteredShadingSystem::AssignLights(const std::vector<Light>& lights) {
    if (!initialized) return;
    
    // Clear previous assignments
    for (auto& list : clusterLights) {
        list.Clear();
    }
    
    // Assign each light to overlapping clusters
    for (uint32_t lightIdx = 0; lightIdx < lights.size() && lightIdx < MAX_TOTAL_LIGHTS; ++lightIdx) {
        const Light& light = lights[lightIdx];
        
        // Calculate light influence radius based on intensity and attenuation
        float radius = std::sqrt(light.intensity / 0.01f);  // Threshold at 1% intensity
        
        // Simple brute-force assignment (can be optimized with BVH)
        for (uint32_t i = 0; i < clusters.size(); ++i) {
            if (clusters[i].IntersectsSphere(light.position, radius)) {
                clusterLights[i].AddLight(lightIdx);
            }
        }
    }
    
    // Build light list texture for GPU
    BuildLightListTexture();
}

void ClusteredShadingSystem::BuildLightListTexture() {
    // Pack light list data for GPU
    std::vector<uint32_t> lightListData;
    lightListData.reserve(clusters.size() * MAX_LIGHTS_PER_CLUSTER);
    
    for (const auto& list : clusterLights) {
        // First element is count
        lightListData.push_back(list.count);
        // Followed by light indices
        for (uint32_t i = 0; i < list.count; ++i) {
            lightListData.push_back(list.lightIndices[i]);
        }
        // Pad to MAX_LIGHTS_PER_CLUSTER + 1
        for (uint32_t i = list.count; i < MAX_LIGHTS_PER_CLUSTER; ++i) {
            lightListData.push_back(0);
        }
    }
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightListBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, lightListData.size() * sizeof(uint32_t), lightListData.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

const ClusterLightList& ClusteredShadingSystem::GetClusterLights(uint32_t x, uint32_t y, uint32_t z) const {
    uint32_t index = z * CLUSTER_GRID_Y * CLUSTER_GRID_X + y * CLUSTER_GRID_X + x;
    return clusterLights[index];
}

const ClusterLightList& ClusteredShadingSystem::GetClusterLights(uint32_t flatIndex) const {
    return clusterLights[flatIndex];
}

uint32_t ClusteredShadingSystem::GetClusterIndex(uint32_t screenX, uint32_t screenY, float viewSpaceZ) const {
    uint32_t x = static_cast<uint32_t>(static_cast<float>(screenX) / screenWidth * CLUSTER_GRID_X);
    uint32_t y = static_cast<uint32_t>(static_cast<float>(screenY) / screenHeight * CLUSTER_GRID_Y);
    uint32_t z = GetSliceFromDepth(viewSpaceZ);
    
    x = std::min(x, CLUSTER_GRID_X - 1);
    y = std::min(y, CLUSTER_GRID_Y - 1);
    z = std::min(z, CLUSTER_GRID_Z - 1);
    
    return z * CLUSTER_GRID_Y * CLUSTER_GRID_X + y * CLUSTER_GRID_X + x;
}

uint32_t ClusteredShadingSystem::GetSliceFromDepth(float viewSpaceZ) const {
    // Exponential depth slicing
    float logDepth = std::log2(-viewSpaceZ);
    uint32_t slice = static_cast<uint32_t>((logDepth - std::log2(nearPlane)) * CLUSTER_GRID_Z / 
                                            (std::log2(farPlane) - std::log2(nearPlane)));
    return std::min(slice, CLUSTER_GRID_Z - 1);
}

const ClusterAABB& ClusteredShadingSystem::GetClusterAABB(uint32_t x, uint32_t y, uint32_t z) const {
    uint32_t index = z * CLUSTER_GRID_Y * CLUSTER_GRID_X + y * CLUSTER_GRID_X + x;
    return clusters[index];
}

uint32_t ClusteredShadingSystem::GetActiveClusters() const {
    uint32_t active = 0;
    for (const auto& list : clusterLights) {
        if (list.count > 0) ++active;
    }
    return active;
}

float ClusteredShadingSystem::GetAverageLightsPerCluster() const {
    uint32_t totalLights = 0;
    for (const auto& list : clusterLights) {
        totalLights += list.count;
    }
    return static_cast<float>(totalLights) / static_cast<float>(clusters.size());
}

uint32_t ClusteredShadingSystem::GetMaxLightsInCluster() const {
    uint32_t maxLights = 0;
    for (const auto& list : clusterLights) {
        maxLights = std::max(maxLights, list.count);
    }
    return maxLights;
}

void ClusteredShadingSystem::FlattenClusterIndex(uint32_t x, uint32_t y, uint32_t z, uint32_t& out) const {
    out = z * CLUSTER_GRID_Y * CLUSTER_GRID_X + y * CLUSTER_GRID_X + x;
}

} // namespace vge
