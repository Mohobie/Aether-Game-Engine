#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include "voxel/world.h"
#include "voxel/chunk.h"

namespace vge {

constexpr uint32_t SAVE_MAGIC = 0x564F5845; // "VOXE"
constexpr uint32_t SAVE_VERSION = 1;

struct SaveHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t chunkCount;
    uint32_t reserved[5]; // Padding for future use
};

struct ChunkData {
    int chunkX, chunkY, chunkZ;
    uint8_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

class SaveSystem {
public:
    static bool SaveWorld(const World& world, const std::string& path);
    static bool LoadWorld(World& world, const std::string& path);
    static bool SaveChunk(const Chunk& chunk, std::ofstream& file);
    static bool LoadChunk(Chunk& chunk, std::ifstream& file);
};

} // namespace vge