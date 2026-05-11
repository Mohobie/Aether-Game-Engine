#pragma once

#include "math/vec3.h"
#include "math/mat4.h"
#include <array>
#include <vector>

namespace vge {

// Forward declaration - AABB is defined in editor/in_game_editor.h
// We'll include it in the cpp file to avoid circular includes
struct AABB;

/**
 * @brief Frustum defined by 6 clipping planes
 * Extracted from view-projection matrix for visibility testing.
 */
struct Frustum {
    /// @brief A clipping plane: normal + distance from origin
    struct Plane {
        Vec3 normal;
        float distance;

        Plane() : normal(0, 0, 0), distance(0) {}
        Plane(const Vec3& n, float d);

        /// Signed distance from point to plane (>0 = in front)
        float distanceTo(const Vec3& point) const;

        /// Fast AABB test: check positive vertex against plane
        bool intersects(const AABB& box) const;
    };

    std::array<Plane, 6> planes;

    enum PlaneIndex {
        Left = 0, Right = 1,
        Bottom = 2, Top = 3,
        Near = 4, Far = 5
    };

    /// @brief Extract frustum planes from view-projection matrix
    void update(const Mat4& viewProj);

    /// @brief Test if point is inside frustum
    bool contains(const Vec3& point) const;

    /// @brief Test if AABB intersects or is inside frustum
    bool intersects(const AABB& box) const;

    /// @brief Test if sphere intersects or is inside frustum
    bool intersects(const Vec3& center, float radius) const;

    /// @brief Detailed test result for AABB
    enum class TestResult { Inside, Intersect, Outside };
    TestResult testAABB(const AABB& box) const;

    /// @brief Get corner points of the frustum (for debug visualization)
    std::array<Vec3, 8> getCorners() const;
};

/**
 * @brief Distance-based LOD culling
 */
struct DistanceCulling {
    /// @brief LOD level configuration
    struct LODLevel {
        float maxDistance;  ///< Maximum distance for this LOD
        float minDistance;  ///< Minimum distance (for hysteresis)
        int lod;            ///< LOD index (0 = full detail)
    };

    std::vector<LODLevel> levels;
    Vec3 cameraPosition;

    void setCameraPosition(const Vec3& pos) { cameraPosition = pos; }

    /// @brief Configure standard chunk LOD levels
    void setupChunkLODs(int chunkSize);

    /// @brief Get LOD level for a point/sphere
    /// @return LOD index, or -1 if beyond max distance (culled)
    int getLODLevel(const Vec3& center, float radius) const;

    /// @brief Check if object is within any LOD distance
    bool isVisible(const Vec3& center, float radius) const;
};

/**
 * @brief Culling result containing visible objects
 */
struct CullingResult {
    std::vector<void*> visibleChunks;      ///< Chunk pointers
    std::vector<void*> visibleEntities;    ///< Entity pointers
    std::vector<int> chunkLODLevels;       ///< Per-chunk LOD
    std::vector<int> entityLODLevels;      ///< Per-entity LOD
    int totalChunksTested = 0;
    int totalEntitiesTested = 0;
    int chunksCulledFrustum = 0;
    int chunksCulledDistance = 0;
    int entitiesCulledFrustum = 0;
    int entitiesCulledDistance = 0;
};

} // namespace vge