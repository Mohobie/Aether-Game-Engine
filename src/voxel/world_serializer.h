#pragma once

#include "voxel/chunk.h"
#include "voxel/world.h"
#include "math/vec3.h"
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace vge {

// ============================================
// Binary Save Format Constants
// ============================================
constexpr uint32_t WORLD_MAGIC = 0x56474557; // "VGEW" (Voxel Game Engine World)
constexpr uint32_t WORLD_VERSION = 1;

// ============================================
// File Header
// ============================================
struct WorldFileHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t worldSeed;
    float playerPosX, playerPosY, playerPosZ;
    float playerRotX, playerRotY;
    int32_t timeOfDay;
    int32_t dayCount;
    uint32_t chunkTableOffset;
    uint32_t chunkCount;
    uint32_t reserved[4];
};

// ============================================
// Chunk Table Entry
// ============================================
struct ChunkTableEntry {
    int32_t chunkX;
    int32_t chunkY;
    int32_t chunkZ;
    uint32_t dataOffset;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint32_t checksum;
};

// ============================================
// WorldSerializer
// ============================================
class WorldSerializer {
public:
    WorldSerializer();
    ~WorldSerializer();

    // Serialize entire world to binary file
    bool SerializeWorld(const World& world, const std::string& path,
                        const Vec3& playerPos, const Vec3& playerRot,
                        int timeOfDay, int dayCount);

    // Deserialize entire world from binary file
    bool DeserializeWorld(World& world, const std::string& path,
                          Vec3& outPlayerPos, Vec3& outPlayerRot,
                          int& outTimeOfDay, int& outDayCount);

    // Serialize single chunk to byte buffer
    std::vector<uint8_t> SerializeChunk(const Chunk& chunk);

    // Deserialize single chunk from byte buffer
    bool DeserializeChunk(Chunk& chunk, const std::vector<uint8_t>& data);

    // Chunk loading on demand
    void SetLoadDistance(int distance) { loadDistance = distance; }
    int GetLoadDistance() const { return loadDistance; }

    // Load chunks around a position, unload distant ones
    void UpdateChunkLoading(World& world, const Vec3& playerPos);

    // Check if a chunk file exists
    bool HasChunkFile(int cx, int cy, int cz) const;

    // Get list of saved chunk coordinates
    std::vector<ChunkCoord> GetSavedChunks() const;

    // Callbacks
    std::function<void(int, int, int)> onChunkLoaded;
    std::function<void(int, int, int)> onChunkUnloaded;

private:
    std::string basePath;
    int loadDistance;
    std::map<ChunkCoord, bool> loadedChunks;

    // Compression
    std::vector<uint8_t> CompressData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& data,
                                         uint32_t uncompressedSize);

    // Simple RLE compression for block data
    std::vector<uint8_t> CompressBlockData(const std::vector<uint8_t>& blockData);
    std::vector<uint8_t> DecompressBlockData(const std::vector<uint8_t>& compressedData,
                                              uint32_t expectedSize);

    // Checksum
    uint32_t ComputeChecksum(const std::vector<uint8_t>& data);

    // Chunk file path
    std::string GetChunkFilePath(int cx, int cy, int cz) const;
};

} // namespace vge
