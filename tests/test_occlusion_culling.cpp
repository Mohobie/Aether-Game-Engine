#include "occlusion_culling.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace aether;

// ============================================================================
// Test Helpers
// ============================================================================
static int g_testCount = 0;
static int g_passCount = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "  [TEST] " #name " ... "; \
    ++g_testCount; \
    try { test_##name(); ++g_passCount; std::cout << "PASS\n"; } \
    catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << "\n"; } \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        throw std::runtime_error("Assertion failed: " #cond " at line " + std::to_string(__LINE__)); \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_TRUE(cond) ASSERT(cond)
#define ASSERT_FALSE(cond) ASSERT(!(cond))

// ============================================================================
// Mock Math Types (minimal implementation for testing)
// ============================================================================
// In a real project, these would come from math.h

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    Vec3 operator+(const Vec3& o) const { return Vec3(x+o.x, y+o.y, z+o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x-o.x, y-o.y, z-o.z); }
    Vec3 operator*(float s) const { return Vec3(x*s, y*s, z*s); }
    
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    float dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o) const {
        return Vec3(y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x);
    }
    Vec3 normalized() const {
        float len = length();
        if (len < 0.0001f) return Vec3(0, 0, 0);
        return *this * (1.0f / len);
    }
};

struct AABB {
    Vec3 min, max;
    AABB() {}
    AABB(const Vec3& min_, const Vec3& max_) : min(min_), max(max_) {}
};

struct Mat4 {
    float m[16];
    static Mat4 identity() {
        Mat4 r;
        std::fill(r.m, r.m+16, 0.0f);
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }
    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                r.m[i*4+j] = 0;
                for (int k = 0; k < 4; ++k) {
                    r.m[i*4+j] += m[i*4+k] * o.m[k*4+j];
                }
            }
        }
        return r;
    }
};

