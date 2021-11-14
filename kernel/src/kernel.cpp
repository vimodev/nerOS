#include "kernelUtil.h"

extern "C" void _start(BootInfo* boot_info){

    KernelInfo kernel_info = InitializeKernel(boot_info);
    PageTableManager* page_table_manager = kernel_info.page_table_manager;
    BasicRenderer renderer = BasicRenderer(boot_info->framebuffer, boot_info->psf1_font); 

    renderer.Print("Kernel Initialized Successfully\n");
    renderer.Print("KB free: ");
    renderer.Print(to_string(GlobalAllocator.GetFreeRAM() / 1000));
    renderer.Print("\n");
    renderer.Print("KB used: ");
    renderer.Print(to_string(GlobalAllocator.GetUsedRAM() / 1000));
    renderer.Print("\n");
    renderer.Print("KB reserved: ");
    renderer.Print(to_string(GlobalAllocator.GetReservedRAM() / 1000));
    renderer.Print("\n");

    while(true);
}