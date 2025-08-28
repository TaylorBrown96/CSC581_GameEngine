#ifndef __MEM_H
#define __MEM_H

typedef struct Allocator {
    size_t capacity;
    uint8_t* base;
    size_t top_i;
} Allocator;

Allocator* createAllocator(size_t capacity);

void* push(Allocator* mem, size_t size);

int resize(Allocator* mem);

#endif