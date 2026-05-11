#pragma once

#include "math/vec3.h"
#include "frustum.h"
#include "editor/in_game_editor.h"  // For AABB
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

namespace vge {

/**
 * @brief Simple ray structure for BVH queries
 */
struct Ray {
    Vec3 origin;
    Vec3 direction;
    
    Ray() : origin(0, 0, 0), direction(0, 0, 1) {}
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d) {}
    
    Vec3 at(float t) const {
        return origin + direction * t;
    }
};

/**
 * @brief Bounding Volume Hierarchy for spatial queries
 * @tparam TPayload User data pointer or index
 */
template<typename TPayload = uint32_t>
class BVHTree {
public:
    struct Node {
        AABB bounds;
        bool isLeaf = false;
        uint32_t leftChild = 0;
        uint32_t rightChild = 0;
        uint32_t startIndex = 0;  // For leaf: index into primitives
        uint32_t count = 0;       // For leaf: number of primitives
    };

    struct BuildPrimitive {
        AABB bounds;
        TPayload payload;
        Vec3 center;
    };

    BVHTree() = default;
    ~BVHTree() = default;

    /// @brief Build BVH from primitives
    void build(const std::vector<BuildPrimitive>& primitives);

    /// @brief Clear all nodes
    void clear();

    /// @brief Query all nodes intersecting frustum
    void frustumQuery(const Frustum& frustum, std::vector<TPayload>& results) const;

    /// @brief Query all nodes intersecting AABB
    void aabbQuery(const AABB& queryBounds, std::vector<TPayload>& results) const;

    /// @brief Ray query: all payloads intersected by ray
    void rayQuery(const Ray& ray, std::vector<TPayload>& results) const;

    /// @brief Ray query with distance: sorted by hit distance
    void rayQuerySorted(const Ray& ray, std::vector<std::pair<TPayload, float>>& results) const;

    /// @brief Get total node count
    size_t nodeCount() const { return nodes.size(); }

    /// @brief Get tree depth (for diagnostics)
    uint32_t getDepth() const;

    /// @brief Get root bounds
    const AABB* getRootBounds() const {
        return nodes.empty() ? nullptr : &nodes[0].bounds;
    }

private:
    std::vector<Node> nodes;
    std::vector<BuildPrimitive> primitives;

    uint32_t buildRecursive(uint32_t start, uint32_t end, uint32_t depth);
    void frustumQueryRecursive(uint32_t nodeIndex, const Frustum& frustum, std::vector<TPayload>& results) const;
    void aabbQueryRecursive(uint32_t nodeIndex, const AABB& queryBounds, std::vector<TPayload>& results) const;
    void rayQueryRecursive(uint32_t nodeIndex, const Ray& ray, float tMin, float tMax, std::vector<std::pair<TPayload, float>>& results) const;

    static constexpr uint32_t MAX_DEPTH = 32;
    static constexpr uint32_t LEAF_SIZE = 8;
};

/**
 * @brief BVH specialized for chunks
 */
using ChunkBVH = BVHTree<uint32_t>;

/**
 * @brief BVH specialized for entities
 */
using EntityBVH = BVHTree<uint32_t>;

/**
 * @brief Main culling system orchestrating all culling passes
 */
class CullingSystem {
public:
    CullingSystem();
    ~CullingSystem();

    /// @brief Set camera for culling
    void setCamera(const Vec3& position, const Mat4& viewProj);

    /// @brief Set view distance
    void setViewDistance(float distance);

    /// @brief Setup LOD levels
    void setupLODs(const std::vector<DistanceCulling::LODLevel>& levels);

    /// @brief Update chunk BVH
    void updateChunkBVH(const std::vector<ChunkBVH::BuildPrimitive>& chunkPrimitives);

    /// @brief Update entity BVH
    void updateEntityBVH(const std::vector<EntityBVH::BuildPrimitive>& entityPrimitives);

    /// @brief Cull everything
    CullingResult cullAll();

    /// @brief Cull chunks only
    CullingResult cullChunks();

    /// @brief Cull entities only
    CullingResult cullEntities();

    /// @brief Enable/disable occlusion culling
    void setOcclusionCulling(bool enabled);

