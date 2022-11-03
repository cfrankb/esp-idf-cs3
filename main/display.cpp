#include <cstring>
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "display.h"
#include "tileset.h"
#include "buffer.h"
#include "colors.h"

static const char *TAG = "display";

#if CONFIG_SPI2_HOST
#define HOST_ID SPI2_HOST
#elif CONFIG_SPI3_HOST
#define HOST_ID SPI3_HOST
#endif

static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode = 1;
static const int SPI_Frequency = SPI_MASTER_FREQ_20M;
// static const int SPI_Frequency = SPI_MASTER_FREQ_26M;
// static const int SPI_Frequency = SPI_MASTER_FREQ_40M;
// static const int SPI_Frequency = SPI_MASTER_FREQ_80M;

void spi_master_init(TFT_t *dev, gpio_num_t GPIO_MOSI, gpio_num_t GPIO_SCLK, gpio_num_t GPIO_CS, gpio_num_t GPIO_DC, gpio_num_t GPIO_RESET, gpio_num_t GPIO_BL)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "GPIO_CS=%d", GPIO_CS);
    if (GPIO_CS >= 0)
    {
        // gpio_pad_select_gpio( GPIO_CS );
        gpio_reset_pin(GPIO_CS);
        gpio_set_direction(GPIO_CS, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_CS, 0);
    }

    ESP_LOGI(TAG, "GPIO_DC=%d", GPIO_DC);
    // gpio_pad_select_gpio( GPIO_DC );
    gpio_reset_pin(GPIO_DC);
    gpio_set_direction(GPIO_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DC, 0);

    ESP_LOGI(TAG, "GPIO_RESET=%d", GPIO_RESET);
    if (GPIO_RESET >= 0)
    {
        // gpio_pad_select_gpio( GPIO_RESET );
        gpio_reset_pin(GPIO_RESET);
        gpio_set_direction(GPIO_RESET, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_RESET, 1);
        delayMS(50);
        gpio_set_level(GPIO_RESET, 0);
        delayMS(50);
        gpio_set_level(GPIO_RESET, 1);
        delayMS(50);
    }

    ESP_LOGI(TAG, "GPIO_BL=%d", GPIO_BL);
    if (GPIO_BL >= 0)
    {
        // gpio_pad_select_gpio(GPIO_BL);
        gpio_reset_pin(GPIO_BL);
        gpio_set_direction(GPIO_BL, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_BL, 0);
    }

    ESP_LOGI(TAG, "GPIO_MOSI=%d", GPIO_MOSI);
    ESP_LOGI(TAG, "GPIO_SCLK=%d", GPIO_SCLK);
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0};

    ret = spi_bus_initialize(HOST_ID, &buscfg, SPI_DMA_CH_AUTO);
    ESP_LOGD(TAG, "spi_bus_initialize=%d", ret);
    assert(ret == ESP_OK);

    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg));
    devcfg.clock_speed_hz = SPI_Frequency;
    devcfg.queue_size = 7;
    devcfg.mode = 2;
    devcfg.flags = SPI_DEVICE_NO_DUMMY;

    if (GPIO_CS >= 0)
    {
        devcfg.spics_io_num = GPIO_CS;
    }
    else
    {
        devcfg.spics_io_num = -1;
    }

    spi_device_handle_t handle;
    ret = spi_bus_add_device(HOST_ID, &devcfg, &handle);
    ESP_LOGD(TAG, "spi_bus_add_device=%d", ret);
    assert(ret == ESP_OK);
    dev->_dc = GPIO_DC;
    dev->_bl = GPIO_BL;
    dev->_SPIHandle = handle;
}

bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t *Data, size_t DataLength)
{
    spi_transaction_t SPITransaction;
    esp_err_t ret;

    if (DataLength > 0)
    {
        memset(&SPITransaction, 0, sizeof(spi_transaction_t));
        SPITransaction.length = DataLength * 8;
        SPITransaction.tx_buffer = Data;
#if 1
        ret = spi_device_transmit(SPIHandle, &SPITransaction);
#else
        ret = spi_device_polling_transmit(SPIHandle, &SPITransaction);
#endif
        assert(ret == ESP_OK);
    }

    return true;
}

bool spi_master_write_command(TFT_t *dev, uint8_t cmd)
{
    static uint8_t Byte = 0;
    Byte = cmd;
    gpio_set_level(dev->_dc, SPI_Command_Mode);
    return spi_master_write_byte(dev->_SPIHandle, &Byte, 1);
}

bool spi_master_write_data_byte(TFT_t *dev, uint8_t data)
{
    static uint8_t Byte = 0;
    Byte = data;
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, &Byte, 1);
}

bool spi_master_write_data_word(TFT_t *dev, uint16_t data)
{
    static uint8_t Byte[2];
    Byte[0] = (data >> 8) & 0xFF;
    Byte[1] = data & 0xFF;
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, Byte, 2);
}

