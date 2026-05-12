#include "rendering/light_system.h"
#include "voxel/block_types.h"
#include "voxel/block_registry.h"
#include <algorithm>
#include <cmath>

namespace vge {

LightSystem::LightSystem(World* w)
    : world(w)
    , skyLightIntensity(1.0f) {}

LightSystem::~LightSystem() {}

void LightSystem::SetSkyLightIntensity(float intensity) {
    skyLightIntensity = std::max(0.0f, std::min(1.0f, intensity));
}

int LightSystem::GetLightLevel(int x, int y, int z) const {
    if (!world) return 15;

    int chunkX = x / CHUNK_SIZE;
    int chunkY = y / CHUNK_SIZE;
    int chunkZ = z / CHUNK_SIZE;

    if (x < 0) chunkX--;
    if (y < 0) chunkY--;
    if (z < 0) chunkZ--;

    Chunk* chunk = world->GetChunk(chunkX, chunkY, chunkZ);
    if (!chunk) return 15; // Fully lit outside loaded chunks

    int localX = x - chunkX * CHUNK_SIZE;
    int localY = y - chunkY * CHUNK_SIZE;
    int localZ = z - chunkZ * CHUNK_SIZE;

    return chunk->GetLightLevel(localX, localY, localZ);
}

int LightSystem::GetBlockLight(int x, int y, int z) const {
    // Block light is stored directly in chunk
    return GetLightLevel(x, y, z);
}

int LightSystem::GetSkyLight(int x, int y, int z) const {
    // Sky light depends on exposure to sky and time of day
    // For simplicity: if block is at or above surface, return sky intensity
    // Otherwise 0 (blocked)
    if (!world) return 0;

    // Check if there's a clear path to the sky (simplified)
    int chunkX = x / CHUNK_SIZE;
    int chunkY = y / CHUNK_SIZE;
    int chunkZ = z / CHUNK_SIZE;

    if (x < 0) chunkX--;
    if (y < 0) chunkY--;
    if (z < 0) chunkZ--;

    Chunk* chunk = world->GetChunk(chunkX, chunkY, chunkZ);
    if (!chunk) return static_cast<int>(15.0f * skyLightIntensity);

    int localX = x - chunkX * CHUNK_SIZE;
    int localY = y - chunkY * CHUNK_SIZE;
    int localZ = z - chunkZ * CHUNK_SIZE;

    // Check blocks above
    for (int checkY = localY + 1; checkY < CHUNK_SIZE; ++checkY) {
        BlockTypeID block = chunk->GetBlock(localX, checkY, localZ);
        if (block != BLOCK_AIR) {
            const BlockDef& def = BlockRegistry::GetInstance().GetBlock(block);
            if (def.IsOpaque()) {
                return 0; // Blocked
            }
        }
    }

    // Check chunk above
    Chunk* chunkAbove = world->GetChunk(chunkX, chunkY + 1, chunkZ);
    if (chunkAbove) {
        for (int checkY = 0; checkY < CHUNK_SIZE; ++checkY) {
            BlockTypeID block = chunkAbove->GetBlock(localX, checkY, localZ);
            if (block != BLOCK_AIR) {
                const BlockDef& def = BlockRegistry::GetInstance().GetBlock(block);
                if (def.IsOpaque()) {
                    return 0; // Blocked
                }
            }
        }
    }

    return static_cast<int>(15.0f * skyLightIntensity);
}

void LightSystem::AddBlockLightSource(const Vec3& position, int level, int radius) {
    lightSources.emplace_back(position, level, radius);

    int x = static_cast<int>(std::floor(position.x));
    int y = static_cast<int>(std::floor(position.y));
    int z = static_cast<int>(std::floor(position.z));

    PropagateLight(x, y, z, level);
}

void LightSystem::RemoveBlockLightSource(const Vec3& position) {
    auto it = std::remove_if(lightSources.begin(), lightSources.end(),
        [&position](const BlockLightSource& src) {
            int sx = static_cast<int>(std::floor(src.position.x));
            int sy = static_cast<int>(std::floor(src.position.y));
            int sz = static_cast<int>(std::floor(src.position.z));
            int px = static_cast<int>(std::floor(position.x));
            int py = static_cast<int>(std::floor(position.y));
            int pz = static_cast<int>(std::floor(position.z));
            return sx == px && sy == py && sz == pz;
        });
    lightSources.erase(it, lightSources.end());

    // Recompute all lighting
    UpdateAllLighting();
}

void LightSystem::ClearLightSources() {
    lightSources.clear();
    UpdateAllLighting();
}

void LightSystem::PlaceTorch(int x, int y, int z, int level) {
    AddBlockLightSource(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), level, 14);
}

