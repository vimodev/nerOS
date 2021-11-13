#pragma once

#include "math.h"
#include "Framebuffer.h"
#include "simpleFonts.h"

enum COLOR {
	COLOR_WHITE = 0x00ffffff,
	COLOR_RED = 0x00ff0000,
	COLOR_BLUE = 0x0000ff00,
	COLOR_GREEN = 0x000000ff
};

class BasicRenderer {
    public:
        Point cursorPosition;
        Framebuffer *framebuffer;
        PSF1_FONT *font;
        COLOR defaultColor;
        void print(unsigned int color, const char *str);
        void print(const char *str);
        void putChar(unsigned int color, char chr, unsigned int xOff, unsigned int yOff);
};