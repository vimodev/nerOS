#include "BasicRenderer.h"

// print the given string at the cursor position
void BasicRenderer::print(unsigned int color, const char *str) {
	char *chr = (char *) str;
	while (*chr != 0) {
		if (*chr == '\n') {
			cursorPosition.x = 0;
			cursorPosition.y += 16;
			chr++;
			continue;
		}
		BasicRenderer::putChar(color, *chr, cursorPosition.x, cursorPosition.y);
		cursorPosition.x += 8;
		if (cursorPosition.x + 8 > framebuffer->Width) {
			cursorPosition.x = 0;
			cursorPosition.y += 16;
		}
		chr++;
	}
}

void BasicRenderer::print(const char *str) {
    print(defaultColor, str);
}

// put the given character with the given color at the given offset in the framebuffer
void BasicRenderer::putChar(unsigned int color, char chr, unsigned int xOff, unsigned int yOff) {
	unsigned int *pxPtr = (unsigned int *) framebuffer->BaseAddress;
	char * fontPtr = (char *) font->glyphBuffer + (chr * font->psf1_header->charsize);
	for (unsigned long y = yOff; y < yOff + 16; y++) {
		for (unsigned long x = xOff; x < xOff + 8; x++) {
			if ((*fontPtr & (0b10000000 >> (x - xOff))) > 0) {
				*(unsigned int *)(pxPtr + x + (y * framebuffer->PixelsPerScanline)) = color;
			}
		}
		fontPtr++;
	}
}