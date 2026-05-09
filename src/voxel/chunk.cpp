#include "chunk.h"
#include <cstring>

namespace vge {

Chunk::Chunk() : chunkX(0), chunkY(0), chunkZ(0), dirty(true), loaded(false), modified(false) {
    blocks.fill(BlockType::Air);
    lightLevels.fill(0);
}

Chunk::Chunk(int cx, int cy, int cz) 
    : chunkX(cx), chunkY(cy), chunkZ(cz), dirty(true), loaded(false), modified(false) {
    blocks.fill(BlockType::Air);
    lightLevels.fill(0);
}

Chunk::~Chunk() {}

BlockType Chunk::GetBlock(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return BlockType::Air;
    }
    return blocks[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE];
}

void Chunk::SetBlock(int x, int y, int z, BlockType type) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return;
    }
    int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
    if (blocks[index] != type) {
        blocks[index] = type;
        modified = true;
        dirty = true;
    }
}

int Chunk::GetLightLevel(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return 0;
    return lightLevels[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE];
}

void Chunk::SetLightLevel(int x, int y, int z, int level) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return;
    lightLevels[x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE] = (uint8_t)level;
}

bool Chunk::IsEmpty() const {
    for (const auto& block : blocks) {
        if (block != BlockType::Air) return false;
    }
    return true;
}

Vec3 Chunk::GetWorldPosition() const {
    return Vec3(
        static_cast<float>(chunkX * CHUNK_SIZE),
        static_cast<float>(chunkY * CHUNK_SIZE),
        static_cast<float>(chunkZ * CHUNK_SIZE)
    );
}

} // namespace vge
