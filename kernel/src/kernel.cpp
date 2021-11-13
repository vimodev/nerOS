#include <stddef.h>
#include "BasicRenderer.h"

extern "C" void _start(Framebuffer *fb, PSF1_FONT *ft) {

	BasicRenderer renderer;
	renderer.cursorPosition = {0, 0};
	renderer.framebuffer = fb;
	renderer.font = ft;
	renderer.defaultColor = COLOR_WHITE;

	char str[] = "Hello Kernel!\n";
	for (int t = 0; t < 30; t++) {
		renderer.print(str);
	}

    return;
}