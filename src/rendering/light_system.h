#pragma once
#include "voxel/world.h"
#include "voxel/chunk.h"
#include "math/vec3.h"
#include <vector>
#include <queue>
#include <functional>

namespace vge {

// ============================================
// Light Source
// ============================================
struct BlockLightSource {
    Vec3 position;
    int level;        // 0-15
    int radius;       // Spread radius
    bool isSky;       // True for sky light

    BlockLightSource(const Vec3& pos, int lvl, int rad, bool sky = false)
        : position(pos), level(lvl), radius(rad), isSky(sky) {}
};

// ============================================
// Light System
// ============================================
// Manages block light levels (0-15) for:
//   - Block light (torches, glowstone, etc.)
//   - Sky light (varies with time of day)
//   - Ambient occlusion
//
// Light propagation uses simple flood fill.
// ============================================
class LightSystem {
private:
    World* world;
    std::vector<BlockLightSource> lightSources;
    float skyLightIntensity;  // 0.0-1.0 based on time of day

    // Flood fill queue entry
    struct LightNode {
        int x, y, z;
        int level;
    };

    void PropagateLight(int startX, int startY, int startZ, int startLevel);
    void PropagateSkyLight(int chunkX, int chunkY, int chunkZ);

public:
    LightSystem(World* w);
    ~LightSystem();

    // Sky light intensity (updated by DayNightCycle)
    void SetSkyLightIntensity(float intensity);
    float GetSkyLightIntensity() const { return skyLightIntensity; }

    // Light queries
    int GetLightLevel(int x, int y, int z) const;
    int GetBlockLight(int x, int y, int z) const;
    int GetSkyLight(int x, int y, int z) const;

    // Light source management
    void AddBlockLightSource(const Vec3& position, int level, int radius);
    void RemoveBlockLightSource(const Vec3& position);
    void ClearLightSources();

    // Torch placement (convenience)
    void PlaceTorch(int x, int y, int z, int level = 14);
    void RemoveTorch(int x, int y, int z);

    // Chunk lighting
    void UpdateChunkLighting(int chunkX, int chunkY, int chunkZ);
    void UpdateAllLighting();

    // Sky light update (call when time of day changes)
    void UpdateSkyLightForTime(float dayNightBlend); // 0.0 = night, 1.0 = day

    // Light at position (combined block + sky)
    int GetTotalLightLevel(int x, int y, int z) const;

    // Check if position is dark enough for mob spawning
    bool IsDarkEnoughForMobs(int x, int y, int z, int threshold = 7) const;

    // Get all light sources
    const std::vector<BlockLightSource>& GetLightSources() const { return lightSources; }
};

} // namespace vge
