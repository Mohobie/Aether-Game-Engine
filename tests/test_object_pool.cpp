#include "aether/object_pool.h"
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

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
// Tests
// ============================================================================

TEST(basic_acquire_release) {
    ObjectPool<Entity> pool(4, 100);

    ASSERT_EQ(pool.available(), 4);
    ASSERT_EQ(pool.active(), 0);

    Entity* e1 = pool.acquire();
    ASSERT_TRUE(e1 != nullptr);
    ASSERT_TRUE(e1->isPooled());  // owned by pool
    ASSERT_TRUE(e1->isActive());  // currently acquired
    ASSERT_EQ(pool.active(), 1);
    ASSERT_EQ(pool.available(), 3);

    pool.release(e1);
    ASSERT_FALSE(e1->isActive());
    ASSERT_EQ(pool.active(), 0);
    ASSERT_EQ(pool.available(), 4);
}

TEST(acquire_cleans_state) {
    ObjectPool<Entity> pool(4, 100);

    Entity* e = pool.acquire();
    e->x = 42.0f;
    e->id = 99;
    e->destroyed = true;
    pool.release(e);

    Entity* e2 = pool.acquire();
    ASSERT_EQ(e2, e); // same memory
    ASSERT_EQ(e2->x, 0.0f); // reset by onAcquire
    ASSERT_EQ(e2->id, 0);
    ASSERT_FALSE(e2->destroyed);
    pool.release(e2);
}

TEST(auto_grow) {
    ObjectPool<Particle> pool(2, 16, 2.0);

    Particle* p1 = pool.acquire();
    Particle* p2 = pool.acquire();
    ASSERT_TRUE(p1 && p2);
    ASSERT_EQ(pool.capacity(), 2);

    // This should trigger growth
    Particle* p3 = pool.acquire();
    ASSERT_TRUE(p3 != nullptr);
    ASSERT_EQ(pool.capacity(), 4); // doubled

    pool.release(p1);
    pool.release(p2);
    pool.release(p3);
}

TEST(max_capacity_limit) {
    ObjectPool<Entity> pool(2, 4, 2.0);

    Entity* e1 = pool.acquire();
    Entity* e2 = pool.acquire();
    Entity* e3 = pool.acquire(); // grows to 4
    Entity* e4 = pool.acquire();
    ASSERT_TRUE(e1 && e2 && e3 && e4);

    Entity* e5 = pool.acquire(); // at max, should fail
    ASSERT_TRUE(e5 == nullptr);

    pool.release(e1);
    pool.release(e2);
    pool.release(e3);
    pool.release(e4);
}

TEST(scoped_handle) {
    ObjectPool<Entity> pool(4, 100);
    Entity* raw = nullptr;

    {
        auto handle = pool.acquireScoped();
        ASSERT_TRUE(handle);
        raw = handle.get();
        ASSERT_TRUE(raw->isActive());
    }

    ASSERT_FALSE(raw->isActive());
    ASSERT_EQ(pool.active(), 0);
}

TEST(move_scoped_handle) {
    ObjectPool<Entity> pool(4, 100);

    auto h1 = pool.acquireScoped();
    ASSERT_TRUE(h1);

    auto h2 = std::move(h1);
    ASSERT_FALSE(h1); // moved-from
    ASSERT_TRUE(h2);

    pool.release(h2.get());
}

TEST(stats_tracking) {
    ObjectPool<Entity> pool(4, 100);

    auto s1 = pool.stats();
    ASSERT_EQ(s1.capacity, 4);
    ASSERT_EQ(s1.available, 4);
    ASSERT_EQ(s1.active, 0);

    Entity* e1 = pool.acquire();
    Entity* e2 = pool.acquire();
    pool.release(e1);

    auto s2 = pool.stats();
    ASSERT_EQ(s2.active, 1);
    ASSERT_EQ(s2.totalAcquires, 2);
    ASSERT_EQ(s2.totalReleases, 1);
    ASSERT_EQ(s2.peakActive, 2);

    pool.release(e2);
}

