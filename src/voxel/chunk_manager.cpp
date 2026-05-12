#include "voxel/chunk_manager.h"
#include "voxel/chunk.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "core/save_system.h"
#include "core/logger.h"
#include <iostream>
#include <cmath>
#include <fstream>
#include <filesystem>

namespace vge {

ChunkManager::ChunkManager() : viewDistance(8), world(nullptr), generator(nullptr) {}

ChunkManager::~ChunkManager() {
    Clear();
}

void ChunkManager::Initialize(World* world, WorldGenerator* generator) {
    this->world = world;
    this->generator = generator;
    std::cout << "[ChunkManager] Initialized with view distance " << viewDistance << std::endl;
}

void ChunkManager::Update(const Vec3& playerPosition) {
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
                
                // Only load if within circular distance
                float dist = std::sqrt((float)(x*x + y*y + z*z));
                if (dist <= viewDistance) {
                    LoadChunk(cx, cy, cz);
                }
            }
        }
    }
    
    // Unload distant chunks
    UnloadDistantChunks(playerChunkX, playerChunkY, playerChunkZ, viewDistance + 2);
}

Chunk* ChunkManager::LoadChunk(int x, int y, int z) {
    if (!world) return nullptr;
    
    // Check if already loaded
    uint64_t key = ((uint64_t)(x + 1000000) << 42) | ((uint64_t)(y + 1000000) << 21) | (uint64_t)(z + 1000000);
    auto it = loadedChunks.find(key);
    if (it != loadedChunks.end()) {
        return it->second.get();
    }
    
    Chunk* chunk = world->GetChunk(x, y, z);
    if (!chunk) {
        chunk = world->GetOrCreateChunk(x, y, z);
        if (generator && chunk) {
            generator->GenerateChunk(*chunk, x, y, z);
        }
    }
    
    if (chunk) {
        loadedChunks[key] = std::unique_ptr<Chunk>(chunk);
    }
    
    return chunk;
}

void ChunkManager::UnloadChunk(int x, int y, int z) {
    uint64_t key = ((uint64_t)(x + 1000000) << 42) | ((uint64_t)(y + 1000000) << 21) | (uint64_t)(z + 1000000);
    
    auto it = loadedChunks.find(key);
    if (it != loadedChunks.end()) {
        Chunk* chunk = it->second.get();
        
        // Save modified chunks before unloading
        if (chunk && chunk->modified) {
            std::string filename = "saves/chunks/chunk_" + 
                std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z) + ".bin";
            std::ofstream file(filename, std::ios::binary);
            if (file.is_open()) {
                SaveSystem::SaveChunk(*chunk, file);
                Logger::Info("Saved modified chunk to: " + filename);
            } else {
                Logger::Error("Failed to save chunk: " + filename);
            }
        }
        
        loadedChunks.erase(it);
        Logger::Info("Unloaded chunk at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
    }
}

void ChunkManager::UnloadDistantChunks(int centerX, int centerY, int centerZ, int radius) {
    std::vector<uint64_t> toUnload;
    
    for (auto& pair : loadedChunks) {
        Chunk* chunk = pair.second.get();
        if (!chunk) continue;
        
        int dx = chunk->GetChunkX() - centerX;
        int dy = chunk->GetChunkY() - centerY;
        int dz = chunk->GetChunkZ() - centerZ;
        
        float dist = std::sqrt((float)(dx*dx + dy*dy + dz*dz));
        if (dist > radius) {
            toUnload.push_back(pair.first);
        }
    }
    
    for (uint64_t key : toUnload) {
        auto it = loadedChunks.find(key);
        if (it != loadedChunks.end()) {
            Chunk* chunk = it->second.get();
            if (chunk) {
                // Extract coordinates from key
                int x = (int)((key >> 42) & 0x1FFFFF) - 1000000;
                int y = (int)((key >> 21) & 0x1FFFFF) - 1000000;
                int z = (int)(key & 0x1FFFFF) - 1000000;
                UnloadChunk(x, y, z);
            }
        }
    }
}

void ChunkManager::Clear() {
    loadedChunks.clear();
}

} // namespace vge