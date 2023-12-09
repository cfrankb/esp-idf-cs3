#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "driver.h"

class CBuffer;

class CDisplay
{
public:
    CDisplay();
    ~CDisplay();
    bool init();
    bool test();
    bool drawTile(uint16_t x, uint16_t y, uint16_t *tile, int mode);
    bool drawBuffer(uint16_t x, uint16_t y, CBuffer &buffer);
    bool fill(const uint8_t color);
    TFT_t *dev();

protected:
    TFT_t m_dev;
};

#endif