#include "culling_system.h"
#include "core/logger.h"
#include <cmath>

namespace vge {

CullingSystem::CullingSystem() = default;
CullingSystem::~CullingSystem() = default;

void CullingSystem::setCamera(const Vec3& position, const Mat4& viewProj) {
    cameraPosition = position;
    viewProjMatrix = viewProj;
    frustum.update(viewProj);
}

void CullingSystem::setViewDistance(float distance) {
    viewDistance = distance;
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

void CullingSystem::cullChunksInternal(CullingResult& result) {
    // TODO: Integrate with actual chunk system
    // For now, mark all as visible
    Logger::Debug("[Culling] Chunk culling not yet integrated");
}

void CullingSystem::cullEntitiesInternal(CullingResult& result) {
    // TODO: Integrate with actual entity system
    // For now, mark all as visible
    Logger::Debug("[Culling] Entity culling not yet integrated");
}

} // namespace vge
