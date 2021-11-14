#include "kernelUtil.h"

KernelInfo kernelInfo;
PageTableManager pageTableManager = NULL;
void PrepareMemory(BootInfo* bootInfo){
    uint64_t mMapEntries = bootInfo->memory_map_size / bootInfo->memory_map_descriptor_size;

    GlobalAllocator = PageFrameAllocator();
    GlobalAllocator.ReadEFIMemoryMap(bootInfo->memory_map, bootInfo->memory_map_size, bootInfo->memory_map_descriptor_size);

    uint64_t kernelSize = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
    uint64_t kernelPages = (uint64_t)kernelSize / 4096 + 1;

    GlobalAllocator.LockPages(&_KernelStart, kernelPages);

    PageTable* PML4 = (PageTable*)GlobalAllocator.RequestPage();
    memset(PML4, 0, 0x1000);

    pageTableManager = PageTableManager(PML4);

    for (uint64_t t = 0; t < GetMemorySize(bootInfo->memory_map, mMapEntries, bootInfo->memory_map_descriptor_size); t+= 0x1000){
        pageTableManager.MapMemory((void*)t, (void*)t);
    }

    uint64_t fbBase = (uint64_t)bootInfo->framebuffer->base_address;
    uint64_t fbSize = (uint64_t)bootInfo->framebuffer->buffer_size + 0x1000;
    GlobalAllocator.LockPages((void*)fbBase, fbSize/ 0x1000 + 1);
    for (uint64_t t = fbBase; t < fbBase + fbSize; t += 4096){
        pageTableManager.MapMemory((void*)t, (void*)t);
    }

    asm ("mov %0, %%cr3" : : "r" (PML4));

    kernelInfo.pageTableManager = &pageTableManager;
}


KernelInfo InitializeKernel(BootInfo* bootInfo){

    PrepareMemory(bootInfo);

    memset(bootInfo->framebuffer->base_address, 0, bootInfo->framebuffer->buffer_size);

    return kernelInfo;
}