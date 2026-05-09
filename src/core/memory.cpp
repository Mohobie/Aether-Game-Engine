#include "memory.h"
#include <cstdlib>
namespace aether {
PoolAllocator::PoolAllocator(size_t objectSize, size_t objectCount) : objectSize(std::max(objectSize, sizeof(FreeNode))), freeList(nullptr) {
    size_t blockSize = this->objectSize * objectCount;
    void* block = std::malloc(blockSize);
    blocks.push_back(block);
    uint8_t* ptr = static_cast<uint8_t*>(block);
    for (size_t i=0; i<objectCount; ++i) {
        FreeNode* node = reinterpret_cast<FreeNode*>(ptr + i*this->objectSize);
        node->next = freeList;
        freeList = node;
    }
}
PoolAllocator::~PoolAllocator() { for (void* block : blocks) std::free(block); }
void* PoolAllocator::allocate() { if (!freeList) return nullptr; FreeNode* node = freeList; freeList = node->next; return node; }
void PoolAllocator::deallocate(void* ptr) { if (!ptr) return; FreeNode* node = static_cast<FreeNode*>(ptr); node->next = freeList; freeList = node; }
} // namespace aether
