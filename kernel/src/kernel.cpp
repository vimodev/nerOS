#include "utility/kernelUtil.h"

extern "C" void _start(BootInfo* boot_info){

    KernelInfo kernel_info = initialize_kernel(boot_info);
    PageTableManager* page_table_manager = kernel_info.page_table_manager;

    GlobalRenderer->print("Kernel Initialized Successfully\n");
    GlobalRenderer->print("KB free: ");
    GlobalRenderer->print(to_string(GlobalAllocator.get_free_ram() / 1000));
    GlobalRenderer->print("\n");
    GlobalRenderer->print("KB used: ");
    GlobalRenderer->print(to_string(GlobalAllocator.get_used_ram() / 1000));
    GlobalRenderer->print("\n");
    GlobalRenderer->print("KB reserved: ");
    GlobalRenderer->print(to_string(GlobalAllocator.get_reserved_ram() / 1000));
    GlobalRenderer->print("\n");

    while(true);
}