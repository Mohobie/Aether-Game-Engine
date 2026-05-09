#include "chunk.h"
#include <cstring>

namespace vge {

Chunk::Chunk(int cx, int cy, int cz) 
    : chunkX(cx), chunkY(cy), chunkZ(cz), modified(false) {
    std::memset(blocks, 0, sizeof(blocks));
}

Chunk::~Chunk() {}

BlockType Chunk::GetBlock(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return BlockType::Air;
    }
    return blocks[x][y][z];
}

void Chunk::SetBlock(int x, int y, int z, BlockType type) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return;
    }
    if (blocks[x][y][z] != type) {
        blocks[x][y][z] = type;
        modified = true;
    }
}

bool Chunk::IsEmpty() const {
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                if (blocks[x][y][z] != BlockType::Air) {
                    return false;
                }
            }
        }
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
