#include "voxel/chunk_manager.h"
#include "voxel/chunk.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include <iostream>

// Stub implementation
namespace vge {

ChunkManager::ChunkManager() : viewDistance(16), world(nullptr), generator(nullptr) {}

ChunkManager::~ChunkManager() {
    Clear();
}

void ChunkManager::Initialize(World* world, WorldGenerator* generator) {
    this->world = world;
    this->generator = generator;
    std::cout << "[ChunkManager] Initialized with view distance " << viewDistance << std::endl;
}

void ChunkManager::Update(const Vec3& playerPosition) {
    // Would load/unload chunks based on player position
    int playerChunkX = (int)(playerPosition.x / CHUNK_SIZE);
    int playerChunkY = (int)(playerPosition.y / CHUNK_SIZE);
    int playerChunkZ = (int)(playerPosition.z / CHUNK_SIZE);
    
    // Load chunks within view distance
    for (int x = -viewDistance; x <= viewDistance; ++x) {
        for (int y = -2; y <= 2; ++y) {
            for (int z = -viewDistance; z <= viewDistance; ++z) {
                int cx = playerChunkX + x;
                int cy = playerChunkY + y;
                int cz = playerChunkZ + z;
                
                LoadChunk(cx, cy, cz);
            }
        }
    }
}

Chunk* ChunkManager::LoadChunk(int x, int y, int z) {
    if (!world) return nullptr;
    
    Chunk* chunk = world->GetChunk(x, y, z);
    if (!chunk) {
        chunk = world->GetOrCreateChunk(x, y, z);
        if (generator) {
            generator->GenerateChunk(*chunk, x, y, z);
        }
    }
    
    return chunk;
}

void ChunkManager::UnloadChunk(int x, int y, int z) {
    // Would remove chunk from memory
}

void ChunkManager::UnloadDistantChunks(int centerX, int centerY, int centerZ, int radius) {
    // Would unload chunks outside radius
}

void ChunkManager::Clear() {
    loadedChunks.clear();
}

} // namespace vge