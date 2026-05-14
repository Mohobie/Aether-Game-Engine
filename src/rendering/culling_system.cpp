#include "bvh.h"
#include "frustum.h"
#include "occlusion_culling.h"
#include "editor/in_game_editor.h"
#include "core/logger.h"

namespace vge {

// ============================================
// CullingSystem
// ============================================

CullingSystem::CullingSystem() = default;
CullingSystem::~CullingSystem() = default;

void CullingSystem::setCamera(const Vec3& position, const Mat4& viewProj) {
    cameraPosition = position;
    frustum.update(viewProj);
    distanceCulling.setCameraPosition(position);
}

void CullingSystem::setViewDistance(float distance) {
    viewDistance = distance;
}

void CullingSystem::setupLODs(const std::vector<DistanceCulling::LODLevel>& levels) {
    distanceCulling.levels = levels;
}

void CullingSystem::updateChunkBVH(const std::vector<ChunkBVH::BuildPrimitive>& chunkPrimitives) {
    chunkBVH.build(chunkPrimitives);
}

void CullingSystem::updateEntityBVH(const std::vector<EntityBVH::BuildPrimitive>& entityPrimitives) {
    entityBVH.build(entityPrimitives);
}

CullingResult CullingSystem::cullAll() {
    lastResult = CullingResult();
    cullChunksInternal(lastResult);
    cullEntitiesInternal(lastResult);
    return lastResult;
}

CullingResult CullingSystem::cullChunks() {
    lastResult = CullingResult();
    cullChunksInternal(lastResult);
    return lastResult;
}

CullingResult CullingSystem::cullEntities() {
    lastResult = CullingResult();
    cullEntitiesInternal(lastResult);
    return lastResult;
}

void CullingSystem::setOcclusionCulling(bool enabled) {
    occlusionEnabled = enabled;
}

void CullingSystem::cullChunksInternal(CullingResult& result) {
    // Step 1: Frustum cull via BVH
    std::vector<uint32_t> frustumVisible;
    chunkBVH.frustumQuery(frustum, frustumVisible);
    
    result.totalChunksTested += static_cast<int>(frustumVisible.size());
    
    // Step 2: Distance cull and assign LOD
    std::vector<uint32_t> distanceVisible;
    for (uint32_t payload : frustumVisible) {
        // TODO: Look up actual chunk position for distance test
        // For now, pass through
        distanceVisible.push_back(payload);
        result.chunkLODLevels.push_back(0); // Default LOD
    }
    
    // Step 3: Occlusion cull (if enabled)
    if (occlusionEnabled) {
        static OcclusionCullingSystem occlusionSystem;
        static bool occlusionInitialized = false;
        
        if (!occlusionInitialized) {
            occlusionSystem.Initialize(256);
            occlusionSystem.EnableHardwareQueries(true);
            occlusionSystem.EnableSoftwareFallback(true);
            occlusionSystem.EnableTemporalCoherence(true);
            occlusionInitialized = true;
            Logger::Info("[CullingSystem] Occlusion culling initialized");
        }
        
        occlusionSystem.ClearOccludees();
        
        // Add chunks as occludees
        for (uint32_t payload : distanceVisible) {
            // TODO: Get actual AABB from chunk
            AABB chunkBounds;
            chunkBounds.min = Vec3(-8, -8, -8);
            chunkBounds.max = Vec3(8, 8, 8);
            occlusionSystem.AddOccludee(chunkBounds, 1.0f);
        }
        
        // Perform occlusion culling
        Mat4 viewProj; // TODO: Get from camera
        occlusionSystem.BeginOcclusionPass(viewProj);
        occlusionSystem.RenderOccluders();
        occlusionSystem.TestOccludees();
        occlusionSystem.EndOcclusionPass();
        
        // Get visible chunks
        std::vector<int> visibleIndices = occlusionSystem.GetVisibleOccludees();
        for (int idx : visibleIndices) {
            if (idx >= 0 && idx < static_cast<int>(distanceVisible.size())) {
                result.visibleChunks.push_back(
                    reinterpret_cast<void*>(static_cast<uintptr_t>(distanceVisible[idx]))
                );
            }
        }
        
        result.chunksCulledFrustum = result.totalChunksTested - static_cast<int>(result.visibleChunks.size());
        
        Logger::Debug("[CullingSystem] Occlusion culled " + 
                     std::to_string(occlusionSystem.GetCulledCount()) + " chunks");
    } else {
        // No occlusion culling
        for (uint32_t payload : distanceVisible) {
            result.visibleChunks.push_back(reinterpret_cast<void*>(static_cast<uintptr_t>(payload)));
        }
        result.chunksCulledFrustum = result.totalChunksTested - static_cast<int>(result.visibleChunks.size());
    }
}

void CullingSystem::cullEntitiesInternal(CullingResult& result) {
    // Step 1: Frustum cull via BVH
    std::vector<uint32_t> frustumVisible;
    entityBVH.frustumQuery(frustum, frustumVisible);
    
    result.totalEntitiesTested += static_cast<int>(frustumVisible.size());
    
    // Step 2: Distance cull
    std::vector<uint32_t> distanceVisible;
    for (uint32_t payload : frustumVisible) {
        distanceVisible.push_back(payload);
        result.entityLODLevels.push_back(0); // Default LOD
    }
    
    // Step 3: Occlusion cull (if enabled)
    if (occlusionEnabled) {
        static OcclusionCullingSystem occlusionSystem;
        static bool occlusionInitialized = false;
        
        if (!occlusionInitialized) {
            occlusionSystem.Initialize(256);
            occlusionSystem.EnableHardwareQueries(true);
            occlusionSystem.EnableSoftwareFallback(true);
            occlusionSystem.EnableTemporalCoherence(true);
            occlusionInitialized = true;
        }
        
        occlusionSystem.ClearOccludees();
        
        // Add entities as occludees
        for (uint32_t payload : distanceVisible) {
            // TODO: Get actual AABB from entity
            AABB entityBounds;
            entityBounds.min = Vec3(-1, -1, -1);
            entityBounds.max = Vec3(1, 1, 1);
            occlusionSystem.AddOccludee(entityBounds, 1.0f);
        }
        
        // Perform occlusion culling
        Mat4 viewProj; // TODO: Get from camera
        occlusionSystem.BeginOcclusionPass(viewProj);
        occlusionSystem.RenderOccluders();
        occlusionSystem.TestOccludees();
        occlusionSystem.EndOcclusionPass();
        
        // Get visible entities
        std::vector<int> visibleIndices = occlusionSystem.GetVisibleOccludees();
        for (int idx : visibleIndices) {
            if (idx >= 0 && idx < static_cast<int>(distanceVisible.size())) {
                result.visibleEntities.push_back(
                    reinterpret_cast<void*>(static_cast<uintptr_t>(distanceVisible[idx]))
                );
            }
        }
        
        result.entitiesCulledFrustum = result.totalEntitiesTested - static_cast<int>(result.visibleEntities.size());
    } else {
        // No occlusion culling
        for (uint32_t payload : distanceVisible) {
            result.visibleEntities.push_back(reinterpret_cast<void*>(static_cast<uintptr_t>(payload)));
        }
        result.entitiesCulledFrustum = result.totalEntitiesTested - static_cast<int>(result.visibleEntities.size());
    }
}

} // namespace vge
