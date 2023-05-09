#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver.h"
#include "esp_log.h"

static const char *TAG = "driver";

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
        .data4_io_num = -1,
        .data5_io_num=0,
        .data6_io_num=0,
        .data7_io_num=0,
        .max_transfer_sz = 0,
        .flags = 0,
        .intr_flags=0
    };

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

bool spi_master_write_comm_byte(TFT_t *dev, uint8_t cmd)
{
    static uint8_t Byte = 0;
    Byte = cmd;
    gpio_set_level(dev->_dc, SPI_Command_Mode);
    // return spi_master_write_byte(dev->_TFT_Handle, &Byte, 1);
    return spi_master_write_byte(dev->_SPIHandle, &Byte, 1);
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
bool spi_master_write_colors(TFT_t *dev, uint16_t *colors, uint16_t size, bool copyBytes)
{
    static uint8_t Byte[1024];
    uint8_t *src = copyBytes ? Byte : reinterpret_cast<uint8_t *>(colors);
    if (copyBytes)
    {
        int index = 0;
        for (int i = 0; i < size; i++)
        {
            Byte[index++] = (colors[i] >> 8) & 0xFF;
            Byte[index++] = colors[i] & 0xFF;
        }
    }
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, src, size * 2);
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

    lcdDrawTile(dev, RectXY{_x1, _x2, _y1, _y2}, size, colors, true);

    /*   spi_master_write_command(dev, 0x2A); // set column(x) address
       spi_master_write_addr(dev, _x1, _x2);
       spi_master_write_command(dev, 0x2B); // set Page(y) address
       spi_master_write_addr(dev, _y1, _y2);
       spi_master_write_command(dev, 0x2C); //	Memory Write
       spi_master_write_colors(dev, colors, size);*/
}

void lcdDrawTile(TFT_t *dev, RectXY rect, uint16_t size, uint16_t *colors, bool copyBytes)
{
    uint16_t _x1 = rect.x1;
    uint16_t _x2 = rect.x2;
    uint16_t _y1 = rect.y1;
    uint16_t _y2 = rect.y2;

    if (dev->_model == 0x9340 ||
        dev->_model == 0x9341 ||
        dev->_model == 0x7789 ||
        dev->_model == 0x7796)
    {
        spi_master_write_comm_byte(dev, 0x2A); // set column(x) address
        spi_master_write_addr(dev, _x1, _x2);
        spi_master_write_comm_byte(dev, 0x2B); // set Page(y) address
        spi_master_write_addr(dev, _y1, _y2);
        spi_master_write_comm_byte(dev, 0x2C); // Memory Write
        spi_master_write_colors(dev, colors, size, copyBytes);
    } // endif 0x9340/0x9341/0x7789/0x7796

    if (dev->_model == 0x7735)
    {
        spi_master_write_comm_byte(dev, 0x2A); // set column(x) address
        spi_master_write_data_word(dev, _x1);
        spi_master_write_data_word(dev, _x2);
        spi_master_write_comm_byte(dev, 0x2B); // set Page(y) address
        spi_master_write_data_word(dev, _y1);
        spi_master_write_data_word(dev, _y2);
        spi_master_write_comm_byte(dev, 0x2C); // Memory Write
        spi_master_write_colors(dev, colors, size, copyBytes);
    } // 0x7735

    if (dev->_model == 0x9225)
    {
        for (int j = _y1; j <= _y2; j++)
        {
            lcdWriteRegisterByte(dev, 0x20, _x1);
            lcdWriteRegisterByte(dev, 0x21, j);
            spi_master_write_comm_byte(dev, 0x22); // Memory Write
            spi_master_write_colors(dev, colors, size, copyBytes);
        }
    } // endif 0x9225

    if (dev->_model == 0x9226)
    {
        for (int j = _x1; j <= _x2; j++)
        {
            lcdWriteRegisterByte(dev, 0x36, j);
            lcdWriteRegisterByte(dev, 0x37, j);
            lcdWriteRegisterByte(dev, 0x38, _y2);
            lcdWriteRegisterByte(dev, 0x39, _y1);
            lcdWriteRegisterByte(dev, 0x20, j);
            lcdWriteRegisterByte(dev, 0x21, _y1);
            spi_master_write_comm_byte(dev, 0x22);
            spi_master_write_colors(dev, colors, size, copyBytes);
        }
    } // endif 0x9226
}

