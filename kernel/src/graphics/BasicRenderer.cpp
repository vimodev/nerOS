#include "BasicRenderer.h"

BasicRenderer *GlobalRenderer;

// Create a renderer with a frame buffer and font
BasicRenderer::BasicRenderer(Framebuffer* target_frame_buffer, PSF1_FONT* psf1_font) {
    this->target_frame_buffer = target_frame_buffer;
    this->font = psf1_font;
    color = 0xffffffff;
    clear_color = 0x00000000;
    cursor_position = {0, 0};
}

// Set the given pixel to the given color
void BasicRenderer::put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    *(uint32_t *) ((uint64_t) target_frame_buffer->base_address + (x * 4) + (y * target_frame_buffer->pixels_per_scanline * 4)) = color;
}

// Get the color of the given pixel
uint32_t BasicRenderer::get_pixel(uint32_t x, uint32_t y) {
    return *(uint32_t *) ((uint64_t) target_frame_buffer->base_address + (x * 4) + (y * target_frame_buffer->pixels_per_scanline * 4));
}

// Draw the MouseCursor in an overlay
void BasicRenderer::draw_overlay_mouse_cursor(uint8_t *mouse_cursor, Point position, uint32_t color) {
    // Prevent us from trying to draw outside the buffer when mouse is at edge
    int y_max = 16; int x_max = 16;
    int delta_x = target_frame_buffer->width - position.X;
    int delta_y = target_frame_buffer->height - position.Y;
    if (delta_x < x_max) x_max = delta_x;
    if (delta_y < y_max) y_max = delta_y;
    // Go over the mouse pointer pixel map
    for (int y = 0; y < y_max; y++) {
        for (int x = 0; x < x_max; x++) {
            int bit = y * 16 + x;
            int byte = bit / 8;
            // If the pixel is set in the buffer, we draw it
            if ((mouse_cursor[byte] & (0b10000000 >> (x % 8)))) {
                // Store pixel state
                mouse_cursor_buffer[x + y * 16] = get_pixel(position.X + x, position.Y + y);
                put_pixel(position.X + x, position.Y + y, color);
                mouse_cursor_buffer_after[x + y * 16] = color;
            }
        }
    }
}

// Restore the pixels to the state they were before mouse cursor was drawn
void BasicRenderer::clear_mouse_cursor(uint8_t *mouse_cursor, Point position) {
    // Prevent out of bounds
    int y_max = 16; int x_max = 16;
    int delta_x = target_frame_buffer->width - position.X;
    int delta_y = target_frame_buffer->height - position.Y;
    if (delta_x < x_max) x_max = delta_x;
    if (delta_y < y_max) y_max = delta_y;
    // Restore every pixel we changed
    for (int y = 0; y < y_max; y++) {
        for (int x = 0; x < x_max; x++) {
            int bit = y * 16 + x;
            int byte = bit / 8;
            if ((mouse_cursor[byte] & (0b10000000 >> (x % 8)))) {
                // Only restore if there has not been written any new colors in the meantime
                if (get_pixel(position.X + x, position.Y + y) == mouse_cursor_buffer_after[x + y * 16]) {
                    put_pixel(position.X + x, position.Y + y, mouse_cursor_buffer[x + y * 16]);
                }   
            }
        }
    }
}

// Clear the screen with the set color
void BasicRenderer::clear() {
    clear(this->clear_color);
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

// Clear the previous cursor position with default clear color
void BasicRenderer::clear_char() {
    clear_char(this->clear_color);
}

// Clear the previous cursor position and move there
void BasicRenderer::clear_char(uint32_t color) {
    // Ignore if we are at top left already
    if (cursor_position.X == 0 && cursor_position.Y == 0) return;
    // Move cursor back
    cursor_position.X -= 8;
    if (cursor_position.X < 0) {
        cursor_position.X += target_frame_buffer->width;
        cursor_position.Y -= 16;
        if (cursor_position.Y < 0) {
            cursor_position.Y = 0;
        }
    }
    // Completely clear that cell
    unsigned int x_off = cursor_position.X;
    unsigned int y_off = cursor_position.Y;
    unsigned int* pixel_ptr = (unsigned int*)target_frame_buffer->base_address;
    // Write the character info to the pixel_ptr
    for (unsigned long y = y_off; y < y_off + 16; y++) {
        for (unsigned long x = x_off; x < x_off + 8; x++) {
            *(unsigned int*)(pixel_ptr + x + (y * target_frame_buffer->pixels_per_scanline)) = color;
        }
    }
}

// Go to the next line
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

// Write the given char at the given x,y with default color
void BasicRenderer::put_char(char chr, unsigned int x_off, unsigned int y_off) {
    put_char(chr, x_off, y_off, this->color);
}

// Write the given char at the given x,y 
void BasicRenderer::put_char(char chr, unsigned int x_off, unsigned int y_off, uint32_t color) {
    unsigned int* pixel_ptr = (unsigned int*)target_frame_buffer->base_address;
    // Get character info from glyph buffer
    char* font_ptr = (char*)font->glyph_buffer + (chr * font->psf1_header->charsize);
    // Write the character info to the pixel_ptr
    for (unsigned long y = y_off; y < y_off + 16; y++) {
        for (unsigned long x = x_off; x < x_off + 8; x++) {
            // If target bit is 1, draw pixel with set color
            if ((*font_ptr & (0b10000000 >> (x - x_off))) > 0) {
                *(unsigned int*)(pixel_ptr + x + (y * target_frame_buffer->pixels_per_scanline)) = color;
            }

        }
        font_ptr++;
    }
}

// Write the char at the cursor
void BasicRenderer::put_char(char chr) {
    put_char(chr, this->color);
}

// Write the char at the cursor
void BasicRenderer::put_char(char chr, uint32_t color) {
    put_char(chr, cursor_position.X, cursor_position.Y, color);
    // Increment cursor position
    cursor_position.X += 8;
    if(cursor_position.X + 8 > target_frame_buffer->width) {
            next();
    }
}