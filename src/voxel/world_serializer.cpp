#include "world_serializer.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>

namespace vge {

WorldSerializer::WorldSerializer() : loadDistance(4) {}

WorldSerializer::~WorldSerializer() {}

bool WorldSerializer::SerializeWorld(const World& world, const std::string& path,
                                     const Vec3& playerPos, const Vec3& playerRot,
                                     int timeOfDay, int dayCount) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[WorldSerializer] Failed to open: " << path << std::endl;
        return false;
    }

    // Build chunk table
    std::vector<ChunkTableEntry> chunkTable;
    std::vector<std::vector<uint8_t>> chunkDataList;

    // Collect all loaded chunks
    for (int x = -50; x <= 50; ++x) {
        for (int y = -10; y <= 10; ++y) {
            for (int z = -50; z <= 50; ++z) {
                Chunk* chunk = const_cast<World&>(world).GetChunk(x, y, z);
                if (chunk && chunk->loaded && !chunk->IsEmpty()) {
                    auto data = SerializeChunk(*chunk);
                    auto compressed = CompressData(data);

                    ChunkTableEntry entry;
                    entry.chunkX = x;
                    entry.chunkY = y;
                    entry.chunkZ = z;
                    entry.compressedSize = static_cast<uint32_t>(compressed.size());
                    entry.uncompressedSize = static_cast<uint32_t>(data.size());
                    entry.checksum = ComputeChecksum(compressed);

                    chunkTable.push_back(entry);
                    chunkDataList.push_back(std::move(compressed));
                }
            }
        }
    }

    // Calculate offsets
    uint32_t headerSize = sizeof(WorldFileHeader);
    uint32_t tableSize = static_cast<uint32_t>(chunkTable.size() * sizeof(ChunkTableEntry));
    uint32_t dataOffset = headerSize + tableSize;

    // Update offsets in table
    uint32_t currentOffset = dataOffset;
    for (size_t i = 0; i < chunkTable.size(); ++i) {
        chunkTable[i].dataOffset = currentOffset;
        currentOffset += chunkTable[i].compressedSize;
    }

    // Write header
    WorldFileHeader header;
    header.magic = WORLD_MAGIC;
    header.version = WORLD_VERSION;
    header.worldSeed = static_cast<uint32_t>(world.GetSeed());
    header.playerPosX = playerPos.x;
    header.playerPosY = playerPos.y;
    header.playerPosZ = playerPos.z;
    header.playerRotX = playerRot.x;
    header.playerRotY = playerRot.y;
    header.timeOfDay = timeOfDay;
    header.dayCount = dayCount;
    header.chunkTableOffset = headerSize;
    header.chunkCount = static_cast<uint32_t>(chunkTable.size());
    std::memset(header.reserved, 0, sizeof(header.reserved));

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Write chunk table
    for (const auto& entry : chunkTable) {
        file.write(reinterpret_cast<const char*>(&entry), sizeof(entry));
    }

    // Write chunk data
    for (const auto& data : chunkDataList) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    file.close();
    std::cout << "[WorldSerializer] World saved: " << path
              << " (" << chunkTable.size() << " chunks, "
              << currentOffset << " bytes)" << std::endl;
    return true;
}

bool WorldSerializer::DeserializeWorld(World& world, const std::string& path,
                                       Vec3& outPlayerPos, Vec3& outPlayerRot,
                                       int& outTimeOfDay, int& outDayCount) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[WorldSerializer] Failed to open: " << path << std::endl;
        return false;
    }

    // Read header
    WorldFileHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.magic != WORLD_MAGIC) {
        std::cerr << "[WorldSerializer] Invalid magic number" << std::endl;
        return false;
    }

    if (header.version != WORLD_VERSION) {
        std::cerr << "[WorldSerializer] Version mismatch: " << header.version
                  << " vs " << WORLD_VERSION << std::endl;
        return false;
    }

    // Extract player data
    outPlayerPos = Vec3(header.playerPosX, header.playerPosY, header.playerPosZ);
    outPlayerRot = Vec3(header.playerRotX, header.playerRotY, 0.0f);
    outTimeOfDay = header.timeOfDay;
    outDayCount = header.dayCount;
    world.SetSeed(static_cast<int>(header.worldSeed));

    // Read chunk table
    std::vector<ChunkTableEntry> chunkTable(header.chunkCount);
    for (uint32_t i = 0; i < header.chunkCount; ++i) {
        file.read(reinterpret_cast<char*>(&chunkTable[i]), sizeof(ChunkTableEntry));
    }

    // Load each chunk
    uint32_t loadedChunks = 0;
    for (const auto& entry : chunkTable) {
        // Seek to data
        file.seekg(entry.dataOffset, std::ios::beg);

        // Read compressed data
        std::vector<uint8_t> compressed(entry.compressedSize);
        file.read(reinterpret_cast<char*>(compressed.data()), entry.compressedSize);

        // Verify checksum
        uint32_t checksum = ComputeChecksum(compressed);
        if (checksum != entry.checksum) {
            std::cerr << "[WorldSerializer] Checksum mismatch for chunk ("
                      << entry.chunkX << ", " << entry.chunkY << ", " << entry.chunkZ << ")" << std::endl;
            continue;
        }

        // Decompress
        auto data = DecompressData(compressed, entry.uncompressedSize);

        // Create and deserialize chunk
        Chunk* chunk = world.GetOrCreateChunk(entry.chunkX, entry.chunkY, entry.chunkZ);
        if (chunk && DeserializeChunk(*chunk, data)) {
            loadedChunks++;
        }
    }

    file.close();
    std::cout << "[WorldSerializer] World loaded: " << path
              << " (" << loadedChunks << "/" << header.chunkCount << " chunks)" << std::endl;
    return true;
}

