#include "interrupts.h"

__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame *frame) {
    GlobalRenderer->print("Page fault detected\n");
    while (true);
}