    /// @brief Get last culling result
    const CullingResult& getLastResult() const { return lastResult; }

private:
    Frustum frustum;
    DistanceCulling distanceCulling;
    ChunkBVH chunkBVH;
    EntityBVH entityBVH;
    CullingResult lastResult;
    Vec3 cameraPosition;
    float viewDistance = 1000.0f;
    bool occlusionEnabled = false;

    void cullChunksInternal(CullingResult& result);
    void cullEntitiesInternal(CullingResult& result);
};

// ============================================
// Template Implementation
// ============================================

template<typename TPayload>
void BVHTree<TPayload>::build(const std::vector<BuildPrimitive>& prims) {
    primitives = prims;
    nodes.clear();
    if (primitives.empty()) return;
    
    buildRecursive(0, static_cast<uint32_t>(primitives.size()), 0);
}

template<typename TPayload>
void BVHTree<TPayload>::clear() {
    nodes.clear();
    primitives.clear();
}

template<typename TPayload>
uint32_t BVHTree<TPayload>::buildRecursive(uint32_t start, uint32_t end, uint32_t depth) {
    Node node;
    
    // Compute bounds for this range
    if (start < end) {
        node.bounds = primitives[start].bounds;
        for (uint32_t i = start + 1; i < end; ++i) {
            node.bounds.min.x = std::min(node.bounds.min.x, primitives[i].bounds.min.x);
            node.bounds.min.y = std::min(node.bounds.min.y, primitives[i].bounds.min.y);
            node.bounds.min.z = std::min(node.bounds.min.z, primitives[i].bounds.min.z);
            node.bounds.max.x = std::max(node.bounds.max.x, primitives[i].bounds.max.x);
            node.bounds.max.y = std::max(node.bounds.max.y, primitives[i].bounds.max.y);
            node.bounds.max.z = std::max(node.bounds.max.z, primitives[i].bounds.max.z);
        }
    }
    
    // Make leaf if small enough or max depth
    uint32_t count = end - start;
    if (count <= LEAF_SIZE || depth >= MAX_DEPTH) {
        node.isLeaf = true;
        node.startIndex = start;
        node.count = count;
        uint32_t index = static_cast<uint32_t>(nodes.size());
        nodes.push_back(node);
        return index;
    }
    
    // Find split axis (longest axis of bounds)
    Vec3 extent(
        node.bounds.max.x - node.bounds.min.x,
        node.bounds.max.y - node.bounds.min.y,
        node.bounds.max.z - node.bounds.min.z
    );
    
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent.y && extent.z > extent.x) axis = 2;
    
    // Sort primitives by center on split axis
    std::sort(primitives.begin() + start, primitives.begin() + end,
        [axis](const BuildPrimitive& a, const BuildPrimitive& b) {
            if (axis == 0) return a.center.x < b.center.x;
            if (axis == 1) return a.center.y < b.center.y;
            return a.center.z < b.center.z;
        });
    
    // Split at median
    uint32_t mid = start + count / 2;
    
    uint32_t nodeIndex = static_cast<uint32_t>(nodes.size());
    nodes.push_back(node); // Reserve spot
    
    uint32_t left = buildRecursive(start, mid, depth + 1);
    uint32_t right = buildRecursive(mid, end, depth + 1);
    
    nodes[nodeIndex].leftChild = left;
    nodes[nodeIndex].rightChild = right;
    
    return nodeIndex;
}

template<typename TPayload>
void BVHTree<TPayload>::frustumQuery(const Frustum& frustum, std::vector<TPayload>& results) const {
    results.clear();
    if (nodes.empty()) return;
    frustumQueryRecursive(0, frustum, results);
}

template<typename TPayload>
void BVHTree<TPayload>::frustumQueryRecursive(uint32_t nodeIndex, const Frustum& frustum, std::vector<TPayload>& results) const {
    const Node& node = nodes[nodeIndex];
    
    // Test node bounds against frustum
    if (!frustum.intersects(node.bounds)) {
        return;
    }
    
    if (node.isLeaf) {
        for (uint32_t i = 0; i < node.count; ++i) {
            results.push_back(primitives[node.startIndex + i].payload);
        }
        return;
    }
    
    frustumQueryRecursive(node.leftChild, frustum, results);
    frustumQueryRecursive(node.rightChild, frustum, results);
}

