#include "save_system.h"
#include <iostream>
#include <fstream>
#include <cstring>

namespace vge {

bool SaveSystem::SaveWorld(const World& world, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[Save] Failed to open: " << path << std::endl;
        return false;
    }
    
    // Count chunks
    uint32_t chunkCount = 0;
    for (int x = -100; x <= 100; x++) {
        for (int y = -10; y <= 10; y++) {
            for (int z = -100; z <= 100; z++) {
                Chunk* chunk = const_cast<World&>(world).GetChunk(x, y, z);
                if (chunk && chunk->loaded) {
                    chunkCount++;
                }
            }
        }
    }
    
    // Write header
    SaveHeader header;
    header.magic = SAVE_MAGIC;
    header.version = SAVE_VERSION;
    header.chunkCount = chunkCount;
    
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // Write each chunk
    uint32_t savedChunks = 0;
    for (int x = -100; x <= 100; x++) {
        for (int y = -10; y <= 10; y++) {
            for (int z = -100; z <= 100; z++) {
                Chunk* chunk = const_cast<World&>(world).GetChunk(x, y, z);
                if (chunk && chunk->loaded) {
                    SaveChunk(*chunk, file);
                    savedChunks++;
                }
            }
        }
    }
    
    std::cout << "[Save] World saved to: " << path << " (" << savedChunks << " chunks)" << std::endl;
    return true;
}

bool SaveSystem::LoadWorld(World& world, const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[Save] Failed to open: " << path << std::endl;
        return false;
    }
    
    SaveHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (header.magic != SAVE_MAGIC) {
        std::cerr << "[Save] Invalid save file" << std::endl;
        return false;
    }
    
    if (header.version != SAVE_VERSION) {
        std::cerr << "[Save] Version mismatch: " << header.version << " vs " << SAVE_VERSION << std::endl;
        return false;
    }
    
    // Load each chunk
    for (uint32_t i = 0; i < header.chunkCount; i++) {
        // Read chunk coordinates first
        int cx, cy, cz;
        file.read(reinterpret_cast<char*>(&cx), sizeof(int));
        file.read(reinterpret_cast<char*>(&cy), sizeof(int));
        file.read(reinterpret_cast<char*>(&cz), sizeof(int));
        
        // Seek back to read full chunk
        file.seekg(-static_cast<int>(sizeof(int) * 3), std::ios::cur);
        
        Chunk* chunk = world.GetOrCreateChunk(cx, cy, cz);
        if (chunk) {
            LoadChunk(*chunk, file);
        }
    }
    
    std::cout << "[Save] World loaded from: " << path << " (" << header.chunkCount << " chunks)" << std::endl;
    return true;
}

bool SaveSystem::SaveChunk(const Chunk& chunk, std::ofstream& file) {
    // Write chunk coordinates first
    int cx = chunk.GetChunkX();
    int cy = chunk.GetChunkY();
    int cz = chunk.GetChunkZ();
    file.write(reinterpret_cast<const char*>(&cx), sizeof(int));
    file.write(reinterpret_cast<const char*>(&cy), sizeof(int));
    file.write(reinterpret_cast<const char*>(&cz), sizeof(int));
    
    // Write block data
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                uint8_t block = static_cast<uint8_t>(chunk.GetBlock(x, y, z));
                file.write(reinterpret_cast<const char*>(&block), sizeof(uint8_t));
            }
        }
    }
    return true;
}

bool SaveSystem::LoadChunk(Chunk& chunk, std::ifstream& file) {
    // Read chunk coordinates (already read in LoadWorld for verification)
    int cx, cy, cz;
    file.read(reinterpret_cast<char*>(&cx), sizeof(int));
    file.read(reinterpret_cast<char*>(&cy), sizeof(int));
    file.read(reinterpret_cast<char*>(&cz), sizeof(int));
    
    // Read block data
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                uint8_t block;
                file.read(reinterpret_cast<char*>(&block), sizeof(uint8_t));
                chunk.SetBlock(x, y, z, static_cast<BlockTypeID>(block));
            }
        }
    }
    
    chunk.loaded = true;
    chunk.dirty = false;
    return true;
}

} // namespace vge