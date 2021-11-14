#include "kernelUtil.h"

KernelInfo kernel_util_kernel_info;
PageTableManager kernel_util_page_table_manager = NULL;

// Prepare the memory map
void prepare_memory(BootInfo* boot_info){

    // Count how many entries memory map has
    uint64_t memory_map_entries = boot_info->memory_map_size / boot_info->memory_map_descriptor_size;

    // Initialize the page frame allocator
    GlobalAllocator = PageFrameAllocator();
    GlobalAllocator.ReadEFIMemoryMap(boot_info->memory_map, boot_info->memory_map_size, boot_info->memory_map_descriptor_size);

    // Calculate how many pages the kernel needs
    uint64_t kernel_size = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
    uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;

    // Lock that many pages starting from kernel start
    GlobalAllocator.LockPages(&_KernelStart, kernel_pages);

    // Initialize the PML4 memory table and manager
    PageTable* PML4 = (PageTable*)GlobalAllocator.RequestPage();
    memset(PML4, 0, 0x1000);
    kernel_util_page_table_manager = PageTableManager(PML4);

    // Map all memory from virtual = physical
    for (uint64_t t = 0; t < get_memory_size(boot_info->memory_map, memory_map_entries, boot_info->memory_map_descriptor_size); t+= 0x1000){
        kernel_util_page_table_manager.MapMemory((void*)t, (void*)t);
    }

    // Also map the framebuffer
    uint64_t fb_base = (uint64_t)boot_info->framebuffer->base_address;
    uint64_t fb_size = (uint64_t)boot_info->framebuffer->buffer_size + 0x1000;
    GlobalAllocator.LockPages((void*)fb_base, fb_size/ 0x1000 + 1);
    for (uint64_t t = fb_base; t < fb_base + fb_size; t += 4096){
        kernel_util_page_table_manager.MapMemory((void*)t, (void*)t);
    }

    // Tell the cpu to use the Memory map we created
    asm ("mov %0, %%cr3" : : "r" (PML4));

    kernel_util_kernel_info.page_table_manager = &kernel_util_page_table_manager;
}

// Everything we need to do to get the kernel basic functionality
KernelInfo InitializeKernel(BootInfo* boot_info){

    // Prepare the memory map and manager
    prepare_memory(boot_info);

    // Set the entire draw buffer to black
    memset(boot_info->framebuffer->base_address, 0, boot_info->framebuffer->buffer_size);

    // Return the kernel info
    return kernel_util_kernel_info;
}