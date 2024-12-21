#include "memory_object.h"
#include "memory_pool.h"

#define USE_MEMORY_POOL 1

#if USE_MEMORY_POOL
MemoryPool *g_memoryPool32 = new MemoryPool(32, 128);  // 4k
MemoryPool *g_memoryPool64 = new MemoryPool(64, 128);  // 8k
MemoryPool *g_memoryPool128 = new MemoryPool(128, 64); // 8k
MemoryPool *g_memoryPool256 = new MemoryPool(256, 32); // 8k
MemoryPool *g_memoryPool512 = new MemoryPool(512, 16); // 8k

#define SELECT_MEMORY_POOL(mallocSize) \
    int blockSize = mallocSize + sizeof(size_t);                   \
    MemoryPool *memoryPool = nullptr;                              \
    if (blockSize <= 32) { memoryPool = g_memoryPool32;}           \
    else if (blockSize <= 64) { memoryPool = g_memoryPool64; }     \
    else if (blockSize <= 128) { memoryPool = g_memoryPool128; }   \
    else if (blockSize <= 256) { memoryPool = g_memoryPool256; }   \
    else { memoryPool = g_memoryPool512; }
    
#endif

void *MemoryObject::operator new(size_t mallocSize)
{
#if USE_MEMORY_POOL
    SELECT_MEMORY_POOL(mallocSize)
    assert(memoryPool);
    void* ptr = memoryPool->allocate(mallocSize);
    return ptr;
#else
    return malloc(mallocSize);
#endif
}
void *MemoryObject::operator new[](size_t mallocSize)
{
    return malloc(mallocSize);
}
void *MemoryObject::operator new(size_t mallocSize, const char *file, int line)
{
#if USE_MEMORY_POOL
    SELECT_MEMORY_POOL(mallocSize)
    void *ptr = memoryPool->allocate(mallocSize);
#if MEMORY_TRACK
    // MemoryLeckDetector::Instance()->AllocateMemory(file, line, mallocSize, (size_t)ptr);
#endif
    return ptr;
#else
    return malloc(mallocSize);
#endif
}
void *MemoryObject::operator new[](size_t mallocSize, const char *file, int line)
{
    return malloc(mallocSize);
}
void MemoryObject::operator delete(void *ptr)
{
#if USE_MEMORY_POOL
#if MEMORY_TRACK
    MemoryLeckDetector::Instance()->DellocateMemory((size_t)ptr);
#endif
    void* blockPtr = (char*)ptr - sizeof(size_t);
    size_t mallocSize = *(static_cast<char*>(blockPtr));
    SELECT_MEMORY_POOL(mallocSize)
    memoryPool->deallocate(ptr);
#else
    free(ptr);
#endif
}
void MemoryObject::operator delete[](void *ptr)
{
    free(ptr);
}