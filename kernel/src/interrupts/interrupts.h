#pragma once

#include "../BasicRenderer.h"
#include "../panic.h"
#include "../io.h"
#include "../userinput/keyboard.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xa0
#define PIC2_DATA 0xa1

#define PIC_EOI 0x20

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

struct interrupt_frame;

__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame *frame);
__attribute__((interrupt)) void double_fault_handler(struct interrupt_frame *frame);
__attribute__((interrupt)) void general_protection_fault_handler(struct interrupt_frame *frame);
__attribute__((interrupt)) void keyboard_interrupt_handler(struct interrupt_frame *frame);

void remap_pic();
void pic_end_master();
void pic_end_slave();