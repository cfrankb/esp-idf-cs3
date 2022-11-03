#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <driver/spi_master.h>
#include <driver/gpio.h>

typedef struct
{
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

void lcdInversionOn(TFT_t *dev);
void lcdInversionOff(TFT_t *dev);
void lcdBacklightOn(TFT_t *dev);
void lcdBacklightOff(TFT_t *dev);
void delayMS(int ms);
bool initDisplay();
void lcdFillScreen(TFT_t *dev, uint16_t color);

class CBuffer;

class CDisplay
{
public:
    CDisplay();
    ~CDisplay();
    bool init();
    bool test();
    bool drawTile(uint16_t x, uint16_t y, uint16_t *tile, int mode);
    bool drawBuffer(uint16_t x, uint16_t y, CBuffer &buffer, int mode);
    TFT_t *dev();

protected:
    TFT_t m_dev;
};

#endif