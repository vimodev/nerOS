#include "utility/kernelUtil.h"
#include "scheduling/pit/pit.h"

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

    for (int t = 0; t < 200; t++) {
        GlobalRenderer->put_char('g');
        PIT::sleep(100);
    }

    while(true);
}