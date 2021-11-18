#include "PageFrameAllocator.h"

#include "../memory/memory.h"

// Variables are kept up to date to track system memory status
uint64_t free_memory;
uint64_t reserved_memory;
uint64_t used_memory;

bool allocator_initialized = false;

// Globally addressable page frame allocator
PageFrameAllocator GlobalAllocator;

// Initialize allocator from info from efi memory map
void PageFrameAllocator::read_efi_memory_map(EFI_MEMORY_DESCRIPTOR* memory_map, size_t memory_map_size, size_t memory_map_descriptor_size) {
    // Only initialize once
    if (allocator_initialized) return;
    allocator_initialized = true;

    // How many entries are in the map?
    uint64_t memory_map_entries = memory_map_size / memory_map_descriptor_size;

    // Find the largest free memory segment
    void* largest_free_segment = NULL;
    size_t largest_free_size = 0;
    for (int i = 0; i < memory_map_entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memory_map + (i * memory_map_descriptor_size));
        // Only consider EfiConventionalMemory type
        if (desc->type == 7) { // type = EfiConventionalMemory
            if (desc->numPages * 0x1000 > largest_free_size) {
                largest_free_segment = desc->physAddr;
                largest_free_size = desc->numPages * 0x1000;
            }
        }
    }

    // initialize the bitmap
    uint64_t memory_size = get_memory_size(memory_map, memory_map_entries, memory_map_descriptor_size);
    free_memory = memory_size;
    uint64_t bitmap_size = memory_size / 0x1000 / 8 + 1;
    init_bitmap(bitmap_size, largest_free_segment);

    // Reserve all the UEFI memory
    reserve_pages(0, memory_size / 0x1000 + 1);

    // For any usable conventional memory, unreserve those pages
    for (int i = 0; i < memory_map_entries; i++){
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memory_map + (i * memory_map_descriptor_size));
        if (desc->type == 7){ // efiConventionalMemory
            unreserve_pages(desc->physAddr, desc->numPages);
        }
    }

    // Reserve first 0x100000 addresses (first 0x100 pages), some systems have BIOS stuff here
    reserve_pages(0, 0x100);
    // Lock the bitmap buffer pages
    lock_pages(page_bitmap.buffer, page_bitmap.size / 0x1000 + 1);
}

// Create the bitmap
void PageFrameAllocator::init_bitmap(size_t bitmap_size, void* buffer_address){
    page_bitmap.size = bitmap_size;
    page_bitmap.buffer = (uint8_t*)buffer_address;
    for (int i = 0; i < bitmap_size; i++){
        *(uint8_t*)(page_bitmap.buffer + i) = 0;
    }
}

// Give the requestor a page of memory
uint64_t page_bitmap_index = 0;
void* PageFrameAllocator::request_page(){
    // Go over all pages and return first free one
    for (; page_bitmap_index < page_bitmap.size * 8; page_bitmap_index++){
        if (page_bitmap[page_bitmap_index] == true) continue;
        lock_page((void*)(page_bitmap_index * 0x1000));
        return (void*)(page_bitmap_index * 0x1000);
    }
    return NULL; // Page Frame Swap to file TODO
}

// Free the page with the given address
void PageFrameAllocator::free_page(void* address){
    // Round down to page number
    uint64_t index = (uint64_t)address / 0x1000;
    if (page_bitmap[index] == false) return;
    if (page_bitmap.set(index, false)){
        free_memory += 0x1000;
        used_memory -= 0x1000;
        // Update efficient search index
        if (page_bitmap_index > index) page_bitmap_index = index;
    }
}

// Free multiple pages
void PageFrameAllocator::free_pages(void* address, uint64_t pageCount){
    for (int t = 0; t < pageCount; t++){
        free_page((void*)((uint64_t)address + (t * 0x1000)));
    }
}

// Mark the given page as locked
void PageFrameAllocator::lock_page(void* address){
    uint64_t index = (uint64_t)address / 0x1000;
    if (page_bitmap[index] == true) return;
    if (page_bitmap.set(index, true)){
        free_memory -= 0x1000;
        used_memory += 0x1000;
    }
}

// Lock multiple pages
void PageFrameAllocator::lock_pages(void* address, uint64_t pageCount){
    for (int t = 0; t < pageCount; t++){
        lock_page((void*)((uint64_t)address + (t * 0x1000)));
    }
}

// Undo the page reservation of address
void PageFrameAllocator::unreserve_page(void* address){
    uint64_t index = (uint64_t)address / 0x1000;
    if (page_bitmap[index] == false) return;
    if (page_bitmap.set(index, false)){
        free_memory += 0x1000;
        reserved_memory -= 0x1000;
        if (page_bitmap_index > index) page_bitmap_index = index;
    }
}

// Unreserve multiple pages
void PageFrameAllocator::unreserve_pages(void* address, uint64_t pageCount){
    for (int t = 0; t < pageCount; t++){
        unreserve_page((void*)((uint64_t)address + (t * 0x1000)));
    }
}

// Mark the given address' page as reserved
void PageFrameAllocator::reserve_page(void* address){
    uint64_t index = (uint64_t)address / 0x1000;
    if (page_bitmap[index] == true) return;
    if (page_bitmap.set(index, true)){
        free_memory -= 0x1000;
        reserved_memory += 0x1000;
    }
}

// Reserve multiple pages
void PageFrameAllocator::reserve_pages(void* address, uint64_t pageCount){
    for (int t = 0; t < pageCount; t++){
        reserve_page((void*)((uint64_t)address + (t * 0x1000)));
    }
}

uint64_t PageFrameAllocator::get_free_ram(){
    return free_memory;
}
uint64_t PageFrameAllocator::get_used_ram(){
    return used_memory;
}
uint64_t PageFrameAllocator::get_reserved_ram(){
    return reserved_memory;
}