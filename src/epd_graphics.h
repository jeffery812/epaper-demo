/* src/epd_graphics.h */
#ifndef EPD_GRAPHICS_H
#define EPD_GRAPHICS_H

#include <stdint.h>
#include "epd_driver.h"

extern uint8_t framebuffer[EPD_WIDTH_BYTES * EPD_HEIGHT];

void epd_clear_buffer(uint8_t color);
void draw_pixel(int x, int y, int color);
void draw_char(int x, int y, char c, int scale);
void draw_string(int x, int y, const char *str, int scale);

#endif /* EPD_GRAPHICS_H */