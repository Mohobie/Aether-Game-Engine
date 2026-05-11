#include "bvh.h"
#include "frustum.h"
#include "editor/in_game_editor.h"

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
    for (uint32_t payload : frustumVisible) {
        // For now, use payload as chunk index
        // In real implementation, we'd look up chunk position
        // For testing, just pass through
        result.visibleChunks.push_back(reinterpret_cast<void*>(static_cast<uintptr_t>(payload)));
        result.chunkLODLevels.push_back(0); // Default LOD
    }
    
    result.chunksCulledFrustum = result.totalChunksTested - static_cast<int>(result.visibleChunks.size());
}

void CullingSystem::cullEntitiesInternal(CullingResult& result) {
    // Step 1: Frustum cull via BVH
    std::vector<uint32_t> frustumVisible;
    entityBVH.frustumQuery(frustum, frustumVisible);
    
    result.totalEntitiesTested += static_cast<int>(frustumVisible.size());
    
    // Step 2: Distance cull
    for (uint32_t payload : frustumVisible) {
        result.visibleEntities.push_back(reinterpret_cast<void*>(static_cast<uintptr_t>(payload)));
        result.entityLODLevels.push_back(0); // Default LOD
    }
    
    result.entitiesCulledFrustum = result.totalEntitiesTested - static_cast<int>(result.visibleEntities.size());
}

} // namespace vge
