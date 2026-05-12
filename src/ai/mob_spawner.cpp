#include "ai/mob_spawner.h"
#include "voxel/block_types.h"
#include "voxel/block_registry.h"
#include "core/logger.h"
#include <cmath>
#include <algorithm>

namespace vge {

// ============================================
// Mob Definitions
// ============================================
MobDef MobSpawner::GetMobDef(MobType type) {
    switch (type) {
        case MobType::Zombie:
            return MobDef(MobType::Zombie, "Zombie", 20.0f, 2.5f, 3.0f, 7, true, false, true);
        case MobType::Skeleton:
            return MobDef(MobType::Skeleton, "Skeleton", 20.0f, 3.0f, 2.0f, 7, true, false, true);
        case MobType::Spider:
            return MobDef(MobType::Spider, "Spider", 16.0f, 3.5f, 2.0f, 7, false, true, false);
    }
    return MobDef(MobType::Zombie, "Zombie", 20.0f, 2.5f, 3.0f, 7, true, false, true);
}

// ============================================
// Constructor / Destructor
// ============================================
MobSpawner::MobSpawner(World* w, LightSystem* lights, DayNightCycle* cycle)
    : world(w)
    , lightSystem(lights)
    , dayNightCycle(cycle)
    , spawnTimer(0.0f)
    , rng(std::random_device{}()) {}

MobSpawner::~MobSpawner() {
    DespawnAllMobs();
}

// ============================================
// Update
// ============================================
void MobSpawner::Update(float deltaTime, const Vec3& playerPosition) {
    if (!world || !lightSystem || !dayNightCycle) return;

    // Update spawn timer
    spawnTimer += deltaTime;

    // Only spawn at night
    if (dayNightCycle->IsNight()) {
        // Try spawning periodically
        if (spawnTimer >= SPAWN_INTERVAL) {
            spawnTimer = 0.0f;
            TrySpawnMobs(playerPosition);
        }

        // Burn daylight-sensitive mobs that aren't in caves
        BurnDaylightMobs();
    } else {
        // Daytime: despawn all surface mobs (unless in caves)
        DespawnDaylightMobs();
    }

    // Despawn distant mobs
    DespawnDistantMobs(playerPosition);

    // Update mob chunk positions
    for (auto& mob : mobs) {
        if (!mob->alive) continue;

        int chunkX = static_cast<int>(std::floor(mob->position.x / CHUNK_SIZE));
        int chunkY = static_cast<int>(std::floor(mob->position.y / CHUNK_SIZE));
        int chunkZ = static_cast<int>(std::floor(mob->position.z / CHUNK_SIZE));

        if (mob->position.x < 0) chunkX--;
        if (mob->position.y < 0) chunkY--;
        if (mob->position.z < 0) chunkZ--;

        mob->chunkX = chunkX;
        mob->chunkY = chunkY;
        mob->chunkZ = chunkZ;

        // Update cave status
        mob->inCave = IsInCave(static_cast<int>(mob->position.x),
                               static_cast<int>(mob->position.y),
                               static_cast<int>(mob->position.z));
    }
}

// ============================================
// Spawn Logic
// ============================================
void MobSpawner::TrySpawnMobs(const Vec3& playerPosition) {
    // Try to spawn each mob type
    std::uniform_int_distribution<int> typeDist(0, 2);
    MobType type = static_cast<MobType>(typeDist(rng));

    Vec3 spawnPos = FindSpawnPosition(playerPosition, SPAWN_RADIUS);
    if (spawnPos.x != 0.0f || spawnPos.y != 0.0f || spawnPos.z != 0.0f) {
        int x = static_cast<int>(std::floor(spawnPos.x));
        int y = static_cast<int>(std::floor(spawnPos.y));
        int z = static_cast<int>(std::floor(spawnPos.z));

        if (CanSpawnAt(x, y, z, type)) {
            SpawnMob(type, spawnPos);
        }
    }
}

Mob* MobSpawner::SpawnMob(MobType type, const Vec3& position) {
    auto mob = std::make_unique<Mob>(type, position);
    Mob* ptr = mob.get();

    int chunkX = static_cast<int>(std::floor(position.x / CHUNK_SIZE));
    int chunkY = static_cast<int>(std::floor(position.y / CHUNK_SIZE));
    int chunkZ = static_cast<int>(std::floor(position.z / CHUNK_SIZE));

    if (position.x < 0) chunkX--;
    if (position.y < 0) chunkY--;
    if (position.z < 0) chunkZ--;

    mob->chunkX = chunkX;
    mob->chunkY = chunkY;
    mob->chunkZ = chunkZ;
    mob->inCave = IsInCave(static_cast<int>(position.x),
                           static_cast<int>(position.y),
                           static_cast<int>(position.z));

    mobs.push_back(std::move(mob));

    MobDef def = GetMobDef(type);
    Logger::Info("[MobSpawner] Spawned " + def.name + " at (" +
                 std::to_string(static_cast<int>(position.x)) + ", " +
                 std::to_string(static_cast<int>(position.y)) + ", " +
                 std::to_string(static_cast<int>(position.z)) + ")");

    return ptr;
}

void MobSpawner::DespawnMob(Mob* mob) {
    if (!mob) return;

    auto it = std::find_if(mobs.begin(), mobs.end(),
        [mob](const std::unique_ptr<Mob>& m) { return m.get() == mob; });

    if (it != mobs.end()) {
        MobDef def = GetMobDef(mob->type);
        Logger::Info("[MobSpawner] Despawned " + def.name);
        mobs.erase(it);
    }
}

void MobSpawner::DespawnAllMobs() {
    mobs.clear();
}

// ============================================
// Day/Night Transitions
// ============================================
void MobSpawner::OnNightBegin() {
    Logger::Info("[MobSpawner] Night began - mobs can now spawn");
}

void MobSpawner::OnDawnBegin() {
    Logger::Info("[MobSpawner] Dawn began - despawning surface mobs");
    DespawnDaylightMobs();
}

void MobSpawner::DespawnDaylightMobs() {
    auto it = mobs.begin();
    while (it != mobs.end()) {
        Mob* mob = it->get();
        MobDef def = GetMobDef(mob->type);

        // Despawn if burns in daylight and not in cave
        if (def.burnsInDaylight && !mob->inCave) {
            Logger::Info("[MobSpawner] " + def.name + " burned in daylight");
            it = mobs.erase(it);
        } else {
            ++it;
        }
    }
}

// ============================================
// Spawn Conditions
// ============================================
bool MobSpawner::CanSpawnAt(int x, int y, int z, MobType type) {
    MobDef def = GetMobDef(type);

    // Check light level
    if (!lightSystem) return false;
    int lightLevel = lightSystem->GetTotalLightLevel(x, y, z);
    if (lightLevel >= def.minLightLevel) {
        return false; // Too bright
    }

    // Check spawn cap for chunk
    int chunkX = x / CHUNK_SIZE;
    int chunkY = y / CHUNK_SIZE;
    int chunkZ = z / CHUNK_SIZE;

    if (x < 0) chunkX--;
    if (y < 0) chunkY--;
    if (z < 0) chunkZ--;

    if (CountMobsInChunk(chunkX, chunkY, chunkZ) >= MAX_MOBS_PER_CHUNK) {
        return false; // Chunk at capacity
    }

    // Check valid position
    if (!IsValidSpawnPosition(x, y, z)) {
        return false;
    }

    // Type-specific checks
    if (def.spawnsOnGround) {
        // Needs solid ground below
        BlockTypeID below = world->GetBlock(x, y - 1, z);
        if (below == BLOCK_AIR) {
            return false;
        }
        const BlockDef& blockDef = BlockRegistry::GetInstance().GetBlock(below);
        if (!blockDef.IsSolid()) {
            return false;
        }
    }

    return true;
}

bool MobSpawner::IsValidSpawnPosition(int x, int y, int z) {
    if (!world) return false;

    // Position must be air
    BlockTypeID block = world->GetBlock(x, y, z);
    if (block != BLOCK_AIR) {
        return false;
    }

    // Position above must be air (headroom)
    BlockTypeID above = world->GetBlock(x, y + 1, z);
    if (above != BLOCK_AIR) {
        return false;
    }

    return true;
}

bool MobSpawner::IsInCave(int x, int y, int z) {
    if (!world) return false;

    // Simple cave detection: check if there's solid blocks above
    // up to a reasonable height (e.g., 20 blocks)
    for (int checkY = y + 2; checkY < y + 25; ++checkY) {
        BlockTypeID block = world->GetBlock(x, checkY, z);
        if (block == BLOCK_AIR) {
            // Found air - might be surface
            // Check further to see if it's open sky
            bool openSky = true;
            for (int furtherY = checkY; furtherY < checkY + 10; ++furtherY) {
                BlockTypeID further = world->GetBlock(x, furtherY, z);
                if (further != BLOCK_AIR) {
                    openSky = false;
                    break;
                }
            }
            if (openSky) {
                return false; // Not in cave
            }
        }
    }

    return true; // Likely in cave
}

int MobSpawner::CountMobsInChunk(int chunkX, int chunkY, int chunkZ) const {
    int count = 0;
    for (const auto& mob : mobs) {
        if (mob->chunkX == chunkX && mob->chunkY == chunkY && mob->chunkZ == chunkZ) {
            count++;
        }
    }
    return count;
}

Vec3 MobSpawner::FindSpawnPosition(const Vec3& center, float radius) {
    if (!world) return Vec3(0, 0, 0);

    std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f);
    std::uniform_real_distribution<float> radiusDist(radius * 0.5f, radius);

