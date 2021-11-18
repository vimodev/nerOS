#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../paging/PageTableManager.h"
#include "../paging/PageFrameAllocator.h"

struct HeapSegmentHeader {
    size_t length;
    HeapSegmentHeader *next;
    HeapSegmentHeader *last;
    bool free;
    void combine_forward();
    void combine_backward();
    HeapSegmentHeader *split(size_t split_length);
};

void initialize_heap(void *heap_address, size_t page_count);
void expand_heap(size_t length);

void *malloc(size_t size);
void free(void *address);

inline void *operator new(size_t size) { return malloc(size); }
inline void *operator new[](size_t size) { return malloc(size); }

inline void operator delete(void *ptr) { free(ptr); }