bool spi_master_write_addr(TFT_t *dev, uint16_t addr1, uint16_t addr2)
{
    static uint8_t Byte[4];
    Byte[0] = (addr1 >> 8) & 0xFF;
    Byte[1] = addr1 & 0xFF;
    Byte[2] = (addr2 >> 8) & 0xFF;
    Byte[3] = addr2 & 0xFF;
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, Byte, 4);
}

bool spi_master_write_color(TFT_t *dev, uint16_t color, uint16_t size)
{
    static uint8_t Byte[1024];
    int index = 0;
    for (int i = 0; i < size; i++)
    {
        Byte[index++] = (color >> 8) & 0xFF;
        Byte[index++] = color & 0xFF;
    }
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, Byte, size * 2);
}

// Add 202001
bool spi_master_write_colors(TFT_t *dev, uint16_t *colors, uint16_t size)
{
    static uint8_t Byte[1024];
    int index = 0;
    for (int i = 0; i < size; i++)
    {
        Byte[index++] = (colors[i] >> 8) & 0xFF;
        Byte[index++] = colors[i] & 0xFF;
    }
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, Byte, size * 2);
}

// Draw multi pixel
// x:X coordinate
// y:Y coordinate
// size:Number of colors
// colors:colors
void lcdDrawMultiPixels(TFT_t *dev, uint16_t x, uint16_t y, uint16_t size, uint16_t *colors)
{
    if (x + size > dev->_width)
        return;
    if (y >= dev->_height)
        return;

    uint16_t _x1 = x + dev->_offsetx;
    uint16_t _x2 = _x1 + size;
    uint16_t _y1 = y + dev->_offsety;
    uint16_t _y2 = _y1;

    spi_master_write_command(dev, 0x2A); // set column(x) address
    spi_master_write_addr(dev, _x1, _x2);
    spi_master_write_command(dev, 0x2B); // set Page(y) address
    spi_master_write_addr(dev, _y1, _y2);
    spi_master_write_command(dev, 0x2C); //	Memory Write
    spi_master_write_colors(dev, colors, size);
}

void delayMS(int ms)
{
    int _ms = ms + (portTICK_PERIOD_MS - 1);
    TickType_t xTicksToDelay = _ms / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "ms=%d _ms=%d portTICK_PERIOD_MS=%d xTicksToDelay=%d", ms, _ms, portTICK_PERIOD_MS, xTicksToDelay);
    vTaskDelay(xTicksToDelay);
}

void lcdInit(TFT_t *dev, int width, int height, int offsetx, int offsety)
{
    dev->_width = width;
    dev->_height = height;
    dev->_offsetx = offsetx;
    dev->_offsety = offsety;
    // dev->_font_direction = DIRECTION0;
    dev->_font_fill = false;
    dev->_font_underline = false;

    spi_master_write_command(dev, 0x01); // Software Reset
    delayMS(150);

    spi_master_write_command(dev, 0x11); // Sleep Out
    delayMS(255);

    spi_master_write_command(dev, 0x3A); // Interface Pixel Format
    spi_master_write_data_byte(dev, 0x55);
    delayMS(10);

    spi_master_write_command(dev, 0x36); // Memory Data Access Control
    spi_master_write_data_byte(dev, 0x00);

    spi_master_write_command(dev, 0x2A); // Column Address Set
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0xF0);

    spi_master_write_command(dev, 0x2B); // Row Address Set
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0xF0);

    spi_master_write_command(dev, 0x21); // Display Inversion On
    delayMS(10);

    spi_master_write_command(dev, 0x13); // Normal Display Mode On
    delayMS(10);

    spi_master_write_command(dev, 0x29); // Display ON
    delayMS(255);

    if (dev->_bl >= 0)
    {
        gpio_set_level(dev->_bl, 1);
    }
}

bool initDisplay()
{
    TFT_t dev;
    spi_master_init(&dev, (gpio_num_t)CONFIG_MOSI_GPIO, (gpio_num_t)CONFIG_SCLK_GPIO, (gpio_num_t)CONFIG_CS_GPIO, (gpio_num_t)CONFIG_DC_GPIO, (gpio_num_t)CONFIG_RESET_GPIO, (gpio_num_t)CONFIG_BL_GPIO);
    lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

    lcdFillScreen(&dev, BLACK);

    ESP_LOGI(TAG, "Disable Display Inversion");
    lcdInversionOff(&dev);

    return true;
}

// Backlight OFF
void lcdBacklightOff(TFT_t *dev)
{
    if (dev->_bl >= 0)
    {
        gpio_set_level(dev->_bl, 0);
    }
}

// Backlight ON
void lcdBacklightOn(TFT_t *dev)
{
    if (dev->_bl >= 0)
    {
        gpio_set_level(dev->_bl, 1);
    }
}

