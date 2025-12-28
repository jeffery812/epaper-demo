/* src/epd_graphics.c */
#include <string.h>
#include "epd_graphics.h"

uint8_t framebuffer[EPD_WIDTH_BYTES * EPD_HEIGHT];

/* Simple 8x8 Bitmap Font Helper */
static const uint8_t *get_char_bitmap(char c) {
    /* Minimal font for "hello epaper" */
    static const uint8_t char_h[] = {0x00, 0x40, 0x40, 0x5C, 0x62, 0x62, 0x62, 0x00};
    static const uint8_t char_e[] = {0x00, 0x00, 0x3C, 0x42, 0x7E, 0x40, 0x3C, 0x00};
    static const uint8_t char_l[] = {0x00, 0x60, 0x20, 0x20, 0x20, 0x20, 0x3C, 0x00};
    static const uint8_t char_o[] = {0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00};
    static const uint8_t char_p[] = {0x00, 0x00, 0x5C, 0x62, 0x62, 0x5C, 0x40, 0x40};
    static const uint8_t char_a[] = {0x00, 0x00, 0x3C, 0x02, 0x3E, 0x42, 0x3E, 0x00};
    static const uint8_t char_r[] = {0x00, 0x00, 0x5C, 0x62, 0x40, 0x40, 0x40, 0x00};
    static const uint8_t char_sp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    switch(c) {
        case 'h': return char_h;
        case 'e': return char_e;
        case 'l': return char_l;
        case 'o': return char_o;
        case 'p': return char_p;
        case 'a': return char_a;
        case 'r': return char_r;
        default: return char_sp;
    }
}

void epd_clear_buffer(uint8_t color)
{
    memset(framebuffer, color, sizeof(framebuffer));
}

void draw_pixel(int x, int y, int color) {
    if (x < 0 || x >= (EPD_WIDTH_BYTES * 8) || y < 0 || y >= EPD_HEIGHT) return;
    
    int idx = y * EPD_WIDTH_BYTES + (x / 8);
    uint8_t mask = 0x80 >> (x % 8);

    if (color) {
        framebuffer[idx] |= mask; // White
    } else {
        framebuffer[idx] &= ~mask; // Black
    }
}

void draw_char(int x, int y, char c, int scale) {
    const uint8_t *bitmap = get_char_bitmap(c);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Check bit j in row i */
            if (bitmap[i] & (0x80 >> j)) {
                for (int dy = 0; dy < scale; dy++) {
                    for (int dx = 0; dx < scale; dx++) {
                        draw_pixel(x + j * scale + dx, y + i * scale + dy, 0); // Black
                    }
                }
            }
        }
    }
}

void draw_string(int x, int y, const char *str, int scale) {
    while (*str) {
        draw_char(x, y, *str, scale);
        x += 8 * scale;
        str++;
    }
}