void delayMS(int ms)
{
    int _ms = ms + (portTICK_PERIOD_MS - 1);
    TickType_t xTicksToDelay = _ms / portTICK_PERIOD_MS;
    //ESP_LOGD(TAG, "ms=%d _ms=%d portTICK_PERIOD_MS=%d xTicksToDelay=%d", ms, _ms, portTICK_PERIOD_MS, xTicksToDelay);
    vTaskDelay(xTicksToDelay);
}

void lcdInit(TFT_t *dev, u_int16_t model, int width, int height, int offsetx, int offsety)
{
    dev->_model = model;
    dev->_width = width;
    dev->_height = height;
    dev->_offsetx = offsetx;
    dev->_offsety = offsety;
    // dev->_font_direction = DIRECTION0;
    dev->_font_fill = false;
    dev->_font_underline = false;

    if (dev->_model == 0x7796)
    {
        ESP_LOGI(TAG, "Your TFT is ST7796");
        ESP_LOGI(TAG, "Screen width:%d", width);
        ESP_LOGI(TAG, "Screen height:%d", height);
        spi_master_write_comm_byte(dev, 0xC0); // Power Control 1
        spi_master_write_data_byte(dev, 0x10);
        spi_master_write_data_byte(dev, 0x10);

        spi_master_write_comm_byte(dev, 0xC1); // Power Control 2
        spi_master_write_data_byte(dev, 0x41);

        spi_master_write_comm_byte(dev, 0xC5); // VCOM Control 1
        spi_master_write_data_byte(dev, 0x00);
        spi_master_write_data_byte(dev, 0x22);
        spi_master_write_data_byte(dev, 0x80);
        spi_master_write_data_byte(dev, 0x40);

        spi_master_write_comm_byte(dev, 0x36); // Memory Access Control
        spi_master_write_data_byte(dev, 0x48); // Right top start, BGR color filter panel
        // spi_master_write_data_byte(dev, 0x68);	//Right top start, BGR color filter panel

        spi_master_write_comm_byte(dev, 0xB0); // Interface Mode Control
        spi_master_write_data_byte(dev, 0x00);

        spi_master_write_comm_byte(dev, 0xB1); // Frame Rate Control
        spi_master_write_data_byte(dev, 0xB0);
        spi_master_write_data_byte(dev, 0x11);

        spi_master_write_comm_byte(dev, 0xB4); // Display Inversion Control
        spi_master_write_data_byte(dev, 0x02);

        spi_master_write_comm_byte(dev, 0xB6); // Display Function Control
        spi_master_write_data_byte(dev, 0x02);
        spi_master_write_data_byte(dev, 0x02);
        spi_master_write_data_byte(dev, 0x3B);

        spi_master_write_comm_byte(dev, 0xB7); // Entry Mode Set
        spi_master_write_data_byte(dev, 0xC6);

        spi_master_write_comm_byte(dev, 0x3A); // Interface Pixel Format
        spi_master_write_data_byte(dev, 0x55);

        spi_master_write_comm_byte(dev, 0xF7); // Adjust Control 3
        spi_master_write_data_byte(dev, 0xA9);
        spi_master_write_data_byte(dev, 0x51);
        spi_master_write_data_byte(dev, 0x2C);
        spi_master_write_data_byte(dev, 0x82);

        spi_master_write_comm_byte(dev, 0x11); // Sleep Out
        delayMS(120);

        spi_master_write_comm_byte(dev, 0x29); // Display ON
    }                                          // endif 0x7796

    if (dev->_model == 0x9340 || dev->_model == 0x9341 || dev->_model == 0x7735)
    {
        if (dev->_model == 0x9340)
            ESP_LOGI(TAG, "Your TFT is ILI9340");
        if (dev->_model == 0x9341)
            ESP_LOGI(TAG, "Your TFT is ILI9341");
        if (dev->_model == 0x7735)
            ESP_LOGI(TAG, "Your TFT is ST7735");
        ESP_LOGI(TAG, "Screen width:%d", width);
        ESP_LOGI(TAG, "Screen height:%d", height);
        spi_master_write_comm_byte(dev, 0xC0); // Power Control 1
        spi_master_write_data_byte(dev, 0x23);

        spi_master_write_comm_byte(dev, 0xC1); // Power Control 2
        spi_master_write_data_byte(dev, 0x10);

        spi_master_write_comm_byte(dev, 0xC5); // VCOM Control 1
        spi_master_write_data_byte(dev, 0x3E);
        spi_master_write_data_byte(dev, 0x28);

        spi_master_write_comm_byte(dev, 0xC7); // VCOM Control 2
        spi_master_write_data_byte(dev, 0x86);

        spi_master_write_comm_byte(dev, 0x36); // Memory Access Control
        spi_master_write_data_byte(dev, 0x08); // Right top start, BGR color filter panel
        // spi_master_write_data_byte(dev, 0x00);//Right top start, RGB color filter panel

        spi_master_write_comm_byte(dev, 0x3A); // Pixel Format Set
        spi_master_write_data_byte(dev, 0x55); // 65K color: 16-bit/pixel

        spi_master_write_comm_byte(dev, 0x20); // Display Inversion OFF

        spi_master_write_comm_byte(dev, 0xB1); // Frame Rate Control
        spi_master_write_data_byte(dev, 0x00);
        spi_master_write_data_byte(dev, 0x18);

        spi_master_write_comm_byte(dev, 0xB6); // Display Function Control
        spi_master_write_data_byte(dev, 0x08);
        spi_master_write_data_byte(dev, 0xA2); // REV:1 GS:0 SS:0 SM:0
        spi_master_write_data_byte(dev, 0x27);
        spi_master_write_data_byte(dev, 0x00);

        spi_master_write_comm_byte(dev, 0x26); // Gamma Set
        spi_master_write_data_byte(dev, 0x01);

        spi_master_write_comm_byte(dev, 0xE0); // Positive Gamma Correction
        spi_master_write_data_byte(dev, 0x0F);
        spi_master_write_data_byte(dev, 0x31);
        spi_master_write_data_byte(dev, 0x2B);
        spi_master_write_data_byte(dev, 0x0C);
        spi_master_write_data_byte(dev, 0x0E);
        spi_master_write_data_byte(dev, 0x08);
        spi_master_write_data_byte(dev, 0x4E);
        spi_master_write_data_byte(dev, 0xF1);
        spi_master_write_data_byte(dev, 0x37);
        spi_master_write_data_byte(dev, 0x07);
        spi_master_write_data_byte(dev, 0x10);
        spi_master_write_data_byte(dev, 0x03);
        spi_master_write_data_byte(dev, 0x0E);
        spi_master_write_data_byte(dev, 0x09);
        spi_master_write_data_byte(dev, 0x00);

        spi_master_write_comm_byte(dev, 0xE1); // Negative Gamma Correction
        spi_master_write_data_byte(dev, 0x00);
        spi_master_write_data_byte(dev, 0x0E);
        spi_master_write_data_byte(dev, 0x14);
        spi_master_write_data_byte(dev, 0x03);
        spi_master_write_data_byte(dev, 0x11);
        spi_master_write_data_byte(dev, 0x07);
        spi_master_write_data_byte(dev, 0x31);
        spi_master_write_data_byte(dev, 0xC1);
        spi_master_write_data_byte(dev, 0x48);
        spi_master_write_data_byte(dev, 0x08);
        spi_master_write_data_byte(dev, 0x0F);
        spi_master_write_data_byte(dev, 0x0C);
        spi_master_write_data_byte(dev, 0x31);
        spi_master_write_data_byte(dev, 0x36);
        spi_master_write_data_byte(dev, 0x0F);

        spi_master_write_comm_byte(dev, 0x11); // Sleep Out
        delayMS(120);

        spi_master_write_comm_byte(dev, 0x29); // Display ON
    }                                          // endif 0x9340/0x9341/0x7735

    if (dev->_model == 0x7789)
    {
        if (dev->_model == 0x7789)
            ESP_LOGI(TAG, "Your TFT is ST7789");
        ESP_LOGI(TAG, "Screen width:%d", width);
        ESP_LOGI(TAG, "Screen height:%d", height);

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
    } // 0x7789

    if (dev->_model == 0x9225)
    {
        ESP_LOGI(TAG, "Your TFT is ILI9225");
        ESP_LOGI(TAG, "Screen width:%d", width);
        ESP_LOGI(TAG, "Screen height:%d", height);
        lcdWriteRegisterByte(dev, 0x10, 0x0000); // Set SAP,DSTB,STB
        lcdWriteRegisterByte(dev, 0x11, 0x0000); // Set APON,PON,AON,VCI1EN,VC
        lcdWriteRegisterByte(dev, 0x12, 0x0000); // Set BT,DC1,DC2,DC3
        lcdWriteRegisterByte(dev, 0x13, 0x0000); // Set GVDD
        lcdWriteRegisterByte(dev, 0x14, 0x0000); // Set VCOMH/VCOML voltage
        delayMS(40);

        // Power-on sequence
        lcdWriteRegisterByte(dev, 0x11, 0x0018); // Set APON,PON,AON,VCI1EN,VC
        lcdWriteRegisterByte(dev, 0x12, 0x6121); // Set BT,DC1,DC2,DC3
        lcdWriteRegisterByte(dev, 0x13, 0x006F); // Set GVDD
        lcdWriteRegisterByte(dev, 0x14, 0x495F); // Set VCOMH/VCOML voltage
        lcdWriteRegisterByte(dev, 0x10, 0x0800); // Set SAP,DSTB,STB
        delayMS(10);
        lcdWriteRegisterByte(dev, 0x11, 0x103B); // Set APON,PON,AON,VCI1EN,VC
        delayMS(50);

        lcdWriteRegisterByte(dev, 0x01, 0x011C); // set the display line number and display direction
        lcdWriteRegisterByte(dev, 0x02, 0x0100); // set 1 line inversion
        lcdWriteRegisterByte(dev, 0x03, 0x1030); // set GRAM write direction and BGR=1.
        lcdWriteRegisterByte(dev, 0x07, 0x0000); // Display off
        lcdWriteRegisterByte(dev, 0x08, 0x0808); // set the back porch and front porch
        lcdWriteRegisterByte(dev, 0x0B, 0x1100); // set the clocks number per line
        lcdWriteRegisterByte(dev, 0x0C, 0x0000); // CPU interface
        // lcdWriteRegisterByte(dev, 0x0F, 0x0D01); // Set Osc
        lcdWriteRegisterByte(dev, 0x0F, 0x0801); // Set Osc
        lcdWriteRegisterByte(dev, 0x15, 0x0020); // Set VCI recycling
        lcdWriteRegisterByte(dev, 0x20, 0x0000); // RAM Address
        lcdWriteRegisterByte(dev, 0x21, 0x0000); // RAM Address

        // Set GRAM area
        lcdWriteRegisterByte(dev, 0x30, 0x0000);
        lcdWriteRegisterByte(dev, 0x31, 0x00DB);
        lcdWriteRegisterByte(dev, 0x32, 0x0000);
        lcdWriteRegisterByte(dev, 0x33, 0x0000);
        lcdWriteRegisterByte(dev, 0x34, 0x00DB);
        lcdWriteRegisterByte(dev, 0x35, 0x0000);
        lcdWriteRegisterByte(dev, 0x36, 0x00AF);
        lcdWriteRegisterByte(dev, 0x37, 0x0000);
        lcdWriteRegisterByte(dev, 0x38, 0x00DB);
        lcdWriteRegisterByte(dev, 0x39, 0x0000);

        // Adjust GAMMA Curve
        lcdWriteRegisterByte(dev, 0x50, 0x0000);
        lcdWriteRegisterByte(dev, 0x51, 0x0808);
        lcdWriteRegisterByte(dev, 0x52, 0x080A);
        lcdWriteRegisterByte(dev, 0x53, 0x000A);
        lcdWriteRegisterByte(dev, 0x54, 0x0A08);
        lcdWriteRegisterByte(dev, 0x55, 0x0808);
        lcdWriteRegisterByte(dev, 0x56, 0x0000);
        lcdWriteRegisterByte(dev, 0x57, 0x0A00);
        lcdWriteRegisterByte(dev, 0x58, 0x0710);
        lcdWriteRegisterByte(dev, 0x59, 0x0710);

        lcdWriteRegisterByte(dev, 0x07, 0x0012);
        delayMS(50); // Delay 50ms
        lcdWriteRegisterByte(dev, 0x07, 0x1017);
    } // endif 0x9225

    if (dev->_model == 0x9226)
    {
        ESP_LOGI(TAG, "Your TFT is ILI9225G");
        ESP_LOGI(TAG, "Screen width:%d", width);
        ESP_LOGI(TAG, "Screen height:%d", height);
        // lcdWriteRegisterByte(dev, 0x01, 0x011c);
        lcdWriteRegisterByte(dev, 0x01, 0x021c);
        lcdWriteRegisterByte(dev, 0x02, 0x0100);
        lcdWriteRegisterByte(dev, 0x03, 0x1030);
        lcdWriteRegisterByte(dev, 0x08, 0x0808); // set BP and FP
        lcdWriteRegisterByte(dev, 0x0B, 0x1100); // frame cycle
        lcdWriteRegisterByte(dev, 0x0C, 0x0000); // RGB interface setting R0Ch=0x0110 for RGB 18Bit and R0Ch=0111for RGB16Bit
        lcdWriteRegisterByte(dev, 0x0F, 0x1401); // Set frame rate----0801
        lcdWriteRegisterByte(dev, 0x15, 0x0000); // set system interface
        lcdWriteRegisterByte(dev, 0x20, 0x0000); // Set GRAM Address
        lcdWriteRegisterByte(dev, 0x21, 0x0000); // Set GRAM Address
        //*************Power On sequence ****************//
        delayMS(50);
        lcdWriteRegisterByte(dev, 0x10, 0x0800); // Set SAP,DSTB,STB----0A00
        lcdWriteRegisterByte(dev, 0x11, 0x1F3F); // Set APON,PON,AON,VCI1EN,VC----1038
        delayMS(50);
        lcdWriteRegisterByte(dev, 0x12, 0x0121); // Internal reference voltage= Vci;----1121
        lcdWriteRegisterByte(dev, 0x13, 0x006F); // Set GVDD----0066
        lcdWriteRegisterByte(dev, 0x14, 0x4349); // Set VCOMH/VCOML voltage----5F60
        //-------------- Set GRAM area -----------------//
        lcdWriteRegisterByte(dev, 0x30, 0x0000);
        lcdWriteRegisterByte(dev, 0x31, 0x00DB);
        lcdWriteRegisterByte(dev, 0x32, 0x0000);
        lcdWriteRegisterByte(dev, 0x33, 0x0000);
        lcdWriteRegisterByte(dev, 0x34, 0x00DB);
        lcdWriteRegisterByte(dev, 0x35, 0x0000);
        lcdWriteRegisterByte(dev, 0x36, 0x00AF);
        lcdWriteRegisterByte(dev, 0x37, 0x0000);
        lcdWriteRegisterByte(dev, 0x38, 0x00DB);
        lcdWriteRegisterByte(dev, 0x39, 0x0000);
        // ----------- Adjust the Gamma Curve ----------//
        lcdWriteRegisterByte(dev, 0x50, 0x0001);
        lcdWriteRegisterByte(dev, 0x51, 0x200B);
        lcdWriteRegisterByte(dev, 0x52, 0x0000);
        lcdWriteRegisterByte(dev, 0x53, 0x0404);
        lcdWriteRegisterByte(dev, 0x54, 0x0C0C);
        lcdWriteRegisterByte(dev, 0x55, 0x000C);
        lcdWriteRegisterByte(dev, 0x56, 0x0101);
        lcdWriteRegisterByte(dev, 0x57, 0x0400);
        lcdWriteRegisterByte(dev, 0x58, 0x1108);
        lcdWriteRegisterByte(dev, 0x59, 0x050C);
        delayMS(50);
        lcdWriteRegisterByte(dev, 0x07, 0x1017);
    } // endif 0x9226

    if (dev->_bl >= 0)
    {
        gpio_set_level(dev->_bl, 1);
    }
}

