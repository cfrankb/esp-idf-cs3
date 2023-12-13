#ifndef __COLORS_H
#define __COLORS_H
#include <stdint.h>
uint16_t rgb888torgb565(uint8_t *rgb888Pixel);
uint16_t rgb888torgb565(uint8_t red, uint8_t green, uint8_t blue);
uint16_t flipColor(const uint16_t c);
#define rgb565 rgb888torgb565

#define RED 0xf800
#define GREEN 0x07e0
#define BLUE 0x001f
#define BLACK 0x0000
#define WHITE 0xffff
#define GRAY 0x8c51
#define YELLOW 0xFFE0
#define CYAN 0x07FF
#define PURPLE 0xF81F
#define LIME 0x0f0f
#define LIGHTGRAY rgb565(0xa9, 0xa9, 0xa9)
#endif