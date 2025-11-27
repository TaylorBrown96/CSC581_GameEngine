#ifndef CUSTOMALLOCATOR_HPP
#define CUSTOMALLOCATOR_HPP

#include <cstring>

/**
 * CustomAllocator: A fixed-size memory pool allocator
 */
class MemoryPool {
public:
    /**
     * Constructor: Allocates the entire memory pool upfront
     * @param slotSize - Size in bytes of each slot
     * @param slotCount - Total number of slots to allocate
     */
    MemoryPool(int slotSize, int slotCount);
    
    /**
     * Destructor: Frees the memory pool
     */
    ~MemoryPool();

    // Prevent copying
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    
    int alloc();

  
    void freeSlot(int id);

    
    void* getPtr(int id);

    int getSlot(void* ptr);
   
    int getUsedCount() const { return usedCount; }
    int getTotalCount() const { return slotCount; }
    float getUsagePercent() const;

private:
    int slotSize;
    int slotCount;
    char* memory;
    bool* used;
    int usedCount; // Efficiently track usage
};

#endif // CUSTOMALLOCATOR_HPP