#include <cstring>
#include <algorithm>
#include "esp_log.h"
#include "display.h"
#include "tileset.h"
#include "buffer.h"
#include "colors.h"
#include "tft_driver.h"

static const char *TAG = "display";

CDisplay::CDisplay()
{
}

CDisplay::~CDisplay()
{
}

TFT_t *CDisplay::dev()
{
    return &m_dev;
}

bool CDisplay::init()
{
    // TFT_t dev;
    spi_master_init(&m_dev, (gpio_num_t)CONFIG_MOSI_GPIO, (gpio_num_t)CONFIG_SCLK_GPIO, (gpio_num_t)CONFIG_CS_GPIO, (gpio_num_t)CONFIG_DC_GPIO, (gpio_num_t)CONFIG_RESET_GPIO, (gpio_num_t)CONFIG_BL_GPIO);

#if CONFIG_ILI9225
    uint16_t model = 0x9225;
#elif CONFIG_ILI9225G
    uint16_t model = 0x9226;
#elif CONFIG_ILI9340
    uint16_t model = 0x9340;
#elif CONFIG_ILI9341
    uint16_t model = 0x9341;
#elif CONFIG_ST7735
    uint16_t model = 0x7735;
#elif CONFIG_ST7796
    uint16_t model = 0x7796;
#elif CONFIG_ST7789
    uint16_t model = 0x7789;
#else
    uint16_t model = 0;
#endif

    if (model == 0)
    {
        ESP_LOGE(TAG, "Invalid model. Please set model using menuconfig.");
        return false;
    }

    lcdInit(&m_dev, model, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

#ifdef CONFIG_INVERSION_TRUE
    ESP_LOGI(TAG, "Enable Display Inversion");
    lcdInversionOn(&m_dev);
#endif
#ifdef CONFIG_INVERSION_FALSE
    ESP_LOGI(TAG, "Disable Display Inversion");
    lcdInversionOff(&m_dev);
#endif
    fill(BLACK);

    return true;
}

bool CDisplay::test()
{
    CTileSet tiles;
    tiles.read("/spiffs/tiles.mcz");

    int i = 0;
    for (int y = 0; y < 16; ++y)
    {
        for (int x = 0; x < 15; ++x)
        {
            drawTile(x * 16, y * 16, reinterpret_cast<uint16_t *>(tiles[i]), 1);
            i++;
            if (i >= tiles.size())
            {
                return true;
            }
        }
    }

    return true;
}

// DrawTile
//
// Parameters:
// x:X coordinate
// y:Y coordinate
// size:Number of colors
// colors:colors
bool CDisplay::drawTile(uint16_t x, uint16_t y, uint16_t *tile, int mode)
{
    switch (mode)
    {
    case 0:

        for (int yy = y; yy < y + 16; ++yy)
        {
            lcdDrawMultiPixels(&m_dev, x, yy, 16, tile);
            tile += 16;
        }

        break;
    case 1:

        TFT_t *dev = &m_dev;
        int len = 16;
        int hei = 16;
        int size = len * hei;
        --len;

        if (x + len > dev->_width)
            return false;
        if (y + hei > dev->_height)
            return false;

        uint16_t _x1 = x + dev->_offsetx;
        uint16_t _x2 = _x1 + len;
        uint16_t _y1 = y + dev->_offsety;
        uint16_t _y2 = _y1 + hei;

        lcdDrawTile(dev, RectXY{_x1, _x2, _y1, _y2}, size, tile, true);
    }
    return true;
}

bool CDisplay::drawBuffer(uint16_t x, uint16_t y, CBuffer &buffer)
{
    TFT_t *dev = &m_dev;
    int len;
    int hei;
    int size;
    uint16_t _x1, _x2, _y1, _y2;

    uint16_t *data = buffer.start(len, hei);
    while (data)
    {
        size = len * hei;
        --len;
        if (x + len > dev->_width)
            return false;
        if (y + hei > dev->_height)
            return false;

        _x1 = x + dev->_offsetx;
        _x2 = _x1 + len;
        _y1 = y + dev->_offsety;
        _y2 = _y1 + hei;

        lcdDrawTile(dev, RectXY{_x1, _x2, _y1, _y2}, size, data, false);

        y += hei;
        data = buffer.next(len, hei);
    }
    return true;
}

bool CDisplay::fill(const uint8_t color)
{
    lcdFillScreen(&m_dev, color);
    return true;
}
