#include "utility/kernelUtil.h"

void print_memory_summary() {
    GlobalRenderer->println("Kernel Initialized Successfully");
    GlobalRenderer->print("KB free: ");
    GlobalRenderer->println(to_string(GlobalAllocator.get_free_ram() / 1000));
    GlobalRenderer->print("KB used: ");
    GlobalRenderer->println(to_string(GlobalAllocator.get_used_ram() / 1000));
    GlobalRenderer->print("KB reserved: ");
    GlobalRenderer->println(to_string(GlobalAllocator.get_reserved_ram() / 1000));
}

extern "C" void _start(BootInfo* boot_info){

    KernelInfo kernel_info = initialize_kernel(boot_info);

    print_memory_summary();

    GlobalRenderer->println(to_hstring((uint64_t) malloc(0x8000)));
    void *address = malloc(0x8000);
    GlobalRenderer->println(to_hstring((uint64_t) address));
    GlobalRenderer->println(to_hstring((uint64_t) malloc(0x8000)));
    free(address);
    GlobalRenderer->println(to_hstring((uint64_t) malloc(0x8000)));

    while(true);
}