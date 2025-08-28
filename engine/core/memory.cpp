#include <cstdlib>
#include <memory.hpp>

Allocator* createAllocator(size_t capacity) {
    Allocator* a = (Allocator*)malloc(sizeof(Allocator));
    a->base = (uint8_t)malloc(sizeof(uint8_t) * capacity);
    a->top_i = 0;
    return a;
}

// simple stack based allocaor for now
void* push(Allocator* mem, size_t size) {
    mem->top_i += size;
    return (void*)&(mem->base[mem->top_i - size]);
}

int resize(Allocator* mem) {
    // TODO add resize mechanism when allocator runs out of memory
}

void destroyAllocator(Allocator* mem) {
    free(mem->base);
    free(mem);
}