#include "kernelUtil.h"

extern "C" void _start(BootInfo* boot_info){

    KernelInfo kernel_info = InitializeKernel(boot_info);
    PageTableManager* page_table_manager = kernel_info.page_table_manager;
    BasicRenderer renderer = BasicRenderer(boot_info->framebuffer, boot_info->psf1_font); 

    renderer.print("Kernel Initialized Successfully\n");
    renderer.print("KB free: ");
    renderer.print(to_string(GlobalAllocator.GetFreeRAM() / 1000));
    renderer.print("\n");
    renderer.print("KB used: ");
    renderer.print(to_string(GlobalAllocator.GetUsedRAM() / 1000));
    renderer.print("\n");
    renderer.print("KB reserved: ");
    renderer.print(to_string(GlobalAllocator.GetReservedRAM() / 1000));
    renderer.print("\n");

    page_table_manager->map_memory((void*)0x600000000, (void*)0x80000);

    uint64_t* test = (uint64_t*)0x600000000;
    *test = 26;

    renderer.print(to_string(*test));

    while(true);
}