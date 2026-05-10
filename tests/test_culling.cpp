#include "render/frustum.h"
#include "render/bvh.h"
#include <cassert>
#include <iostream>

using namespace aether;

// --- Test Helpers ---

static bool approxEqual(float a, float b, float eps = 0.0001f) {
    return std::abs(a - b) < eps;
}

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name) void test_##name()
#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { std::cerr << "FAIL: " << #cond << " at " << __LINE__ << "\n"; g_failed++; } else { g_passed++; } } while(0)
#define ASSERT_EQ(a, b) \
    do { if ((a) != (b)) { std::cerr << "FAIL: " << #a << " == " << #b << " at " << __LINE__ << " (" << (a) << " != " << (b) << ")\n"; g_failed++; } else { g_passed++; } } while(0)
#define ASSERT_NEAR(a, b, eps) \
    do { if (!approxEqual(a, b, eps)) { std::cerr << "FAIL: " << #a << " ~= " << #b << " at " << __LINE__ << "\n"; g_failed++; } else { g_passed++; } } while(0)

// --- Math Tests ---

TEST(vec3_basic) {
    Vec3 a(1, 2, 3);
    Vec3 b(4, 5, 6);
    Vec3 c = a + b;
    ASSERT_EQ(c.x, 5.0f);
    ASSERT_EQ(c.y, 7.0f);
    ASSERT_EQ(c.z, 9.0f);

    Vec3 d = b - a;
    ASSERT_EQ(d.x, 3.0f);
    ASSERT_EQ(d.y, 3.0f);
    ASSERT_EQ(d.z, 3.0f);

    Vec3 e = a * 2.0f;
    ASSERT_EQ(e.x, 2.0f);
    ASSERT_EQ(e.y, 4.0f);
    ASSERT_EQ(e.z, 6.0f);

    ASSERT_NEAR(a.length(), std::sqrt(14.0f), 0.0001f);
    ASSERT_NEAR(a.normalized().length(), 1.0f, 0.0001f);
}

TEST(vec3_dot_cross) {
    Vec3 a(1, 0, 0);
    Vec3 b(0, 1, 0);
    ASSERT_NEAR(a.dot(b), 0.0f, 0.0001f);

    Vec3 c = a.cross(b);
    ASSERT_NEAR(c.x, 0.0f, 0.0001f);
    ASSERT_NEAR(c.y, 0.0f, 0.0001f);
    ASSERT_NEAR(c.z, 1.0f, 0.0001f);
}

TEST(mat4_identity) {
    Mat4 m = Mat4::identity();
    Vec3 p(1, 2, 3);
    Vec3 r = m.transformPoint(p);
    ASSERT_NEAR(r.x, 1.0f, 0.0001f);
    ASSERT_NEAR(r.y, 2.0f, 0.0001f);
    ASSERT_NEAR(r.z, 3.0f, 0.0001f);
}

TEST(mat4_translate) {
    Mat4 m = Mat4::translate(Vec3(10, 20, 30));
    Vec3 p(1, 2, 3);
    Vec3 r = m.transformPoint(p);
    ASSERT_NEAR(r.x, 11.0f, 0.0001f);
    ASSERT_NEAR(r.y, 22.0f, 0.0001f);
    ASSERT_NEAR(r.z, 33.0f, 0.0001f);
}

TEST(mat4_lookat) {
    Vec3 eye(0, 0, 5);
    Vec3 center(0, 0, 0);
    Vec3 up(0, 1, 0);
    Mat4 view = Mat4::lookAt(eye, center, up);

    // A point at origin in view space should be along -Z
    Vec3 p(0, 0, 0);
    Vec3 v = view.transformPoint(p);
    ASSERT_NEAR(v.z, -5.0f, 0.0001f);
}

TEST(quat_rotation) {
    Quat q = Quat::fromAxisAngle(Vec3(0, 0, 1), 3.1415926535f / 2.0f); // 90 degrees
    Vec3 v(1, 0, 0);
    Vec3 r = q * v;
    ASSERT_NEAR(r.x, 0.0f, 0.001f);
    ASSERT_NEAR(r.y, 1.0f, 0.001f);
    ASSERT_NEAR(r.z, 0.0f, 0.001f);
}

// --- AABB Tests ---

TEST(aabb_basic) {
    AABB box(Vec3(0, 0, 0), Vec3(10, 10, 10));
    ASSERT_TRUE(box.contains(Vec3(5, 5, 5)));
    ASSERT_TRUE(!box.contains(Vec3(15, 5, 5)));

    AABB box2(Vec3(5, 5, 5), Vec3(15, 15, 15));
    ASSERT_TRUE(box.intersects(box2));

    AABB box3(Vec3(20, 20, 20), Vec3(30, 30, 30));
    ASSERT_TRUE(!box.intersects(box3));
}

TEST(aabb_merge) {
    AABB a(Vec3(0, 0, 0), Vec3(5, 5, 5));
    AABB b(Vec3(3, 3, 3), Vec3(8, 8, 8));
    AABB c = a.merge(b);
    ASSERT_NEAR(c.min.x, 0.0f, 0.0001f);
    ASSERT_NEAR(c.max.x, 8.0f, 0.0001f);
}

TEST(aabb_ray_intersect) {
    AABB box(Vec3(-1, -1, -1), Vec3(1, 1, 1));
    Ray ray(Vec3(0, 0, -5), Vec3(0, 0, 1));
    float tmin, tmax;
    ASSERT_TRUE(box.intersectRay(ray.origin, ray.direction, tmin, tmax));
    ASSERT_NEAR(tmin, 4.0f, 0.0001f);
    ASSERT_NEAR(tmax, 6.0f, 0.0001f);
}

// --- Frustum Tests ---

TEST(frustum_extraction) {
    // Create a simple perspective frustum
    float fov = 3.1415926535f / 4.0f; // 45 degrees
    float aspect = 16.0f / 9.0f;
    Mat4 proj = Mat4::perspective(fov, aspect, 0.1f, 100.0f);
    Mat4 view = Mat4::lookAt(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
    Mat4 viewProj = proj * view;

    Frustum frustum;
    frustum.update(viewProj);

    // Point in front of camera should be inside
    ASSERT_TRUE(frustum.contains(Vec3(0, 0, 2)));

    // Point behind camera should be outside
    ASSERT_TRUE(!frustum.contains(Vec3(0, 0, 10)));

    // Point far to the side should be outside
    ASSERT_TRUE(!frustum.contains(Vec3(100, 0, 2)));
}

TEST(frustum_aabb_test) {
    float fov = 3.1415926535f / 4.0f;
    Mat4 proj = Mat4::perspective(fov, 16.0f/9.0f, 0.1f, 100.0f);
    Mat4 view = Mat4::lookAt(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
    Mat4 viewProj = proj * view;

    Frustum frustum;
    frustum.update(viewProj);

    // AABB at origin, visible
    AABB visibleBox(Vec3(-1, -1, -1), Vec3(1, 1, 1));
    ASSERT_TRUE(frustum.intersects(visibleBox));

    // AABB behind camera, not visible
    AABB hiddenBox(Vec3(-1, -1, 10), Vec3(1, 1, 12));
    ASSERT_TRUE(!frustum.intersects(hiddenBox));
}

TEST(frustum_sphere_test) {
    float fov = 3.1415926535f / 4.0f;
    Mat4 proj = Mat4::perspective(fov, 16.0f/9.0f, 0.1f, 100.0f);
    Mat4 view = Mat4::lookAt(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
    Mat4 viewProj = proj * view;

    Frustum frustum;
    frustum.update(viewProj);

    // Sphere at origin, visible
    ASSERT_TRUE(frustum.intersects(Vec3(0, 0, 0), 1.0f));

    // Sphere behind camera, not visible
    ASSERT_TRUE(!frustum.intersects(Vec3(0, 0, 10), 1.0f));
}

TEST(frustum_test_result) {
    float fov = 3.1415926535f / 4.0f;
    Mat4 proj = Mat4::perspective(fov, 16.0f/9.0f, 0.1f, 100.0f);
    Mat4 view = Mat4::lookAt(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
    Mat4 viewProj = proj * view;

    Frustum frustum;
    frustum.update(viewProj);

    // Small box fully inside
    AABB inside(Vec3(-0.1f, -0.1f, -0.1f), Vec3(0.1f, 0.1f, 0.1f));
    auto result = frustum.testAABB(inside);
    ASSERT_TRUE(result == Frustum::TestResult::Inside || result == Frustum::TestResult::Intersect);

    // Large box straddling frustum
    AABB straddle(Vec3(-50, -50, -50), Vec3(50, 50, 50));
    result = frustum.testAABB(straddle);
    ASSERT_TRUE(result == Frustum::TestResult::Intersect);

    // Box fully outside
    AABB outside(Vec3(-100, 0, 0), Vec3(-90, 10, 10));
    result = frustum.testAABB(outside);
    ASSERT_TRUE(result == Frustum::TestResult::Outside);
}

// --- BVH Tests ---

TEST(bvh_build) {
    std::vector<AABB> bounds;
    std::vector<uint32_t> payloads;

    for (int i = 0; i < 100; ++i) {
        float x = static_cast<float>(i);
        bounds.push_back(AABB(Vec3(x, 0, 0), Vec3(x + 1, 1, 1)));
        payloads.push_back(i);
    }

    ChunkBVH bvh;
    bvh.build(bounds, payloads);

    ASSERT_TRUE(bvh.nodeCount() > 0);
    ASSERT_TRUE(bvh.getDepth() > 0);
    ASSERT_TRUE(bvh.getRootBounds() != nullptr);
}

TEST(bvh_frustum_query) {
    std::vector<ChunkBVH::BuildPrimitive> prims;

    // Create 10x10 grid of chunks centered around origin
    for (int x = -5; x < 5; ++x) {
        for (int z = -5; z < 5; ++z) {
            ChunkBVH::BuildPrimitive p;
            p.bounds = AABB(Vec3(x * 32.0f, 0, z * 32.0f), Vec3((x + 1) * 32.0f, 32.0f, (z + 1) * 32.0f));
            p.payload = (x + 5) * 10 + (z + 5);
            p.center = p.bounds.center();
            prims.push_back(p);
        }
    }

    ChunkBVH bvh;
    bvh.build(prims);

    // Create frustum looking at origin from (0, 80, 160)
    float fov = 3.1415926535f / 3.0f; // 60 degrees
    Mat4 proj = Mat4::perspective(fov, 1.0f, 0.1f, 500.0f);
    Mat4 view = Mat4::lookAt(Vec3(0, 80, 160), Vec3(0, 0, 0), Vec3(0, 1, 0));
    Mat4 viewProj = proj * view;

    Frustum frustum;
    frustum.update(viewProj);

    std::vector<uint32_t> results;
    bvh.frustumQuery(frustum, results);

    // Should find some chunks but not all 100
    ASSERT_TRUE(results.size() > 0);
    ASSERT_TRUE(results.size() < 100);
}

TEST(bvh_aabb_query) {
    std::vector<ChunkBVH::BuildPrimitive> prims;

    for (int i = 0; i < 50; ++i) {
        ChunkBVH::BuildPrimitive p;
        p.bounds = AABB(Vec3(i * 2.0f, 0, 0), Vec3(i * 2.0f + 1.0f, 1.0f, 1.0f));
        p.payload = i;
        p.center = p.bounds.center();
        prims.push_back(p);
    }

    ChunkBVH bvh;
    bvh.build(prims);

    // Query AABB that should overlap primitives 10-20
    AABB query(Vec3(15.0f, -1, -1), Vec3(35.0f, 2, 2));
    std::vector<uint32_t> results;
    bvh.aabbQuery(query, results);

    ASSERT_TRUE(results.size() > 0);
    // Should find roughly primitives in range 7-17
    for (uint32_t id : results) {
        ASSERT_TRUE(id >= 7 && id <= 18);
    }
}

TEST(bvh_ray_query) {
    std::vector<ChunkBVH::BuildPrimitive> prims;

    for (int i = 0; i < 50; ++i) {
        ChunkBVH::BuildPrimitive p;
        p.bounds = AABB(Vec3(0, 0, i * 2.0f), Vec3(1.0f, 1.0f, i * 2.0f + 1.0f));
        p.payload = i;
        p.center = p.bounds.center();
        prims.push_back(p);
    }

    ChunkBVH bvh;
    bvh.build(prims);

    // Ray along Z axis
    Ray ray(Vec3(0.5f, 0.5f, -10.0f), Vec3(0, 0, 1));
    std::vector<std::pair<uint32_t, float>> sortedResults;
    bvh.rayQuerySorted(ray, sortedResults);

    // Should hit all boxes in order
    ASSERT_TRUE(sortedResults.size() > 0);
    // Results should be ordered by hit distance
    for (size_t i = 1; i < sortedResults.size(); ++i) {
        ASSERT_TRUE(sortedResults[i].second >= sortedResults[i-1].second);
    }
}

// --- Distance Culling Tests ---

TEST(distance_lod_setup) {
    DistanceCulling dc;
    dc.setupChunkLODs(32);

    ASSERT_TRUE(dc.levels.size() > 0);
    ASSERT_TRUE(dc.levels[0].lod == 0);
}

TEST(distance_lod_levels) {
    DistanceCulling dc;
    dc.setupChunkLODs(32);
    dc.setCameraPosition(Vec3(0, 0, 0));

    // Close chunk -> LOD 0
    int lod = dc.getLODLevel(Vec3(16, 16, 16), 16.0f);
    ASSERT_TRUE(lod == 0);

    // Far chunk -> higher LOD or culled
    int lodFar = dc.getLODLevel(Vec3(1000, 0, 0), 16.0f);
    ASSERT_TRUE(lodFar == -1 || lodFar > 0);
}

TEST(distance_visibility) {
    DistanceCulling dc;
    dc.setupChunkLODs(32);
    dc.setCameraPosition(Vec3(0, 0, 0));

    ASSERT_TRUE(dc.isVisible(Vec3(16, 16, 16), 16.0f));
    ASSERT_TRUE(!dc.isVisible(Vec3(2000, 0, 0), 16.0f));
}

// --- CullingSystem Tests ---

TEST(cullingsystem_basic) {
    CullingSystem cs;

    float fov = 3.1415926535f / 3.0f;
    Mat4 proj = Mat4::perspective(fov, 1.0f, 0.1f, 500.0f);
    Mat4 view = Mat4::lookAt(Vec3(0, 80, 160), Vec3(0, 0, 0), Vec3(0, 1, 0));

    cs.setCamera(Vec3(0, 80, 160), proj * view);
    cs.setViewDistance(512.0f);

    std::vector<ChunkBVH::BuildPrimitive> chunks;
    for (int x = -3; x < 3; ++x) {
        for (int z = -3; z < 3; ++z) {
            ChunkBVH::BuildPrimitive p;
            p.bounds = AABB(Vec3(x * 32.0f, 0, z * 32.0f), Vec3((x+1) * 32.0f, 32.0f, (z+1) * 32.0f));
            p.payload = (x + 3) * 6 + (z + 3);
            p.center = p.bounds.center();
            chunks.push_back(p);
        }
    }

    cs.updateChunkBVH(chunks);

    CullingResult result = cs.cullChunks();

    ASSERT_TRUE(result.visibleChunks.size() > 0);
    ASSERT_TRUE(result.visibleChunks.size() <= 36);
}

TEST(cullingsystem_stats) {
    CullingSystem cs;

    float fov = 3.1415926535f / 3.0f;
    Mat4 proj = Mat4::perspective(fov, 1.0f, 0.1f, 500.0f);
    Mat4 view = Mat4::lookAt(Vec3(0, 80, 160), Vec3(0, 0, 0), Vec3(0, 1, 0));

    cs.setCamera(Vec3(0, 80, 160), proj * view);

    std::vector<ChunkBVH::BuildPrimitive> chunks;
    for (int x = -5; x < 5; ++x) {
        for (int z = -5; z < 5; ++z) {
            ChunkBVH::BuildPrimitive p;
            p.bounds = AABB(Vec3(x * 32.0f, 0, z * 32.0f), Vec3((x+1) * 32.0f, 32.0f, (z+1) * 32.0f));
            p.payload = (x + 5) * 10 + (z + 5);
            p.center = p.bounds.center();
            chunks.push_back(p);
        }
    }

    cs.updateChunkBVH(chunks);

    CullingResult result = cs.cullChunks();

    ASSERT_TRUE(result.totalChunksTested > 0);
    ASSERT_TRUE(result.chunksCulledFrustum >= 0);
}

// --- Main ---

int main() {
    std::cout << "Running culling system tests...\n\n";

    test_vec3_basic();
    test_vec3_dot_cross();
    test_mat4_identity();
    test_mat4_translate();
    test_mat4_lookat();
    test_quat_rotation();
    test_aabb_basic();
    test_aabb_merge();
    test_aabb_ray_intersect();
    test_frustum_extraction();
    test_frustum_aabb_test();
    test_frustum_sphere_test();
    test_frustum_test_result();
    test_bvh_build();
    test_bvh_frustum_query();
    test_bvh_aabb_query();
    test_bvh_ray_query();
    test_distance_lod_setup();
    test_distance_lod_levels();
    test_distance_visibility();
    test_cullingsystem_basic();
    test_cullingsystem_stats();

    std::cout << "\n========================================\n";
    std::cout << "Results: " << g_passed << " passed, " << g_failed << " failed\n";
    std::cout << "========================================\n";

    return g_failed > 0 ? 1 : 0;
}