    // Try several random positions
    for (int attempt = 0; attempt < 10; ++attempt) {
        float angle = angleDist(rng);
        float dist = radiusDist(rng);

        int x = static_cast<int>(std::floor(center.x + std::cos(angle) * dist));
        int z = static_cast<int>(std::floor(center.z + std::sin(angle) * dist));

        // Find ground level at this x,z
        int chunkX = x / CHUNK_SIZE;
        int chunkZ = z / CHUNK_SIZE;
        if (x < 0) chunkX--;
        if (z < 0) chunkZ--;

        Chunk* chunk = world->GetChunk(chunkX, 0, chunkZ);
        if (!chunk) continue;

        // Find highest solid block
        for (int y = CHUNK_SIZE - 2; y >= 0; --y) {
            BlockTypeID block = world->GetBlock(x, y, z);
            if (block != BLOCK_AIR) {
                const BlockDef& def = BlockRegistry::GetInstance().GetBlock(block);
                if (def.IsSolid()) {
                    // Found ground, spawn above it
                    int spawnY = y + 1;
                    if (IsValidSpawnPosition(x, spawnY, spawnY + 1)) {
                        return Vec3(static_cast<float>(x), static_cast<float>(spawnY), static_cast<float>(z));
                    }
                    break;
                }
            }
        }
    }

