#ifndef ___TILESET_H
#define ___TILESET_H

#include <stdint.h>

class CTileSet
{

public:
    CTileSet(int width = 16, int height = 16, int count = 0);
    ~CTileSet();

    uint16_t *operator[](int i);
    int add(const uint16_t *tile);
    void set(int i, const uint16_t *pixels);

    bool read(const char *fname);
    bool write(const char *fname);
    void forget();
    int size();
    int extendBy(int tiles);
    static void toggleFlipColors(bool b);

protected:
    int m_width;
    int m_height;
    int m_size = 0;
    uint16_t *m_tiles;
    static bool m_enableFlipColors;
};

#endif
