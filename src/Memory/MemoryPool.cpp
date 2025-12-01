#include "Memory/MemoryPool.h"

MemoryPool::MemoryPool(int slotSize, int slotCount)
    : slotSize(slotSize), slotCount(slotCount), usedCount(0) {
    // Allocate one large contiguous block of memory
    memory = new char[slotSize * slotCount];
    // Track which slots are used
    used = new bool[slotCount];
    std::memset(used, 0, slotCount * sizeof(bool));
}

MemoryPool::~MemoryPool() {
    delete[] memory;
    delete[] used;
}

int MemoryPool::alloc() {
    // First-fit: find the first free slot
    for (int i = 0; i < slotCount; i++) {
        if (!used[i]) {
            used[i] = true;
            usedCount++;
            return i;  // Return the slot ID
        }
    }
    return -1; // Pool is full
}

void MemoryPool::freeSlot(int id) {
    // Bounds checking and ensure it was actually used
    if (id >= 0 && id < slotCount && used[id]) {
        used[id] = false;
        usedCount--;
    }
}

void* MemoryPool::getPtr(int id) {
    // Convert slot ID to memory address
    return memory + (id * slotSize);
}

int MemoryPool::getSlot(void* ptr) {
    return ((char*)ptr - memory) / slotSize;
}

float MemoryPool::getUsagePercent() const {
    if (slotCount == 0) {
        return 0.0f;
    }
    // Calculate percentage of pool in use
    return (float)usedCount / (float)slotCount * 100.0f;
}