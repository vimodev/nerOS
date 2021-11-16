#include "panel.h"
#include "graphics/BasicRenderer.h"

// Display kernel panic stream
void panic(const char *panic_message) {
    GlobalRenderer->clear(0x00ff0000);
    GlobalRenderer->cursor_position = {0, 0};
    GlobalRenderer->color = 0x0;
    GlobalRenderer->print("KERNEL PANIC:\n\n");
    GlobalRenderer->print(panic_message);
}