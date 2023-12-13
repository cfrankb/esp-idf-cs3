#ifndef ___TILESET_H
#define ___TILESET_H

#include <stdint.h>

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb24_t;

class CTileSet
{

public:
    CTileSet(int width = 16, int height = 16, int count = 0, int pixelWidth = 2);
    ~CTileSet();

    void *operator[](int i);
    int add(const void *tile);
    void set(int i, const void *pixels);

    bool read(const char *fname, bool flipByteOrder = false);
    bool write(const char *fname, bool flipByteOrder = false, bool headerless = false);
    void forget();
    int size();
    int extendBy(int tiles);

    enum
    {
        pixel16 = 2,
        pixel18 = 3,
        pixel24 = 3,
    };

protected:
    uint16_t flipColor(const uint16_t c);
    uint8_t m_pixelWidth;
    uint16_t m_tileSize;
    uint16_t m_width;
    uint16_t m_height;
    uint16_t m_size = 0;
    uint8_t *m_tiles;
};

#endif
