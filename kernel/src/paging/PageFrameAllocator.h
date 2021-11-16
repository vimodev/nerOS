#pragma once
#include "../efiMemory.h"
#include <stdint.h>
#include "../utility/Bitmap.h"
#include "../memory.h"

class PageFrameAllocator {
    public:
    void read_efi_memory_map(EFI_MEMORY_DESCRIPTOR* memory_map, size_t memory_map_size, size_t memory_map_descriptor_size);
    Bitmap page_bitmap;
    void free_page(void* address);
    void free_pages(void* address, uint64_t pageCount);
    void lock_page(void* address);
    void lock_pages(void* address, uint64_t pageCount);
    void* request_page();
    uint64_t get_free_ram();
    uint64_t get_used_ram();
    uint64_t get_reserved_ram();


    private:
    void init_bitmap(size_t bitmap_size, void* buffer_address);
    void reserve_page(void* address);
    void reserve_pages(void* address, uint64_t pageCount);
    void unreserve_page(void* address);
    void unreserve_pages(void* address, uint64_t pageCount);

};

extern PageFrameAllocator GlobalAllocator;