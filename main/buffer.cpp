#include "buffer.h"
#include <esp_heap_caps_init.h>
#include <algorithm>
#include "font.h"
#include "colors.h"

CBuffer::CBuffer(int len, int hei, int limit)
{
    m_buffer = reinterpret_cast<uint16_t *>(heap_caps_malloc(len * hei * sizeof(m_buffer[0]), MALLOC_CAP_DMA | MALLOC_CAP_32BIT));
    m_len = len;
    m_hei = hei;
    m_limit = limit;
    printf("DMA: %p  len: %d, hei:%d\n", m_buffer, len, hei);
}

CBuffer::~CBuffer()
{
    if (m_buffer)
    {
        free(m_buffer);
    }
}

int CBuffer::len()
{
    return m_len;
}
int CBuffer::hei()
{
    return m_hei;
}

void CBuffer::drawTile32(const int x, const int y, uint16_t *tile)
{
    uint16_t *o = m_buffer + x + y * m_len;
    uint32_t *p32 = reinterpret_cast<uint32_t *>(tile);
    for (int yy = 0; yy < 16; ++yy)
    {
        uint32_t *d32 = reinterpret_cast<uint32_t *>(o);
        d32[0] = p32[0];
        d32[1] = p32[1];
        d32[2] = p32[2];
        d32[3] = p32[3];
        d32[4] = p32[4];
        d32[5] = p32[5];
        d32[6] = p32[6];
        d32[7] = p32[7];
        o += m_len;
        p32 += 8;
    }
}

void CBuffer::drawTile(int x, int y, uint16_t *tile)
{
    uint16_t *o = m_buffer + x + y * m_len;

    for (int yy = 0; yy < 16; ++yy)
    {
        for (int xx = 0; xx < 16; ++xx)
        {
            uint8_t *d = reinterpret_cast<uint8_t *>(o + xx);
            d[0] = tile[0] >> 8;
            d[1] = tile[0] & 0xff;
            // o[xx] = *tile;
            ++tile;
        }
        o += m_len;
    }
}

uint16_t *CBuffer::buffer()
{
    return m_buffer;
}

uint16_t *CBuffer::start(int &len, int &hei)
{
    hei = std::min(m_limit / m_len, m_hei);
    len = m_len;
    m_oY = hei;
    return m_buffer;
}
uint16_t *CBuffer::next(int &len, int &hei)
{
    hei = std::min(m_limit / m_len, m_hei - m_oY);
    len = m_len;
    int oY = m_oY;
    m_oY += hei;
    return hei > 0 ? m_buffer + oY * m_len : nullptr;
}

void CBuffer::fill(const uint16_t color)
{
    const uint16_t fcolor = flipColor(color);
    if (m_buffer)
    {
        for (int i = 0; i < m_len * m_hei; ++i)
        {
            m_buffer[i] = fcolor;
        }
    }
}

void CBuffer::forget()
{
    if (m_buffer)
    {
        free(m_buffer);
        m_buffer = nullptr;
    }
    m_hei = m_len = 0;
}

void CBuffer::drawFont(int x, int y, CFont &font, const char *s, uint16_t color)
{
    const uint16_t fcolor = flipColor(color);
    for (int j = 0; s[j]; ++j)
    {
        int u = s[j] < 127 ? s[j] - 32 : 0;
        uint8_t *data = font[u > 0 ? u : 0];
        uint16_t *o = m_buffer + x + y * m_len + 8 * j;
        for (int yy = 0; yy < 8; ++yy)
        {
            for (int xx = 0; xx < 8; ++xx)
            {
                o[xx] = *data ? fcolor : 0;
                ++data;
            }
            o += m_len;
        }
    }
}

void CBuffer::drawRect(const Rect &rect, uint16_t color)
{
    const uint16_t fcolor = flipColor(color);
    uint16_t *buf = m_buffer + rect.x + rect.y * m_len;
    for (int y = 0; y < rect.height; ++y)
    {
        for (int x = 0; x < rect.width; ++x)
        {
            buf[x] = fcolor;
        }
        buf += m_len;
    }
}