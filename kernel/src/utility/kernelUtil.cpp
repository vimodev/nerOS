#include "kernelUtil.h"

BasicRenderer renderer = BasicRenderer(NULL, NULL);
KernelInfo kernel_util_kernel_info;
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
    uint64_t kernel_pages = (uint64_t)kernel_size / 0x1000 + 1;

    // Lock that many pages starting from kernel start
    GlobalAllocator.lock_pages(&_KernelStart, kernel_pages);

    // Initialize the PML4 memory table and manager
    PageTable* PML4 = (PageTable*)GlobalAllocator.request_page();
    memset(PML4, 0, 0x1000);
    GlobalPageTableManager = PageTableManager(PML4);

    // Map all memory from virtual = physical
    for (uint64_t t = 0; t < get_memory_size(boot_info->memory_map, memory_map_entries, boot_info->memory_map_descriptor_size); t+= 0x1000){
        GlobalPageTableManager.map_memory((void*)t, (void*)t);
    }

    // Also map the framebuffer
    uint64_t fb_base = (uint64_t)boot_info->framebuffer->base_address;
    uint64_t fb_size = (uint64_t)boot_info->framebuffer->buffer_size + 0x1000;
    GlobalAllocator.lock_pages((void*)fb_base, fb_size/ 0x1000 + 1);
    for (uint64_t t = fb_base; t < fb_base + fb_size; t += 0x1000){
        GlobalPageTableManager.map_memory((void*)t, (void*)t);
    }

    // Tell the cpu to use the Memory map we created
    asm ("mov %0, %%cr3" : : "r" (PML4));

    kernel_util_kernel_info.page_table_manager = &GlobalPageTableManager;
}

// Set an IDT gate (binding interrupt handlers in the IDTR)
void set_idt_gate(void *handler, uint8_t entry_offset, uint8_t type_attribute, uint8_t selector) {
    IDTDescriptorEntry *interrupt = (IDTDescriptorEntry *)(idtr.offset + entry_offset * sizeof(IDTDescriptorEntry));
    interrupt->set_offset((uint64_t) handler);
    interrupt->type_attr = type_attribute;
    interrupt->selector = selector;
}

// Prepare the system's interrupts
void prepare_interrupts() {
    // Create the IDTR
    idtr.limit = 0x0fff;
    idtr.offset = (uint64_t) GlobalAllocator.request_page();

    // Page fault handler
    set_idt_gate((void *) page_fault_handler, 0xe, IDT_TA_InterruptGate, 0x08);
    // Double fault handler (two unhandled faults in a row)
    set_idt_gate((void *) double_fault_handler, 0x8, IDT_TA_InterruptGate, 0x08);
    // General protection fault handler (all sorts of reasons, usually wrong execution permission)
    set_idt_gate((void *) general_protection_fault_handler, 0xd, IDT_TA_InterruptGate, 0x08);
    // Keyboard interrupt
    // PIC was remapped to 0x20 and keyboard was the second interrupt hence 0x21
    set_idt_gate((void *) keyboard_interrupt_handler, 0x21, IDT_TA_InterruptGate, 0x08);
    // Mouse interrupt
    set_idt_gate((void *) mouse_interrupt_handler, 0x2c, IDT_TA_InterruptGate, 0x08);

    asm ("lidt %0" : : "m" (idtr));

    // Remap the PIC interrupts
    remap_pic();
}

// Prepare the ACPI interface for usage
void prepare_acpi(BootInfo *boot_info) {
    ACPI::SDTHeader *xsdt = (ACPI::SDTHeader *) (boot_info->rsdp->xsdt_address);
    // Find the MCFG table
    ACPI::MCFGHeader *mcfg = (ACPI::MCFGHeader *) ACPI::find_table(xsdt, (char *) "MCFG");
    // Enumerate over all pci devices
    PCI::enumerate_pci(mcfg);
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

    // Initialize the heap at a very large address
    initialize_heap((void *) 0x0000100000000000, 0x10);

    // Prepare the interrupt handlers
    prepare_interrupts();

    // Prepare the mouse
    init_ps2_mouse();

    // Prepare ACPI interface
    prepare_acpi(boot_info);

    // Unmask interrupts from master PIC
    // First zero bit allows interrupts from PIC2 slave to go through PIC1 master (cascade)
    // Second zero bit unmasks the keyboard interrupts, allowing us to handle them
    outb(PIC1_DATA, 0b11111001);
    // Zero bit unmasks the mouse interrupts, allowing us to handle them
    outb(PIC2_DATA, 0b11101111);
    // Enable the maskable interrupts
    asm ("sti");

    // Return the kernel info
    return kernel_util_kernel_info;
}