    return Vec3(0, 0, 0); // Failed to find valid position
}

// ============================================
// Despawn Logic
// ============================================
void MobSpawner::DespawnDistantMobs(const Vec3& playerPos) {
    auto it = mobs.begin();
    while (it != mobs.end()) {
        float dist = ((*it)->position - playerPos).length();
        if (dist > DESPAWN_RADIUS) {
            it = mobs.erase(it);
        } else {
            ++it;
        }
    }
}

void MobSpawner::BurnDaylightMobs() {
    // Only burn if it's actually day (not just transitioning)
    if (!dayNightCycle || !dayNightCycle->IsDay()) return;

    auto it = mobs.begin();
    while (it != mobs.end()) {
        Mob* mob = it->get();
        MobDef def = GetMobDef(mob->type);

        if (def.burnsInDaylight && !mob->inCave) {
            // Check if in direct sunlight
            int x = static_cast<int>(mob->position.x);
            int y = static_cast<int>(mob->position.y);
            int z = static_cast<int>(mob->position.z);

            if (lightSystem && lightSystem->GetSkyLight(x, y, z) > 10) {
                Logger::Info("[MobSpawner] " + def.name + " burning in sunlight");
                it = mobs.erase(it);
                continue;
            }
        }
        ++it;
    }
}

// ============================================
// Queries
// ============================================
std::vector<Mob*> MobSpawner::GetMobsInChunk(int chunkX, int chunkY, int chunkZ) const {
    std::vector<Mob*> result;
    for (const auto& mob : mobs) {
        if (mob->chunkX == chunkX && mob->chunkY == chunkY && mob->chunkZ == chunkZ) {
            result.push_back(mob.get());
        }
    }
    return result;
}

std::vector<Mob*> MobSpawner::GetMobsInRadius(const Vec3& center, float radius) const {
    std::vector<Mob*> result;
    for (const auto& mob : mobs) {
        if ((mob->position - center).length() <= radius) {
            result.push_back(mob.get());
        }
    }
    return result;
}

std::vector<Mob*> MobSpawner::GetAllMobs() const {
    std::vector<Mob*> result;
    for (const auto& mob : mobs) {
        result.push_back(mob.get());
    }
    return result;
}

} // namespace vge