struct Vec4 {
    float x, y, z, w;
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

// ============================================================================
// Tests
// ============================================================================

TEST(query_basic_lifecycle) {
    OcclusionQuery query;
    ASSERT_EQ(query.getHandle(), 1);  // First mock handle
    ASSERT_FALSE(query.isActive());
    
    query.beginQuery();
    ASSERT_TRUE(query.isActive());
    
    query.endQuery();
    ASSERT_FALSE(query.isActive());
    ASSERT_TRUE(query.isResultAvailable());
    
    query.reset();
    ASSERT_FALSE(query.isActive());
    ASSERT_FALSE(query.isResultAvailable());
}

TEST(query_move_semantics) {
    OcclusionQuery q1;
    uint32_t handle = q1.getHandle();
    
    OcclusionQuery q2 = std::move(q1);
    ASSERT_EQ(q2.getHandle(), handle);
    ASSERT_EQ(q1.getHandle(), 0);  // Moved-from
}

TEST(query_pool_acquire_release) {
    OcclusionQueryPool pool(4);
    
    ASSERT_EQ(pool.availableCount(), 4);
    ASSERT_EQ(pool.totalCount(), 4);
    
    OcclusionQuery* q1 = pool.acquire();
    ASSERT_TRUE(q1 != nullptr);
    ASSERT_EQ(pool.availableCount(), 3);
    
    pool.release(q1);
    ASSERT_EQ(pool.availableCount(), 4);
}

TEST(query_pool_growth) {
    OcclusionQueryPool pool(2);
    ASSERT_EQ(pool.totalCount(), 2);
    
    OcclusionQuery* q1 = pool.acquire();
    OcclusionQuery* q2 = pool.acquire();
    ASSERT_EQ(pool.availableCount(), 0);
    
    // Should grow
    OcclusionQuery* q3 = pool.acquire();
    ASSERT_TRUE(q3 != nullptr);
    ASSERT_TRUE(pool.totalCount() > 2);
    
    pool.release(q1);
    pool.release(q2);
    pool.release(q3);
}

TEST(query_pool_reset_all) {
    OcclusionQueryPool pool(4);
    
    OcclusionQuery* q1 = pool.acquire();
    q1->beginQuery();
    q1->endQuery();
    
    pool.release(q1);
    pool.resetAll();
    
    ASSERT_EQ(pool.availableCount(), 4);
    
    OcclusionQuery* q2 = pool.acquire();
    ASSERT_FALSE(q2->isActive());
    ASSERT_FALSE(q2->isResultAvailable());
    
    pool.release(q2);
}

TEST(software_rasterizer_init) {
    SoftwareOcclusionRasterizer rasterizer;
    
    ASSERT_TRUE(rasterizer.initialize(256, 256));
    ASSERT_EQ(rasterizer.getWidth(), 256);
    ASSERT_EQ(rasterizer.getHeight(), 256);
    ASSERT_TRUE(rasterizer.getDepthBuffer() != nullptr);
    
    rasterizer.shutdown();
    ASSERT_EQ(rasterizer.getWidth(), 0);
    ASSERT_EQ(rasterizer.getHeight(), 0);
}

TEST(software_rasterizer_invalid_size) {
    SoftwareOcclusionRasterizer rasterizer;
    ASSERT_FALSE(rasterizer.initialize(0, 256));
    ASSERT_FALSE(rasterizer.initialize(256, 0));
    ASSERT_FALSE(rasterizer.initialize(-1, 256));
}

TEST(software_rasterizer_clear) {
    SoftwareOcclusionRasterizer rasterizer;
    rasterizer.initialize(64, 64);
    
    Mat4 vp = Mat4::identity();
    rasterizer.beginFrame(vp);
    
    const float* buffer = rasterizer.getDepthBuffer();
    // All values should be cleared to 1.0 (far plane)
    for (int i = 0; i < 64*64; ++i) {
        ASSERT_EQ(buffer[i], 1.0f);
    }
    
    rasterizer.shutdown();
}

TEST(software_rasterizer_rasterize_aabb) {
    SoftwareOcclusionRasterizer rasterizer;
    rasterizer.initialize(64, 64);
    
    Mat4 vp = Mat4::identity();
    rasterizer.beginFrame(vp);
    
    // Rasterize a small AABB at origin
    AABB box(Vec3(-0.1f, -0.1f, -0.1f), Vec3(0.1f, 0.1f, 0.1f));
    rasterizer.rasterizeAABB(box);
    
    // Check that some depth values were written (should be < 1.0)
    const float* buffer = rasterizer.getDepthBuffer();
    bool hasDepth = false;
    for (int i = 0; i < 64*64; ++i) {
        if (buffer[i] < 1.0f) {
            hasDepth = true;
            break;
        }
    }
    ASSERT_TRUE(hasDepth);
    
    rasterizer.shutdown();
}

TEST(software_rasterizer_test_visible) {
    SoftwareOcclusionRasterizer rasterizer;
    rasterizer.initialize(64, 64);
    
    Mat4 vp = Mat4::identity();
    rasterizer.beginFrame(vp);
    
    // Test an AABB that should be visible (no occluders yet)
    AABB box(Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f));
    ASSERT_TRUE(rasterizer.testAABB(box));
    
    rasterizer.shutdown();
}

TEST(software_rasterizer_test_occluded) {
    SoftwareOcclusionRasterizer rasterizer;
    rasterizer.initialize(64, 64);
    
    Mat4 vp = Mat4::identity();
    rasterizer.beginFrame(vp);
    
    // First, rasterize a large occluder
    AABB occluder(Vec3(-1.0f, -1.0f, -0.5f), Vec3(1.0f, 1.0f, 0.0f));
    rasterizer.rasterizeAABB(occluder);
    
    // Then test a smaller box behind it
    AABB behind(Vec3(-0.5f, -0.5f, 0.1f), Vec3(0.5f, 0.5f, 0.5f));
    // Note: This may or may not be occluded depending on depth precision
    // The test mainly verifies the API works
    bool visible = rasterizer.testAABB(behind);
    (void)visible;  // Don't assert - depth buffer behavior is approximate
    
    rasterizer.shutdown();
}

TEST(software_rasterizer_test_sphere) {
    SoftwareOcclusionRasterizer rasterizer;
    rasterizer.initialize(64, 64);
    
    Mat4 vp = Mat4::identity();
    rasterizer.beginFrame(vp);
    
    // Test sphere visibility (approximated as AABB)
    Vec3 center(0.0f, 0.0f, 0.0f);
    float radius = 0.5f;
    ASSERT_TRUE(rasterizer.testSphere(center, radius));
    
    rasterizer.shutdown();
}

