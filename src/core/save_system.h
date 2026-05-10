#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include "voxel/world.h"
#include "voxel/chunk.h"
#include "voxel/block_types.h"

namespace vge {

constexpr uint32_t SAVE_MAGIC = 0x564F5845; // "VOXE"
constexpr uint32_t SAVE_VERSION = 2; // Updated for string-based blocks

struct SaveHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t chunkCount;
    uint32_t blockPaletteSize; // Number of unique block types
    uint32_t reserved[4]; // Padding for future use
};

struct BlockPaletteEntry {
    char blockId[64]; // String ID of block
    uint16_t typeId;  // Runtime type ID
};

struct ChunkData {
    int chunkX, chunkY, chunkZ;
    uint16_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // BlockTypeID storage
};

class SaveSystem {
public:
    static bool SaveWorld(const World& world, const std::string& path);
    static bool LoadWorld(World& world, const std::string& path);
    static bool SaveChunk(const Chunk& chunk, std::ofstream& file);
    static bool LoadChunk(Chunk& chunk, std::ifstream& file);
private:
    static bool WriteBlockPalette(std::ofstream& file, const World& world);
    static bool ReadBlockPalette(std::ifstream& file, World& world);
};

} // namespace vge