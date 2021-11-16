#include "kernelUtil.h"

BasicRenderer renderer = BasicRenderer(NULL, NULL);
KernelInfo kernel_util_kernel_info;
PageTableManager kernel_util_page_table_manager = NULL;
IDTR idtr;

// Prepare the memory map
void prepare_memory(BootInfo* boot_info){

    // Count how many entries memory map has
    uint64_t memory_map_entries = boot_info->memory_map_size / boot_info->memory_map_descriptor_size;

    // Initialize the page frame allocator
    GlobalAllocator = PageFrameAllocator();
    GlobalAllocator.read_efi_memory_map(boot_info->memory_map, boot_info->memory_map_size, boot_info->memory_map_descriptor_size);

    // Calculate how many pages the kernel needs
    uint64_t kernel_size = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
    uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;

    // Lock that many pages starting from kernel start
    GlobalAllocator.lock_pages(&_KernelStart, kernel_pages);

    // Initialize the PML4 memory table and manager
    PageTable* PML4 = (PageTable*)GlobalAllocator.request_page();
    memset(PML4, 0, 0x1000);
    kernel_util_page_table_manager = PageTableManager(PML4);

    // Map all memory from virtual = physical
    for (uint64_t t = 0; t < get_memory_size(boot_info->memory_map, memory_map_entries, boot_info->memory_map_descriptor_size); t+= 0x1000){
        kernel_util_page_table_manager.map_memory((void*)t, (void*)t);
    }

    // Also map the framebuffer
    uint64_t fb_base = (uint64_t)boot_info->framebuffer->base_address;
    uint64_t fb_size = (uint64_t)boot_info->framebuffer->buffer_size + 0x1000;
    GlobalAllocator.lock_pages((void*)fb_base, fb_size/ 0x1000 + 1);
    for (uint64_t t = fb_base; t < fb_base + fb_size; t += 4096){
        kernel_util_page_table_manager.map_memory((void*)t, (void*)t);
    }

    // Tell the cpu to use the Memory map we created
    asm ("mov %0, %%cr3" : : "r" (PML4));

    kernel_util_kernel_info.page_table_manager = &kernel_util_page_table_manager;
}

// Prepare the system's interrupts
void prepare_interrupts() {
    // Create the IDTR
    idtr.limit = 0x0fff;
    idtr.offset = (uint64_t) GlobalAllocator.request_page();

    // Page fault handler
    IDTDescriptorEntry *interrupt_page_fault = (IDTDescriptorEntry *)(idtr.offset + 0xe * sizeof(IDTDescriptorEntry));
    interrupt_page_fault->set_offset((uint64_t) page_fault_handler);
    interrupt_page_fault->type_attr = IDT_TA_InterruptGate;
    interrupt_page_fault->selector = 0x08;

    // Double fault handler (two unhandled faults in a row)
    IDTDescriptorEntry *interrupt_double_fault = (IDTDescriptorEntry *)(idtr.offset + 0x8 * sizeof(IDTDescriptorEntry));
    interrupt_double_fault->set_offset((uint64_t) double_fault_handler);
    interrupt_double_fault->type_attr = IDT_TA_InterruptGate;
    interrupt_double_fault->selector = 0x08;

    // General protection fault handler (all sorts of reasons, usually wrong execution permission)
    IDTDescriptorEntry *interrupt_general_protection_fault = (IDTDescriptorEntry *)(idtr.offset + 0xd * sizeof(IDTDescriptorEntry));
    interrupt_general_protection_fault->set_offset((uint64_t) general_protection_fault_handler);
    interrupt_general_protection_fault->type_attr = IDT_TA_InterruptGate;
    interrupt_general_protection_fault->selector = 0x08;

    // Keyboard interrupt
    // PIC was remapped to 0x20 and keyboard was the second interrupt hence 0x21
    IDTDescriptorEntry *interrupt_keyboard = (IDTDescriptorEntry *)(idtr.offset + 0x21 * sizeof(IDTDescriptorEntry));
    interrupt_keyboard->set_offset((uint64_t) keyboard_interrupt_handler);
    interrupt_keyboard->type_attr = IDT_TA_InterruptGate;
    interrupt_keyboard->selector = 0x08;

    asm ("lidt %0" : : "m" (idtr));

    // Remap the PIC interrupts
    remap_pic();
    // Unmask the keyboard interrupt from master PIC
    outb(PIC1_DATA, 0b11111101);
    outb(PIC2_DATA, 0b11111111);
    // Enable the maskable interrupts
    asm ("sti");
}

// Everything we need to do to get the kernel basic functionality
KernelInfo initialize_kernel(BootInfo* boot_info){

    // Create GlobalRenderer
    renderer = BasicRenderer(boot_info->framebuffer, boot_info->psf1_font);
    GlobalRenderer = &renderer;

    // Load the GDT
    GDTDescriptor gdt_descriptor;
    gdt_descriptor.size = sizeof(GDT) - 1;
    gdt_descriptor.offset = (uint64_t)&DefaultGDT;
    load_gdt(&gdt_descriptor);

    // Prepare the memory map and manager
    prepare_memory(boot_info);

    // Set the entire draw buffer to black
    GlobalRenderer->clear(0x0);

    // Prepare the interrupt handlers
    prepare_interrupts();

    // Return the kernel info
    return kernel_util_kernel_info;
}