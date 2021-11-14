#pragma once
#include "math.h"
#include "Framebuffer.h"
#include "simpleFonts.h"

class BasicRenderer {
    public:
    BasicRenderer(Framebuffer* target_frame_buffer, PSF1_FONT* psf1_font);
    Point cursor_position;
    Framebuffer* target_frame_buffer;
    PSF1_FONT* font;
    unsigned int color;
    void print(const char* str);
    void put_char(char chr, unsigned int x_off, unsigned int y_off);
};