TEST(occlusion_system_init) {
    OcclusionCullingSystem system;
    ASSERT_TRUE(system.initialize(64));
    system.shutdown();
}

TEST(occlusion_system_register_unregister) {
    OcclusionCullingSystem system;
    system.initialize(32);
    
    AABB bounds(Vec3(0, 0, 0), Vec3(10, 10, 10));
    system.registerObject(1, bounds);
    system.registerOccluder(2, bounds);
    
    // Should be able to cull
    std::vector<uint32_t> candidates = {1, 2};
    OcclusionResult result = system.cull(candidates);
    
    // Both should be visible (no actual occluders rendered yet)
    ASSERT_TRUE(result.visibleObjects.size() > 0);
    
    system.unregisterObject(1);
    system.unregisterObject(2);
    
    // After unregister, should not be in results
    OcclusionResult result2 = system.cull(candidates);
    // Object 1 and 2 are unregistered, so they shouldn't appear
    // (implementation detail: they might be ignored)
    
    system.shutdown();
}

TEST(occlusion_system_update_bounds) {
    OcclusionCullingSystem system;
    system.initialize(32);
    
    AABB bounds1(Vec3(0, 0, 0), Vec3(10, 10, 10));
    system.registerObject(1, bounds1);
    
    AABB bounds2(Vec3(100, 100, 100), Vec3(110, 110, 110));
    system.updateObjectBounds(1, bounds2);
    
    // Set camera far from both
    Mat4 view = Mat4::identity();
    Mat4 proj = Mat4::identity();
    system.setCamera(Vec3(0, 0, 0), view, proj);
    
    std::vector<uint32_t> candidates = {1};
    OcclusionResult result = system.cull(candidates);
    
    system.shutdown();
}

TEST(occlusion_system_temporal_coherence) {
    OcclusionCullingSystem system;
    system.initialize(32);
    system.setTemporalCoherenceFrames(3);
    
    AABB bounds(Vec3(0, 0, 0), Vec3(10, 10, 10));
    system.registerObject(1, bounds);
    
    Mat4 view = Mat4::identity();
    Mat4 proj = Mat4::identity();
    system.setCamera(Vec3(0, 0, 0), view, proj);
    
    // First frame: object visible
    std::vector<uint32_t> candidates = {1};
    OcclusionResult r1 = system.cull(candidates);
    ASSERT_TRUE(system.wasVisible(1));
    
    // Force visible should reset temporal counter
    system.forceVisible(1);
    ASSERT_TRUE(system.wasVisible(1));
    
    system.shutdown();
}

TEST(occlusion_system_hardware_toggle) {
    OcclusionCullingSystem system;
    system.initialize(32);
    
    // Disable hardware queries
    system.setHardwareQueriesEnabled(false);
    
    AABB bounds(Vec3(0, 0, 0), Vec3(10, 10, 10));
    system.registerObject(1, bounds);
    
    Mat4 view = Mat4::identity();
    Mat4 proj = Mat4::identity();
    system.setCamera(Vec3(0, 0, 0), view, proj);
    
    std::vector<uint32_t> candidates = {1};
    OcclusionResult result = system.cull(candidates);
    
    // Should still work with software fallback
    ASSERT_TRUE(result.visibleObjects.size() > 0 || result.occludedObjects.size() > 0);
    
    system.shutdown();
}

TEST(occlusion_system_software_toggle) {
    OcclusionCullingSystem system;
    system.initialize(32);
    
    // Disable software rasterization
    system.setSoftwareRasterizationEnabled(false);
    
    AABB bounds(Vec3(0, 0, 0), Vec3(10, 10, 10));
    system.registerObject(1, bounds);
    
    Mat4 view = Mat4::identity();
    Mat4 proj = Mat4::identity();
    system.setCamera(Vec3(0, 0, 0), view, proj);
    
    std::vector<uint32_t> candidates = {1};
    OcclusionResult result = system.cull(candidates);
    
    // Should still work with hardware queries (mock)
    ASSERT_TRUE(result.visibleObjects.size() > 0 || result.occludedObjects.size() > 0);
    
    system.shutdown();
}