void lcdWriteRegisterByte(TFT_t *dev, uint8_t addr, uint16_t data)
{
    spi_master_write_comm_byte(dev, addr);
    spi_master_write_data_word(dev, data);
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

    if (dev->_model == 0x9340 ||
        dev->_model == 0x9341 ||
        dev->_model == 0x7789 ||
        dev->_model == 0x7796)
    {
        spi_master_write_comm_byte(dev, 0x2A); // set column(x) address
        spi_master_write_addr(dev, _x1, _x2);
        spi_master_write_comm_byte(dev, 0x2B); // set Page(y) address
        spi_master_write_addr(dev, _y1, _y2);
        spi_master_write_comm_byte(dev, 0x2C); // Memory Write
        for (int i = _x1; i <= _x2; i++)
        {
            uint16_t size = _y2 - _y1 + 1;
            spi_master_write_color(dev, color, size);
        }
    } // endif 0x9340/0x9341/0x7796

    if (dev->_model == 0x7735)
    {
        spi_master_write_comm_byte(dev, 0x2A); // set column(x) address
        spi_master_write_data_word(dev, _x1);
        spi_master_write_data_word(dev, _x2);
        spi_master_write_comm_byte(dev, 0x2B); // set Page(y) address
        spi_master_write_data_word(dev, _y1);
        spi_master_write_data_word(dev, _y2);
        spi_master_write_comm_byte(dev, 0x2C); // Memory Write
        for (int i = _x1; i <= _x2; i++)
        {
            uint16_t size = _y2 - _y1 + 1;
            spi_master_write_color(dev, color, size);
        }
    } // 0x7735

    if (dev->_model == 0x9225)
    {
        for (int j = _y1; j <= _y2; j++)
        {
            lcdWriteRegisterByte(dev, 0x20, _x1);
            lcdWriteRegisterByte(dev, 0x21, j);
            spi_master_write_comm_byte(dev, 0x22); // Memory Write
            uint16_t size = _x2 - _x1 + 1;
            spi_master_write_color(dev, color, size);
        }
    } // endif 0x9225

    if (dev->_model == 0x9226)
    {
        for (int j = _x1; j <= _x2; j++)
        {
            lcdWriteRegisterByte(dev, 0x36, j);
            lcdWriteRegisterByte(dev, 0x37, j);
            lcdWriteRegisterByte(dev, 0x38, _y2);
            lcdWriteRegisterByte(dev, 0x39, _y1);
            lcdWriteRegisterByte(dev, 0x20, j);
            lcdWriteRegisterByte(dev, 0x21, _y1);
            spi_master_write_comm_byte(dev, 0x22);
            uint16_t size = _y2 - _y1 + 1;
            spi_master_write_color(dev, color, size);
#if 0
			for(int i=_y1;i<=_y2;i++) {
				spi_master_write_data_word(dev, color);
			}
#endif
        }
    } // endif 0x9226
}

