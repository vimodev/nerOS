#include "memory.h"

// Get the system's memory size in bytes
uint64_t get_memory_size(EFI_MEMORY_DESCRIPTOR* memory_map, uint64_t memory_map_entries, uint64_t memory_map_descriptor_size){

    // Use a static variable to cache the result
    static uint64_t memory_size = 0;
    if (memory_size > 0) return memory_size;

    // Otherwise loop over all memory map entries, count their pages and multiply by 0x1000
    for (int i = 0; i < memory_map_entries; i++){
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memory_map + (i * memory_map_descriptor_size));
        memory_size += desc->numPages * 4096;
    }

    return memory_size;
}

// Set memory to value for num bytes
void memset(void* start, uint8_t value, uint64_t num){
    for (uint64_t i = 0; i < num; i++){
        *(uint8_t*)((uint64_t)start + i) = value;
    }
}