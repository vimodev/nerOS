#include "utility/kernelUtil.h"

#include "scheduling/pit/pit.h"
#include "graphics/BasicRenderer.h"
#include "paging/PageFrameAllocator.h"
#include "utility/cstr.h"

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

    while(true) {
        asm ("hlt");
    }
}