TEST(pool_manager_registry) {
    auto& pool = PoolManager::getOrCreate<Entity>("test_entities", 8, 64);
    ASSERT_EQ(pool.capacity(), 8);

    Entity* e = pool.acquire();
    ASSERT_TRUE(e != nullptr);
    pool.release(e);

    // Retrieve same pool
    auto* same = PoolManager::get<Entity>("test_entities");
    ASSERT_TRUE(same != nullptr);
    ASSERT_EQ(same->capacity(), 8);

    // List pools
    auto names = PoolManager::listPools();
    ASSERT_TRUE(std::find(names.begin(), names.end(), "test_entities") != names.end());

    // Clean up before destroyPool to avoid active-object assert on pool destruction
    PoolManager::destroyPool("test_entities");
    ASSERT_TRUE(PoolManager::get<Entity>("test_entities") == nullptr);
}

TEST(particle_lifecycle) {
    ObjectPool<Particle> pool(4, 100);

    Particle* p = pool.acquire();
    p->life = 2.5f;
    p->maxLife = 5.0f;
    p->color = 0xFF00FF00;

    ASSERT_EQ(p->normalizedLife(), 0.5f);

    pool.release(p);
    Particle* p2 = pool.acquire();
    ASSERT_EQ(p2, p);
    ASSERT_EQ(p2->life, 0.0f); // reset
    ASSERT_EQ(p2->color, 0xFFFFFFFF);
    pool.release(p2);
}

TEST(temporary_object_expiry) {
    ObjectPool<TemporaryObject> pool(4, 100);

    TemporaryObject* t = pool.acquire();
    t->duration = 1.0f;
    t->elapsed = 0.5f;
    ASSERT_FALSE(t->isExpired());

    t->elapsed = 1.5f;
    ASSERT_TRUE(t->isExpired());

    pool.release(t);
}

TEST(concurrent_stress) {
    ObjectPool<Entity> pool(16, 1024, 2.0, nullptr, true);
    constexpr int threads = 4;
    constexpr int ops = 100;

    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&pool]() {
            for (int i = 0; i < ops; ++i) {
                Entity* e = pool.acquire();
                if (e) {
                    e->x = static_cast<float>(i);
                    std::this_thread::yield();
                    pool.release(e);
                }
            }
        });
    }

    for (auto& w : workers) w.join();

    auto s = pool.stats();
    ASSERT_EQ(s.active, 0);
    ASSERT_EQ(s.totalAcquires, threads * ops);
    ASSERT_EQ(s.totalReleases, threads * ops);
}

TEST(shrink_to_fit) {
    ObjectPool<Entity> pool(16, 256, 2.0);

    std::vector<Entity*> ents;
    for (int i = 0; i < 8; ++i) {
        ents.push_back(pool.acquire());
    }

    // Grow to 16
    ASSERT_EQ(pool.capacity(), 16);

    // Release all
    for (auto* e : ents) pool.release(e);

    // Shrink to minimum 4
    bool shrunk = pool.shrinkToFit(4);
    ASSERT_TRUE(shrunk);
    ASSERT_EQ(pool.capacity(), 4);
    ASSERT_EQ(pool.available(), 4);
}

// ============================================================================
// Main
// ============================================================================
int main() {
    std::cout << "=== Object Pool Tests ===\n\n";

    RUN_TEST(basic_acquire_release);
    RUN_TEST(acquire_cleans_state);
    RUN_TEST(auto_grow);
    RUN_TEST(max_capacity_limit);
    RUN_TEST(scoped_handle);
    RUN_TEST(move_scoped_handle);
    RUN_TEST(stats_tracking);
    RUN_TEST(pool_manager_registry);
    RUN_TEST(particle_lifecycle);
    RUN_TEST(temporary_object_expiry);
    RUN_TEST(concurrent_stress);
    RUN_TEST(shrink_to_fit);

    std::cout << "\n=== Results: " << g_passCount << "/" << g_testCount << " passed ===\n";
    return (g_passCount == g_testCount) ? 0 : 1;
}