// Draw rectangle of filling
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End X coordinate
// y2:End Y coordinate
// color:color
void lcdDrawFillRect(TFT_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    if (x1 >= dev->_width)
        return;
    if (x2 >= dev->_width)
        x2 = dev->_width - 1;
    if (y1 >= dev->_height)
        return;
    if (y2 >= dev->_height)
        y2 = dev->_height - 1;

    ESP_LOGD(TAG, "offset(x)=%d offset(y)=%d", dev->_offsetx, dev->_offsety);
    uint16_t _x1 = x1 + dev->_offsetx;
    uint16_t _x2 = x2 + dev->_offsetx;
    uint16_t _y1 = y1 + dev->_offsety;
    uint16_t _y2 = y2 + dev->_offsety;

    spi_master_write_command(dev, 0x2A); // set column(x) address
    spi_master_write_addr(dev, _x1, _x2);
    spi_master_write_command(dev, 0x2B); // set Page(y) address
    spi_master_write_addr(dev, _y1, _y2);
    spi_master_write_command(dev, 0x2C); //	Memory Write
    for (int i = _x1; i <= _x2; i++)
    {
        uint16_t size = _y2 - _y1 + 1;
        spi_master_write_color(dev, color, size);
#if 0
		for(j=y1;j<=y2;j++){
			//ESP_LOGD(TAG,"i=%d j=%d",i,j);
			spi_master_write_data_word(dev, color);
		}
#endif
    }
}

// Fill screen
// color:color
void lcdFillScreen(TFT_t *dev, uint16_t color)
{
    lcdDrawFillRect(dev, 0, 0, dev->_width - 1, dev->_height - 1, color);
}

// Display Inversion Off
void lcdInversionOff(TFT_t *dev)
{
    spi_master_write_command(dev, 0x20); // Display Inversion Off
}

// Display Inversion On
void lcdInversionOn(TFT_t *dev)
{
    spi_master_write_command(dev, 0x21); // Display Inversion On
}

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
    lcdInit(&m_dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

    ESP_LOGI(TAG, "Disable Display Inversion");
    lcdInversionOff(&m_dev);

    lcdFillScreen(&m_dev, RED);

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
            drawTile(x * 16, y * 16, tiles[i], 1);
            i++;
            if (i >= tiles.size())
            {
                return true;
            }
        }
    }

    // lcdDrawMultiPixels(&m_dev, 0, 30, 256, )

    // lcdDrawMultiPixels(TFT_t *dev, uint16_t x, uint16_t y, uint16_t size, uint16_t *colors)
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

        spi_master_write_command(dev, 0x2A); // set column(x) address
        spi_master_write_addr(dev, _x1, _x2);
        spi_master_write_command(dev, 0x2B); // set Page(y) address
        spi_master_write_addr(dev, _y1, _y2);
        spi_master_write_command(dev, 0x2C); //	Memory Write
        spi_master_write_colors(dev, tile, size);
        // spi_master_write_byte(dev->_SPIHandle, reinterpret_cast<uint8_t *>(tile), size * 2);
    }
    return true;
}

bool CDisplay::drawBuffer(uint16_t x, uint16_t y, CBuffer &buffer, int mode)
{
    TFT_t *dev = &m_dev;
    int len;
    int hei;
    int size;
    uint16_t _x1, _x2, _y1, _y2;

    switch (mode)
    {
    case 0:
        len = buffer.len();
        hei = 8; // buffer.hei();
        size = len * hei;
        --len;

        if (x + len > dev->_width)
            return false;
        if (y + hei >= dev->_height)
            return false;

        _x1 = x + dev->_offsetx;
        _x2 = _x1 + len;
        _y1 = y + dev->_offsety;
        _y2 = _y1 + hei;

        spi_master_write_command(dev, 0x2A); // set column(x) address
        spi_master_write_addr(dev, _x1, _x2);
        spi_master_write_command(dev, 0x2B); // set Page(y) address
        spi_master_write_addr(dev, _y1, _y2);
        spi_master_write_command(dev, 0x2C); //	Memory Write
                                             // spi_master_write_colors(dev, tile, size);
        gpio_set_level(dev->_dc, SPI_Data_Mode);
        spi_master_write_byte(dev->_SPIHandle, reinterpret_cast<uint8_t *>(buffer.buffer()), size * 2);
        break;

    case 1:
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

            spi_master_write_command(dev, 0x2A); // set column(x) address
            spi_master_write_addr(dev, _x1, _x2);
            spi_master_write_command(dev, 0x2B); // set Page(y) address
            spi_master_write_addr(dev, _y1, _y2);
            spi_master_write_command(dev, 0x2C); //	Memory Write
            gpio_set_level(dev->_dc, SPI_Data_Mode);
            //  printf("DMA USED: %p  len: %d, hei:%d\n", data, len, hei);
            spi_master_write_byte(dev->_SPIHandle, reinterpret_cast<uint8_t *>(data), size * 2);
            y += hei;
            data = buffer.next(len, hei);
        }
    }
    return true;
}