#pragma once

#include "math/vec3.h"
#include "math/mat4.h"
#include "frustum.h"
#include <vector>
#include <memory>

namespace vge {

// Forward declarations
struct AABB;
class Chunk;
class Entity;

// ============================================
// Culling System
// ============================================
// Integrates frustum, distance, and occlusion culling
// for efficient render pipeline usage

class CullingSystem {
public:
    CullingSystem();
    ~CullingSystem();

    void setCamera(const Vec3& position, const Mat4& viewProj);
    void setViewDistance(float distance);
    
    CullingResult cullAll();
    CullingResult cullChunks();
    CullingResult cullEntities();
    
    const CullingResult& getLastResult() const { return lastResult; }
    
private:
    Vec3 cameraPosition;
    Mat4 viewProjMatrix;
    Frustum frustum;
    float viewDistance = 1000.0f;
    CullingResult lastResult;
    
    void cullChunksInternal(CullingResult& result);
    void cullEntitiesInternal(CullingResult& result);
};

} // namespace vge
