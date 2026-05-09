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
    
    // Write header
    SaveHeader header;
    header.magic = SAVE_MAGIC;
    header.version = SAVE_VERSION;
    header.chunkCount = 0; // Would count actual chunks
    
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // TODO: Iterate chunks and write them
    std::cout << "[Save] World saved to: " << path << std::endl;
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
    
    std::cout << "[Save] World loaded from: " << path << " (" << header.chunkCount << " chunks)" << std::endl;
    return true;
}

bool SaveSystem::SaveChunk(const Chunk& chunk, std::ofstream& file) {
    ChunkData data;
    data.chunkX = chunk.GetChunkX();
    data.chunkY = chunk.GetChunkY();
    data.chunkZ = chunk.GetChunkZ();
    
    // Copy block data
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                data.blocks[x][y][z] = static_cast<uint8_t>(chunk.GetBlock(x, y, z));
            }
        }
    }
    
    file.write(reinterpret_cast<const char*>(&data), sizeof(data));
    return true;
}

bool SaveSystem::LoadChunk(Chunk& chunk, std::ifstream& file) {
    ChunkData data;
    file.read(reinterpret_cast<char*>(&data), sizeof(data));
    
    // Verify chunk coordinates match
    if (data.chunkX != chunk.GetChunkX() || 
        data.chunkY != chunk.GetChunkY() || 
        data.chunkZ != chunk.GetChunkZ()) {
        std::cerr << "[Save] Chunk coordinate mismatch" << std::endl;
        return false;
    }
    
    // Copy block data
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                chunk.SetBlock(x, y, z, static_cast<BlockType>(data.blocks[x][y][z]));
            }
        }
    }
    
    return true;
}

} // namespace vge