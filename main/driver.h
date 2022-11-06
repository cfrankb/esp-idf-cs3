#ifndef __DRIVER_H___
#define __DRIVER_H___
#include <stdint.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>

typedef struct
{
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;
} RectXY;

typedef struct
{
    uint16_t _model;
    uint16_t _width;
    uint16_t _height;
    uint16_t _offsetx;
    uint16_t _offsety;
    uint16_t _font_direction;
    uint16_t _font_fill;
    uint16_t _font_fill_color;
    uint16_t _font_underline;
    uint16_t _font_underline_color;
    gpio_num_t _dc;
    gpio_num_t _bl;
    spi_device_handle_t _SPIHandle;
} TFT_t;

void spi_master_init(TFT_t *dev, gpio_num_t GPIO_MOSI, gpio_num_t GPIO_SCLK, gpio_num_t GPIO_CS, gpio_num_t GPIO_DC, gpio_num_t GPIO_RESET, gpio_num_t GPIO_BL);
bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t *Data, size_t DataLength);
bool spi_master_write_command(TFT_t *dev, uint8_t cmd);
bool spi_master_write_colors(TFT_t *dev, uint16_t *colors, uint16_t size, bool copyBytes);
bool spi_master_write_addr(TFT_t *dev, uint16_t addr1, uint16_t addr2);
bool spi_master_write_comm_byte(TFT_t *dev, uint8_t cmd);

void lcdInit(TFT_t *dev, uint16_t model, int width, int height, int offsetx, int offsety);
void lcdInversionOn(TFT_t *dev);
void lcdInversionOff(TFT_t *dev);
void lcdBacklightOn(TFT_t *dev);
void lcdBacklightOff(TFT_t *dev);
void lcdWriteRegisterByte(TFT_t *dev, uint8_t addr, uint16_t data);
void delayMS(int ms);
void lcdFillScreen(TFT_t *dev, uint16_t color);
void lcdDrawMultiPixels(TFT_t *dev, uint16_t x, uint16_t y, uint16_t size, uint16_t *colors);
void lcdDrawTile(TFT_t *dev, RectXY rect, uint16_t size, uint16_t *colors, bool copyBytes);

#endif