void LightSystem::RemoveTorch(int x, int y, int z) {
    RemoveBlockLightSource(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
}

void LightSystem::PropagateLight(int startX, int startY, int startZ, int startLevel) {
    if (!world || startLevel <= 0) return;

    std::queue<LightNode> queue;
    queue.push({startX, startY, startZ, startLevel});

    // Simple BFS flood fill
    while (!queue.empty()) {
        LightNode node = queue.front();
        queue.pop();

        int chunkX = node.x / CHUNK_SIZE;
        int chunkY = node.y / CHUNK_SIZE;
        int chunkZ = node.z / CHUNK_SIZE;

        if (node.x < 0) chunkX--;
        if (node.y < 0) chunkY--;
        if (node.z < 0) chunkZ--;

        Chunk* chunk = world->GetChunk(chunkX, chunkY, chunkZ);
        if (!chunk) continue;

        int localX = node.x - chunkX * CHUNK_SIZE;
        int localY = node.y - chunkY * CHUNK_SIZE;
        int localZ = node.z - chunkZ * CHUNK_SIZE;

        if (localX < 0 || localX >= CHUNK_SIZE ||
            localY < 0 || localY >= CHUNK_SIZE ||
            localZ < 0 || localZ >= CHUNK_SIZE) {
            continue;
        }

        int currentLevel = chunk->GetLightLevel(localX, localY, localZ);
        if (node.level <= currentLevel) continue;

        chunk->SetLightLevel(localX, localY, localZ, node.level);

        if (node.level > 1) {
            int nextLevel = node.level - 1;

            // Spread to neighbors
            queue.push({node.x + 1, node.y, node.z, nextLevel});
            queue.push({node.x - 1, node.y, node.z, nextLevel});
            queue.push({node.x, node.y + 1, node.z, nextLevel});
            queue.push({node.x, node.y - 1, node.z, nextLevel});
            queue.push({node.x, node.y, node.z + 1, nextLevel});
            queue.push({node.x, node.y, node.z - 1, nextLevel});
        }
    }
}

void LightSystem::UpdateChunkLighting(int chunkX, int chunkY, int chunkZ) {
    if (!world) return;

    Chunk* chunk = world->GetChunk(chunkX, chunkY, chunkZ);
    if (!chunk) return;

    // Reset light levels
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                chunk->SetLightLevel(x, y, z, 0);
            }
        }
    }

    // Re-apply all light sources that affect this chunk
    int chunkWorldX = chunkX * CHUNK_SIZE;
    int chunkWorldY = chunkY * CHUNK_SIZE;
    int chunkWorldZ = chunkZ * CHUNK_SIZE;

    for (const auto& source : lightSources) {
        int sx = static_cast<int>(std::floor(source.position.x));
        int sy = static_cast<int>(std::floor(source.position.y));
        int sz = static_cast<int>(std::floor(source.position.z));

        // Check if source affects this chunk
        if (sx >= chunkWorldX - source.radius && sx < chunkWorldX + CHUNK_SIZE + source.radius &&
            sy >= chunkWorldY - source.radius && sy < chunkWorldY + CHUNK_SIZE + source.radius &&
            sz >= chunkWorldZ - source.radius && sz < chunkWorldZ + CHUNK_SIZE + source.radius) {
            PropagateLight(sx, sy, sz, source.level);
        }
    }

    // Apply sky light
    PropagateSkyLight(chunkX, chunkY, chunkZ);
}

void LightSystem::PropagateSkyLight(int chunkX, int chunkY, int chunkZ) {
    if (!world) return;

    Chunk* chunk = world->GetChunk(chunkX, chunkY, chunkZ);
    if (!chunk) return;

    int skyLevel = static_cast<int>(15.0f * skyLightIntensity);
    if (skyLevel <= 0) return;

    // Top-down propagation for sky light
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            int currentLevel = skyLevel;

            for (int y = CHUNK_SIZE - 1; y >= 0; --y) {
                BlockTypeID block = chunk->GetBlock(x, y, z);

                if (block != BLOCK_AIR) {
                    const BlockDef& def = BlockRegistry::GetInstance().GetBlock(block);
                    if (def.IsOpaque()) {
                        currentLevel = 0; // Blocked
                    } else {
                        currentLevel = std::max(0, currentLevel - 1); // Reduced by translucent block
                    }
                }

                int existingLevel = chunk->GetLightLevel(x, y, z);
                chunk->SetLightLevel(x, y, z, std::max(existingLevel, currentLevel));

                if (currentLevel <= 0) break;
            }
        }
    }
}

void LightSystem::UpdateAllLighting() {
    if (!world) return;

    // Reset all chunk lighting
    for (auto& pair : world->GetChunks()) {
        Chunk* chunk = pair.second.get();
        if (!chunk) continue;

        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int y = 0; y < CHUNK_SIZE; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    chunk->SetLightLevel(x, y, z, 0);
                }
            }
        }
    }

    // Re-apply all light sources
    for (const auto& source : lightSources) {
        int sx = static_cast<int>(std::floor(source.position.x));
        int sy = static_cast<int>(std::floor(source.position.y));
        int sz = static_cast<int>(std::floor(source.position.z));
        PropagateLight(sx, sy, sz, source.level);
    }

    // Re-apply sky light to all chunks
    for (auto& pair : world->GetChunks()) {
        Chunk* chunk = pair.second.get();
        if (!chunk) continue;
        PropagateSkyLight(chunk->GetChunkX(), chunk->GetChunkY(), chunk->GetChunkZ());
    }
}

void LightSystem::UpdateSkyLightForTime(float dayNightBlend) {
    SetSkyLightIntensity(dayNightBlend);

    // Update sky light for all loaded chunks
    if (!world) return;

    for (auto& pair : world->GetChunks()) {
        Chunk* chunk = pair.second.get();
        if (!chunk) continue;
        PropagateSkyLight(chunk->GetChunkX(), chunk->GetChunkY(), chunk->GetChunkZ());
    }
}

int LightSystem::GetTotalLightLevel(int x, int y, int z) const {
    int blockLight = GetBlockLight(x, y, z);
    int skyLight = GetSkyLight(x, y, z);
    return std::max(blockLight, skyLight);
}

bool LightSystem::IsDarkEnoughForMobs(int x, int y, int z, int threshold) const {
    return GetTotalLightLevel(x, y, z) < threshold;
}

} // namespace vge
