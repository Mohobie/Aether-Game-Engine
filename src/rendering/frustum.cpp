#include "frustum.h"
#include "editor/in_game_editor.h"  // For AABB definition
#include <algorithm>
#include <cmath>

namespace vge {

// ============================================
// Plane
// ============================================

Frustum::Plane::Plane(const Vec3& n, float d) : normal(n), distance(d) {
    float len = normal.length();
    if (len > 0.0001f) {
        normal = normal * (1.0f / len);
        distance *= 1.0f / len;
    }
}

float Frustum::Plane::distanceTo(const Vec3& point) const {
    return normal.dot(point) + distance;
}

bool Frustum::Plane::intersects(const AABB& box) const {
    // Get the positive vertex (furthest in normal direction)
    Vec3 positiveVertex(
        normal.x > 0.0f ? box.max.x : box.min.x,
        normal.y > 0.0f ? box.max.y : box.min.y,
        normal.z > 0.0f ? box.max.z : box.min.z
    );
    return distanceTo(positiveVertex) >= 0.0f;
}

// ============================================
// Frustum
// ============================================

void Frustum::update(const Mat4& viewProj) {
    const float* m = viewProj.data;

    // For column-major matrices (OpenGL style):
    // Planes are rows of the inverse-transpose, but we can extract them
    // directly from the view-projection matrix for world-space testing.
    //
    // For a point to be inside the frustum, it must be inside all 6 planes.
    // Each plane is defined as: ax + by + cz + d = 0
    // where (a,b,c) is the normal and d is the distance.
    //
    // We extract planes from the view-projection matrix:
    // Left:   row3 + row0  (clip space: -w <= x)
    // Right:  row3 - row0  (clip space: x <= w)
    // Bottom: row3 + row1  (clip space: -w <= y)
    // Top:    row3 - row1  (clip space: y <= w)
    // Near:   row3 + row2  (clip space: -w <= z)
    // Far:    row3 - row2  (clip space: z <= w)
    //
    // For row-major memory layout (data[i*4+j] = row i, col j):
    // Row 0: m[0], m[1], m[2], m[3]
    // Row 1: m[4], m[5], m[6], m[7]
    // Row 2: m[8], m[9], m[10], m[11]
    // Row 3: m[12], m[13], m[14], m[15]

    // Left plane
    planes[Left].normal.x = m[3] + m[0];
    planes[Left].normal.y = m[7] + m[4];
    planes[Left].normal.z = m[11] + m[8];
    planes[Left].distance = m[15] + m[12];

    // Right plane
    planes[Right].normal.x = m[3] - m[0];
    planes[Right].normal.y = m[7] - m[4];
    planes[Right].normal.z = m[11] - m[8];
    planes[Right].distance = m[15] - m[12];

    // Bottom plane
    planes[Bottom].normal.x = m[3] + m[1];
    planes[Bottom].normal.y = m[7] + m[5];
    planes[Bottom].normal.z = m[11] + m[9];
    planes[Bottom].distance = m[15] + m[13];

    // Top plane
    planes[Top].normal.x = m[3] - m[1];
    planes[Top].normal.y = m[7] - m[5];
    planes[Top].normal.z = m[11] - m[9];
    planes[Top].distance = m[15] - m[13];

    // Near plane
    planes[Near].normal.x = m[3] + m[2];
    planes[Near].normal.y = m[7] + m[6];
    planes[Near].normal.z = m[11] + m[10];
    planes[Near].distance = m[15] + m[14];

    // Far plane
    planes[Far].normal.x = m[3] - m[2];
    planes[Far].normal.y = m[7] - m[6];
    planes[Far].normal.z = m[11] - m[10];
    planes[Far].distance = m[15] - m[14];

    // Normalize all planes
    for (auto& plane : planes) {
        float len = plane.normal.length();
        if (len > 0.0001f) {
            plane.normal = plane.normal * (1.0f / len);
            plane.distance *= 1.0f / len;
        }
    }
}

bool Frustum::contains(const Vec3& point) const {
    for (const auto& plane : planes) {
        if (plane.distanceTo(point) < 0.0f) {
            return false;
        }
    }
    return true;
}

bool Frustum::intersects(const AABB& box) const {
    for (const auto& plane : planes) {
        if (!plane.intersects(box)) {
            return false;
        }
    }
    return true;
}

bool Frustum::intersects(const Vec3& center, float radius) const {
    for (const auto& plane : planes) {
        if (plane.distanceTo(center) < -radius) {
            return false;
        }
    }
    return true;
}

Frustum::TestResult Frustum::testAABB(const AABB& box) const {
    int insideCount = 0;

    for (const auto& plane : planes) {
        // Get positive vertex (furthest in normal direction)
        Vec3 positiveVertex(
            plane.normal.x > 0.0f ? box.max.x : box.min.x,
            plane.normal.y > 0.0f ? box.max.y : box.min.y,
            plane.normal.z > 0.0f ? box.max.z : box.min.z
        );

        // Get negative vertex (closest to plane)
        Vec3 negativeVertex(
            plane.normal.x > 0.0f ? box.min.x : box.max.x,
            plane.normal.y > 0.0f ? box.min.y : box.max.y,
            plane.normal.z > 0.0f ? box.min.z : box.max.z
        );

        float positiveDist = plane.distanceTo(positiveVertex);
        float negativeDist = plane.distanceTo(negativeVertex);

        if (positiveDist < 0.0f) {
            // All corners outside this plane
            return TestResult::Outside;
        }

        if (negativeDist >= 0.0f) {
            // All corners inside this plane
            insideCount++;
        }
    }

    // If all 6 planes have all corners inside, AABB is fully inside
    return (insideCount == 6) ? TestResult::Inside : TestResult::Intersect;
}

std::array<Vec3, 8> Frustum::getCorners() const {
    std::array<Vec3, 8> corners;
    
    // Helper to intersect 3 planes
    auto intersectPlanes = [](const Plane& p1, const Plane& p2, const Plane& p3) -> Vec3 {
        Vec3 n1 = p1.normal, n2 = p2.normal, n3 = p3.normal;
        float d1 = p1.distance, d2 = p2.distance, d3 = p3.distance;

        Vec3 n2xn3 = n2.cross(n3);
        float denom = n1.dot(n2xn3);
        if (std::abs(denom) < 0.0001f) return Vec3();

        Vec3 term1 = n2.cross(n3) * (-d1);
        Vec3 term2 = n3.cross(n1) * (-d2);
        Vec3 term3 = n1.cross(n2) * (-d3);

        return (term1 + term2 + term3) * (1.0f / denom);
    };

    // Near corners
    corners[0] = intersectPlanes(planes[Near], planes[Left],  planes[Bottom]);
    corners[1] = intersectPlanes(planes[Near], planes[Right], planes[Bottom]);
    corners[2] = intersectPlanes(planes[Near], planes[Left],  planes[Top]);
    corners[3] = intersectPlanes(planes[Near], planes[Right], planes[Top]);

    // Far corners
    corners[4] = intersectPlanes(planes[Far], planes[Left],  planes[Bottom]);
    corners[5] = intersectPlanes(planes[Far], planes[Right], planes[Bottom]);
    corners[6] = intersectPlanes(planes[Far], planes[Left],  planes[Top]);
    corners[7] = intersectPlanes(planes[Far], planes[Right], planes[Top]);

    return corners;
}

// ============================================
// Distance Culling
// ============================================

void DistanceCulling::setupChunkLODs(int chunkSize) {
    levels.clear();
    // LOD 0: 0-2 chunks (full detail)
    levels.push_back({2.0f * chunkSize, 0.0f, 0});
    // LOD 1: 2-4 chunks
    levels.push_back({4.0f * chunkSize, 1.5f * chunkSize, 1});
    // LOD 2: 4-8 chunks
    levels.push_back({8.0f * chunkSize, 3.0f * chunkSize, 2});
    // LOD 3: 8-16 chunks
    levels.push_back({16.0f * chunkSize, 6.0f * chunkSize, 3});
    // LOD 4: 16-32 chunks (lowest detail)
    levels.push_back({32.0f * chunkSize, 12.0f * chunkSize, 4});
}

int DistanceCulling::getLODLevel(const Vec3& center, float radius) const {
    float dist = (center - cameraPosition).length() - radius;

    for (size_t i = 0; i < levels.size(); ++i) {
        if (dist <= levels[i].maxDistance) {
            return levels[i].lod;
        }
    }

    return -1; // Beyond max distance, culled
}

bool DistanceCulling::isVisible(const Vec3& center, float radius) const {
    if (levels.empty()) return true;

    float dist = (center - cameraPosition).length() - radius;
    return dist <= levels.back().maxDistance;
}

} // namespace vge