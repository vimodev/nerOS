#include "BasicRenderer.h"

BasicRenderer *GlobalRenderer;

// Create a renderer with a frame buffer and font
BasicRenderer::BasicRenderer(Framebuffer* target_frame_buffer, PSF1_FONT* psf1_font) {
    this->target_frame_buffer = target_frame_buffer;
    this->font = psf1_font;
    color = 0xffffffff;
    cursor_position = {0, 0};
}

// Clear the screen
void BasicRenderer::clear(uint32_t color) {
    // Base of the frame buffer
    uint64_t fb_base = (uint64_t) target_frame_buffer->base_address;
    // Pixels per scanline times 4, since 4 bytes per pixel
    uint64_t bytes_per_scanline = target_frame_buffer->pixels_per_scanline * 4;
    uint64_t fb_height = target_frame_buffer->height;
    uint64_t fb_size = target_frame_buffer->buffer_size;
    // Go over all scanlines
    for (int vertical_scan_line = 0; vertical_scan_line < fb_height; vertical_scan_line++) {
        // Calculate the base ptr to that scanline
        uint64_t pixel_ptr_base = fb_base + (bytes_per_scanline * vertical_scan_line);
        // Set all the pixels to color
        for (uint32_t *pixel_ptr = (uint32_t *) pixel_ptr_base; pixel_ptr < (uint32_t *) (pixel_ptr_base + bytes_per_scanline); pixel_ptr++) {
            *pixel_ptr = color;
        }
    }
}

void BasicRenderer::next() {
    cursor_position.X = 0;
    cursor_position.Y += 16;
}

// print a piece of text to terminal
void BasicRenderer::print(const char* str) {
    // While chr is not string terminator
    char* chr = (char*)str;
    while(*chr != 0){
        // Handle newlines
        if (*chr == '\n') {
            next();
            chr++;
            continue;
        }
        // Write the character
        put_char(*chr, cursor_position.X, cursor_position.Y);
        // And move cursor
        cursor_position.X+=8;
        if(cursor_position.X + 8 > target_frame_buffer->width) {
            next();
        }
        chr++;
    }
}

// Write the given char at the given x,y 
void BasicRenderer::put_char(char chr, unsigned int x_off, unsigned int y_off) {
    unsigned int* pixel_ptr = (unsigned int*)target_frame_buffer->base_address;
    // Get character info from glyph buffer
    char* font_ptr = (char*)font->glyph_buffer + (chr * font->psf1_header->charsize);
    // Write the character info to the pixel_ptr
    for (unsigned long y = y_off; y < y_off + 16; y++) {
        for (unsigned long x = x_off; x < x_off+8; x++) {
            // If target bit is 1, draw pixel with set color
            if ((*font_ptr & (0b10000000 >> (x - x_off))) > 0) {
                *(unsigned int*)(pixel_ptr + x + (y * target_frame_buffer->pixels_per_scanline)) = color;
            }

        }
        font_ptr++;
    }
}