std::vector<uint8_t> WorldSerializer::SerializeChunk(const Chunk& chunk) {
    std::vector<uint8_t> data;

    // Chunk coordinates
    data.resize(sizeof(int32_t) * 3);
    int32_t cx = chunk.GetChunkX();
    int32_t cy = chunk.GetChunkY();
    int32_t cz = chunk.GetChunkZ();
    std::memcpy(data.data(), &cx, sizeof(int32_t));
    std::memcpy(data.data() + 4, &cy, sizeof(int32_t));
    std::memcpy(data.data() + 8, &cz, sizeof(int32_t));

    // Block data (use RLE for compression)
    std::vector<uint8_t> blockData(CHUNK_VOLUME);
    for (int i = 0; i < CHUNK_VOLUME; ++i) {
        blockData[i] = static_cast<uint8_t>(chunk.blocks[i]);
    }

    auto compressedBlocks = CompressBlockData(blockData);
    uint32_t blockDataSize = static_cast<uint32_t>(compressedBlocks.size());

    // Append block data size and compressed data
    size_t oldSize = data.size();
    data.resize(oldSize + sizeof(uint32_t) + compressedBlocks.size());
    std::memcpy(data.data() + oldSize, &blockDataSize, sizeof(uint32_t));
    std::memcpy(data.data() + oldSize + sizeof(uint32_t), compressedBlocks.data(), compressedBlocks.size());

    // Light levels
    oldSize = data.size();
    data.resize(oldSize + CHUNK_VOLUME);
    std::memcpy(data.data() + oldSize, chunk.lightLevels.data(), CHUNK_VOLUME);

    // Modified flag
    oldSize = data.size();
    data.resize(oldSize + 1);
    data[oldSize] = chunk.modified ? 1 : 0;

    return data;
}

bool WorldSerializer::DeserializeChunk(Chunk& chunk, const std::vector<uint8_t>& data) {
    if (data.size() < 12) return false;

    size_t offset = 0;

    // Read coordinates
    int32_t cx, cy, cz;
    std::memcpy(&cx, data.data() + offset, sizeof(int32_t)); offset += 4;
    std::memcpy(&cy, data.data() + offset, sizeof(int32_t)); offset += 4;
    std::memcpy(&cz, data.data() + offset, sizeof(int32_t)); offset += 4;

    chunk.chunkX = cx;
    chunk.chunkY = cy;
    chunk.chunkZ = cz;

    // Read compressed block data size
    uint32_t blockDataSize;
    std::memcpy(&blockDataSize, data.data() + offset, sizeof(uint32_t)); offset += 4;

    // Decompress block data
    if (offset + blockDataSize > data.size()) return false;
    std::vector<uint8_t> compressedBlocks(data.begin() + offset, data.begin() + offset + blockDataSize);
    offset += blockDataSize;

    auto blockData = DecompressBlockData(compressedBlocks, CHUNK_VOLUME);
    if (blockData.size() != CHUNK_VOLUME) return false;

    for (int i = 0; i < CHUNK_VOLUME; ++i) {
        chunk.blocks[i] = static_cast<BlockTypeID>(blockData[i]);
    }

    // Read light levels
    if (offset + CHUNK_VOLUME > data.size()) return false;
    std::memcpy(chunk.lightLevels.data(), data.data() + offset, CHUNK_VOLUME);
    offset += CHUNK_VOLUME;

    // Read modified flag
    if (offset < data.size()) {
        chunk.modified = (data[offset] != 0);
    }

    chunk.loaded = true;
    chunk.dirty = false;
    return true;
}

