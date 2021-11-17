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
        uint32_t mouse_cursor_buffer[16 * 16];
        uint32_t mouse_cursor_buffer_after[16 * 16];
        unsigned int color;
        unsigned int clear_color;
        void put_pixel(uint32_t x, uint32_t y, uint32_t color);
        uint32_t get_pixel(uint32_t x, uint32_t y);
        void print(const char* str);
        void println(const char* str = "");
        void put_char(char chr, unsigned int x_off, unsigned int y_off);
        void put_char(char chr, unsigned int x_off, unsigned int y_off, uint32_t color);
        void put_char(char chr);
        void put_char(char chr, uint32_t color);
        void clear_char(uint32_t color);
        void clear_char();
        void clear(uint32_t color);
        void clear();
        void next();
        void draw_overlay_mouse_cursor(uint8_t *mouse_cursor, Point position, uint32_t color);
        void clear_mouse_cursor(uint8_t *mouse_cursor, Point position);
};

extern BasicRenderer *GlobalRenderer;