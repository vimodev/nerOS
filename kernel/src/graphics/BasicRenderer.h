#pragma once
#include "../utility/math.h"
#include "Framebuffer.h"
#include "simpleFonts.h"
#include <stdint.h>

class BasicRenderer {
    public:
    BasicRenderer(Framebuffer* target_frame_buffer, PSF1_FONT* psf1_font);
    Point cursor_position;
    Framebuffer* target_frame_buffer;
    PSF1_FONT* font;
    unsigned int color;
    unsigned int clear_color;
    void print(const char* str);
    void put_char(char chr, unsigned int x_off, unsigned int y_off);
    void put_char(char chr);
    void clear_char(uint32_t color);
    void clear_char();
    void clear(uint32_t color);
    void clear();
    void next();
};

extern BasicRenderer *GlobalRenderer;