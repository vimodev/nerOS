#pragma once

#include "../BasicRenderer.h"

struct interrupt_frame;

__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame *frame);