// Fill screen
// color:color
void lcdFillScreen(TFT_t *dev, uint16_t color)
{
    lcdDrawFillRect(dev, 0, 0, dev->_width - 1, dev->_height - 1, color);
}

// Display ON
void lcdDisplayOn(TFT_t *dev)
{
    if (dev->_model == 0x9340 || dev->_model == 0x9341 || dev->_model == 0x7735 || dev->_model == 0x7789 || dev->_model == 0x7796)
    {
        spi_master_write_comm_byte(dev, 0x29);
    } // endif 0x9340/0x9341/0x7735/0x7796

    if (dev->_model == 0x9225 || dev->_model == 0x9226)
    {
        lcdWriteRegisterByte(dev, 0x07, 0x1017);
    } // endif 0x9225/0x9226
}

// Display Inversion OFF
void lcdInversionOff(TFT_t *dev)
{
    if (dev->_model == 0x9340 || dev->_model == 0x9341 || dev->_model == 0x7735 || dev->_model == 0x7789 || dev->_model == 0x7796)
    {
        spi_master_write_comm_byte(dev, 0x20);
    } // endif 0x9340/0x9341/0x7735/0x7796

    if (dev->_model == 0x9225 || dev->_model == 0x9226)
    {
        lcdWriteRegisterByte(dev, 0x07, 0x1017);
    } // endif 0x9225/0x9226
}

// Display Inversion ON
void lcdInversionOn(TFT_t *dev)
{
    if (dev->_model == 0x9340 || dev->_model == 0x9341 || dev->_model == 0x7735 || dev->_model == 0x7789 || dev->_model == 0x7796)
    {
        spi_master_write_comm_byte(dev, 0x21);
    } // endif 0x9340/0x9341/0x7735/0x7796

    if (dev->_model == 0x9225 || dev->_model == 0x9226)
    {
        lcdWriteRegisterByte(dev, 0x07, 0x1013);
    } // endif 0x9225/0x9226
}