template<typename TPayload>
void BVHTree<TPayload>::aabbQuery(const AABB& queryBounds, std::vector<TPayload>& results) const {
    results.clear();
    if (nodes.empty()) return;
    aabbQueryRecursive(0, queryBounds, results);
}

template<typename TPayload>
void BVHTree<TPayload>::aabbQueryRecursive(uint32_t nodeIndex, const AABB& queryBounds, std::vector<TPayload>& results) const {
    const Node& node = nodes[nodeIndex];
    
    if (!node.bounds.Intersects(queryBounds)) {
        return;
    }
    
    if (node.isLeaf) {
        for (uint32_t i = 0; i < node.count; ++i) {
            results.push_back(primitives[node.startIndex + i].payload);
        }
        return;
    }
    
    aabbQueryRecursive(node.leftChild, queryBounds, results);
    aabbQueryRecursive(node.rightChild, queryBounds, results);
}

template<typename TPayload>
void BVHTree<TPayload>::rayQuery(const Ray& ray, std::vector<TPayload>& results) const {
    std::vector<std::pair<TPayload, float>> sorted;
    rayQuerySorted(ray, sorted);
    results.reserve(sorted.size());
    for (const auto& pair : sorted) {
        results.push_back(pair.first);
    }
}

template<typename TPayload>
void BVHTree<TPayload>::rayQuerySorted(const Ray& ray, std::vector<std::pair<TPayload, float>>& results) const {
    results.clear();
    if (nodes.empty()) return;
    rayQueryRecursive(0, ray, 0.0f, 1e30f, results);
    
    // Sort by distance
    std::sort(results.begin(), results.end(),
        [](const std::pair<TPayload, float>& a, const std::pair<TPayload, float>& b) {
            return a.second < b.second;
        });
}

// Simple ray-AABB intersection
template<typename TPayload>
void BVHTree<TPayload>::rayQueryRecursive(uint32_t nodeIndex, const Ray& ray, float tMin, float tMax, std::vector<std::pair<TPayload, float>>& results) const {
    const Node& node = nodes[nodeIndex];
    
    // Ray-AABB intersection test
    float t1 = tMin, t2 = tMax;
    
    for (int i = 0; i < 3; ++i) {
        float invD = 1.0f / (i == 0 ? ray.direction.x : (i == 1 ? ray.direction.y : ray.direction.z));
        float tNear = ((i == 0 ? node.bounds.min.x : (i == 1 ? node.bounds.min.y : node.bounds.min.z)) - (i == 0 ? ray.origin.x : (i == 1 ? ray.origin.y : ray.origin.z))) * invD;
        float tFar = ((i == 0 ? node.bounds.max.x : (i == 1 ? node.bounds.max.y : node.bounds.max.z)) - (i == 0 ? ray.origin.x : (i == 1 ? ray.origin.y : ray.origin.z))) * invD;
        
        if (invD < 0.0f) std::swap(tNear, tFar);
        
        t1 = std::max(t1, tNear);
        t2 = std::min(t2, tFar);
        
        if (t1 > t2) return; // Miss
    }
    
    if (node.isLeaf) {
        for (uint32_t i = 0; i < node.count; ++i) {
            results.push_back({primitives[node.startIndex + i].payload, t1});
        }
        return;
    }
    
    rayQueryRecursive(node.leftChild, ray, tMin, tMax, results);
    rayQueryRecursive(node.rightChild, ray, tMin, tMax, results);
}

template<typename TPayload>
uint32_t BVHTree<TPayload>::getDepth() const {
    if (nodes.empty()) return 0;
    
    uint32_t maxDepth = 0;
    std::function<void(uint32_t, uint32_t)> traverse = [&](uint32_t idx, uint32_t depth) {
        if (idx >= nodes.size()) return;
        maxDepth = std::max(maxDepth, depth);
        if (!nodes[idx].isLeaf) {
            traverse(nodes[idx].leftChild, depth + 1);
            traverse(nodes[idx].rightChild, depth + 1);
        }
    };
    
    traverse(0, 1);
    return maxDepth;
}

} // namespace vge
