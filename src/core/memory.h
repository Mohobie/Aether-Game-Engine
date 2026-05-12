#pragma once
#include "types.h"
#include <vector>
#include <cstddef>
namespace vge {
class PoolAllocator {
public:
    PoolAllocator(size_t objectSize, size_t objectCount);
    ~PoolAllocator();
    void* allocate();
    void deallocate(void* ptr);
private:
    struct FreeNode { FreeNode* next; };
    size_t objectSize;
    FreeNode* freeList;
    std::vector<void*> blocks;
};
} // namespace vge
