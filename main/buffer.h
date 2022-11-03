#ifndef __BUFFER__H
#define __BUFFER__H

#include <stdint.h>

class CFont;
class CBuffer
{
public:
    CBuffer(int len, int hei, int limit);
    ~CBuffer();

    int len();
    int hei();
    void drawTile(int x, int y, uint16_t *tile);
    void drawTile32(const int x, const int y, uint16_t *tile);
    void drawFont(int x, int y, CFont &font, const char *s, uint16_t color = 0xffff);
    uint16_t *buffer();

    uint16_t *start(int &hei, int &len);
    uint16_t *next(int &hei, int &len);
    void fill(const uint16_t color);
    void forget();

protected:
    uint16_t *m_buffer;
    int m_len;
    int m_hei;
    uint16_t m_limit;
    int m_oY;
};

#endif