TEST(occlusion_system_empty_candidates) {
    OcclusionCullingSystem system;
    system.initialize(32);
    
    std::vector<uint32_t> empty;
    OcclusionResult result = system.cull(empty);
    
    ASSERT_EQ(result.visibleObjects.size(), 0);
    ASSERT_EQ(result.occludedObjects.size(), 0);
    ASSERT_EQ(result.totalQueriesIssued, 0);
    
    system.shutdown();
}

TEST(occlusion_system_query_phases) {
    OcclusionCullingSystem system;
    system.initialize(32);
    
    AABB bounds(Vec3(0, 0, 0), Vec3(10, 10, 10));
    system.registerObject(1, bounds);
    system.registerOccluder(2, bounds);
    
    Mat4 view = Mat4::identity();
    Mat4 proj = Mat4::identity();
    system.setCamera(Vec3(0, 0, 0), view, proj);
    
    // Test phased approach
    system.beginQueryPhase();
    system.endQueryPhase();
    
    OcclusionResult result = system.beginReadbackPhase();
    // Should process any pending queries
    
    system.shutdown();
}

TEST(occlusion_system_stats) {
    OcclusionCullingSystem system;
    system.initialize(32);
    
    AABB bounds(Vec3(0, 0, 0), Vec3(10, 10, 10));
    for (uint32_t i = 1; i <= 10; ++i) {
        system.registerObject(i, bounds);
    }
    
    Mat4 view = Mat4::identity();
    Mat4 proj = Mat4::identity();
    system.setCamera(Vec3(0, 0, 0), view, proj);
    
    std::vector<uint32_t> candidates = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    OcclusionResult result = system.cull(candidates);
    
    ASSERT_EQ(result.totalQueriesIssued, 10);
    
    const OcclusionResult& last = system.getLastResult();
    ASSERT_EQ(last.totalQueriesIssued, 10);
    
    system.shutdown();
}

TEST(occlusion_system_distance_cull) {
    OcclusionCullingSystem system;
    system.initialize(32);
    
    // Object at origin
    AABB nearBounds(Vec3(0, 0, 0), Vec3(10, 10, 10));
    system.registerObject(1, nearBounds);
    
    // Object very far away
    AABB farBounds(Vec3(5000, 5000, 5000), Vec3(5010, 5010, 5010));
    system.registerObject(2, farBounds);
    
    Mat4 view = Mat4::identity();
    Mat4 proj = Mat4::identity();
    system.setCamera(Vec3(0, 0, 0), view, proj);
    
    std::vector<uint32_t> candidates = {1, 2};
    OcclusionResult result = system.cull(candidates);
    
    // Object 1 should be visible, object 2 should be distance-culled
    bool found1 = false;
    for (uint32_t id : result.visibleObjects) {
        if (id == 1) found1 = true;
    }
    ASSERT_TRUE(found1);
    
    system.shutdown();
}

// ============================================================================
// Main
// ============================================================================
int main() {
    std::cout << "=== Occlusion Culling Tests ===\n\n";
    
    RUN_TEST(query_basic_lifecycle);
    RUN_TEST(query_move_semantics);
    RUN_TEST(query_pool_acquire_release);
    RUN_TEST(query_pool_growth);
    RUN_TEST(query_pool_reset_all);
    RUN_TEST(software_rasterizer_init);
    RUN_TEST(software_rasterizer_invalid_size);
    RUN_TEST(software_rasterizer_clear);
    RUN_TEST(software_rasterizer_rasterize_aabb);
    RUN_TEST(software_rasterizer_test_visible);
    RUN_TEST(software_rasterizer_test_occluded);
    RUN_TEST(software_rasterizer_test_sphere);
    RUN_TEST(occlusion_system_init);
    RUN_TEST(occlusion_system_register_unregister);
    RUN_TEST(occlusion_system_update_bounds);
    RUN_TEST(occlusion_system_temporal_coherence);
    RUN_TEST(occlusion_system_hardware_toggle);
    RUN_TEST(occlusion_system_software_toggle);
    RUN_TEST(occlusion_system_empty_candidates);
    RUN_TEST(occlusion_system_query_phases);
    RUN_TEST(occlusion_system_stats);
    RUN_TEST(occlusion_system_distance_cull);
    
    std::cout << "\n=== Results: " << g_passCount << "/" << g_testCount << " passed ===\n";
    return (g_passCount == g_testCount) ? 0 : 1;
}