void WorldSerializer::UpdateChunkLoading(World& world, const Vec3& playerPos) {
    int playerChunkX = static_cast<int>(playerPos.x) / CHUNK_SIZE;
    int playerChunkY = static_cast<int>(playerPos.y) / CHUNK_SIZE;
    int playerChunkZ = static_cast<int>(playerPos.z) / CHUNK_SIZE;

    // Load nearby chunks
    for (int dx = -loadDistance; dx <= loadDistance; ++dx) {
        for (int dy = -loadDistance; dy <= loadDistance; ++dy) {
            for (int dz = -loadDistance; dz <= loadDistance; ++dz) {
                int cx = playerChunkX + dx;
                int cy = playerChunkY + dy;
                int cz = playerChunkZ + dz;

                ChunkCoord coord{cx, cy, cz};
                if (loadedChunks.find(coord) == loadedChunks.end()) {
                    // Try to load from file
                    std::string chunkPath = GetChunkFilePath(cx, cy, cz);
                    std::ifstream file(chunkPath, std::ios::binary);
                    if (file.is_open()) {
                        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                                                   std::istreambuf_iterator<char>());
                        file.close();

                        Chunk* chunk = world.GetOrCreateChunk(cx, cy, cz);
                        if (chunk && DeserializeChunk(*chunk, data)) {
                            loadedChunks[coord] = true;
                            if (onChunkLoaded) onChunkLoaded(cx, cy, cz);
                        }
                    } else {
                        // Mark as loaded even if not on disk (will be generated)
                        loadedChunks[coord] = true;
                    }
                }
            }
        }
    }

    // Unload distant chunks
    std::vector<ChunkCoord> toUnload;
    for (const auto& pair : loadedChunks) {
        const ChunkCoord& coord = pair.first;
        int dist = std::abs(coord.x - playerChunkX) + std::abs(coord.y - playerChunkY) + std::abs(coord.z - playerChunkZ);
        if (dist > loadDistance * 2) {
            toUnload.push_back(coord);
        }
    }

    for (const auto& coord : toUnload) {
        loadedChunks.erase(coord);
        if (onChunkUnloaded) onChunkUnloaded(coord.x, coord.y, coord.z);
    }
}

bool WorldSerializer::HasChunkFile(int cx, int cy, int cz) const {
    std::ifstream file(GetChunkFilePath(cx, cy, cz));
    return file.good();
}

std::vector<ChunkCoord> WorldSerializer::GetSavedChunks() const {
    std::vector<ChunkCoord> result;
    // This would scan the save directory in a full implementation
    return result;
}

std::vector<uint8_t> WorldSerializer::CompressData(const std::vector<uint8_t>& data) {
    // Simple RLE compression - good for sparse block data
    return CompressBlockData(data);
}

std::vector<uint8_t> WorldSerializer::DecompressData(const std::vector<uint8_t>& data,
                                                      uint32_t uncompressedSize) {
    return DecompressBlockData(data, uncompressedSize);
}

std::vector<uint8_t> WorldSerializer::CompressBlockData(const std::vector<uint8_t>& blockData) {
    std::vector<uint8_t> result;
    if (blockData.empty()) return result;

    uint8_t current = blockData[0];
    uint16_t count = 1;

    for (size_t i = 1; i < blockData.size(); ++i) {
        if (blockData[i] == current && count < 65535) {
            count++;
        } else {
            // Write run: count (2 bytes) + value (1 byte)
            result.push_back(static_cast<uint8_t>(count & 0xFF));
            result.push_back(static_cast<uint8_t>((count >> 8) & 0xFF));
            result.push_back(current);
            current = blockData[i];
            count = 1;
        }
    }

    // Write final run
    result.push_back(static_cast<uint8_t>(count & 0xFF));
    result.push_back(static_cast<uint8_t>((count >> 8) & 0xFF));
    result.push_back(current);

    return result;
}

std::vector<uint8_t> WorldSerializer::DecompressBlockData(const std::vector<uint8_t>& compressedData,
                                                           uint32_t expectedSize) {
    std::vector<uint8_t> result;
    result.reserve(expectedSize);

    size_t offset = 0;
    while (offset + 3 <= compressedData.size() && result.size() < expectedSize) {
        uint16_t count = compressedData[offset] | (compressedData[offset + 1] << 8);
        uint8_t value = compressedData[offset + 2];
        offset += 3;

        for (uint16_t i = 0; i < count && result.size() < expectedSize; ++i) {
            result.push_back(value);
        }
    }

    return result;
}

uint32_t WorldSerializer::ComputeChecksum(const std::vector<uint8_t>& data) {
    // Simple FNV-1a hash
    uint32_t hash = 2166136261u;
    for (uint8_t byte : data) {
        hash ^= byte;
        hash *= 16777619u;
    }
    return hash;
}

std::string WorldSerializer::GetChunkFilePath(int cx, int cy, int cz) const {
    return basePath + "/chunk_" + std::to_string(cx) + "_" + std::to_string(cy) + "_" + std::to_string(cz) + ".bin";
}

} // namespace vge
