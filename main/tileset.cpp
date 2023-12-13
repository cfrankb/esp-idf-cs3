#include "tileset.h"
#include <cstring>
#include <cstdio>
#include <cassert>

static const char SIG[] = "MCXZ";
static const uint16_t VERSION = 0;

CTileSet::CTileSet(int width, int height, int count, int pixelWidth)
{
    m_pixelWidth = pixelWidth ? pixelWidth : pixel16;
    assert(m_pixelWidth == pixel16 || m_pixelWidth == pixel24);
    m_height = height;
    m_width = width;
    assert(m_width > 0 && m_height > 0);
    m_size = count;
    m_tileSize = m_height * m_width * m_pixelWidth;
    m_tiles = (count * m_tileSize) > 0 ? new uint8_t[m_tileSize * m_size] : nullptr;
    if (m_tiles)
    {
        memset(m_tiles, 0, m_tileSize * m_size);
    }
}

CTileSet::~CTileSet()
{
    forget();
}

void *CTileSet::operator[](int i)
{
    return m_tiles + i * m_tileSize;
}

// set tile
// i        index of target tile
// pixels   16bits color array
void CTileSet::set(int i, const void *pixels)
{
    memcpy(m_tiles + i * m_tileSize, pixels, m_tileSize);
}

int CTileSet::add(const void *tile)
{
    printf("size:%d;  %d x %d x %d = %d \n", m_size, m_height, m_width, m_pixelWidth, m_tileSize);
    uint8_t *t = new uint8_t[(m_size + 1) * m_tileSize];
    if (m_tiles != nullptr)
    {
        memcpy(t, m_tiles, m_tileSize * m_size);
        delete[] m_tiles;
    }
    memcpy(t + m_size * m_tileSize, tile, m_tileSize);
    m_tiles = t;
    return ++m_size;
}

// read
// read tileset from file
bool CTileSet::read(const char *fname, bool flipByteOrder)
{
    FILE *sfile = fopen(fname, "rb");
    if (sfile)
    {
        forget();
        char sig[sizeof(SIG)];
        uint16_t version;
        fread(sig, strlen(SIG), 1, sfile);
        fread(&version, sizeof(version), 1, sfile);
        if (memcmp(sig, SIG, strlen(SIG)) != 0)
        {
            printf("wrong signature\n");
            return false;
        }
        if ((version & 0xff) > VERSION)
        {
            printf("wrong version : 0x%.x\n", version);
            return false;
        }
        m_pixelWidth = version >> 8 ? version >> 8 : pixel16;
        if (m_pixelWidth != pixel16 && m_pixelWidth != pixel24)
        {
            printf("wrong pixelWidth: %d\n", m_pixelWidth);
            return false;
        }

        m_width = m_height = m_size = 0;
        fread(&m_width, 1, 1, sfile);
        fread(&m_height, 1, 1, sfile);
        fread(&m_size, 2, 1, sfile);
        uint16_t t;
        fread(&t, 2, 1, sfile);
        m_tileSize = m_width * m_height * m_pixelWidth;
        if (m_size)
        {
            m_tiles = new uint8_t[m_size * m_tileSize];
            fread(m_tiles, m_size * m_tileSize, 1, sfile);
            if (flipByteOrder && m_pixelWidth == pixel16)
            {
                // for displays that have a reversed byte order
                uint16_t *pixels = reinterpret_cast<uint16_t *>(m_tiles);
                for (int i = 0; i < m_height * m_width * m_size; ++i)
                {
                    pixels[i] = flipColor(pixels[i]);
                }
            }
        }
        fclose(sfile);
    }
    return sfile != nullptr;
}

// write
// write tileset to file
bool CTileSet::write(const char *fname, bool flipByteOrder, bool headerless)
{
    FILE *tfile = fopen(fname, "wb");
    if (tfile)
    {
        if (!headerless)
        {
            fwrite(SIG, strlen(SIG), 1, tfile);
            uint16_t version = VERSION + (m_pixelWidth << 8);
            fwrite(&version, sizeof(version), 1, tfile);
            fwrite(&m_width, 1, 1, tfile);
            fwrite(&m_height, 1, 1, tfile);
            fwrite(&m_size, 2, 1, tfile);
            uint16_t t = 0;
            fwrite(&t, 2, 1, tfile); // reserved
        }
        if (m_size)
        {
            if (flipByteOrder && m_pixelWidth == pixel16)
            {
                int tilePixels = m_width * m_height;
                uint16_t *tmpTile = new uint16_t[tilePixels];
                for (int i = 0; i < m_size; ++i)
                {
                    memcpy(tmpTile, m_tiles + m_tileSize * i, m_tileSize);
                    for (int j = 0; j < tilePixels; ++j)
                    {
                        tmpTile[j] = flipColor(tmpTile[j]);
                    }
                    fwrite(tmpTile, m_tileSize, 1, tfile);
                }
                delete[] tmpTile;
            }
            else
            {
                fwrite(m_tiles, m_size * m_tileSize, 1, tfile);
            }
        }
        fclose(tfile);
    }
    return tfile != nullptr;
}

// clear tileset
void CTileSet::forget()
{
    if (m_tiles)
    {
        delete[] m_tiles;
        m_tiles = nullptr;
    }
    m_size = 0;
}

// return tileset size in tiles
int CTileSet::size()
{
    return m_size;
}

// extend
// extend size of tileset by x tiles
int CTileSet::extendBy(int tiles)
{
    uint8_t *t = new uint8_t[m_tileSize * (m_size + tiles)];
    if (m_tiles != nullptr)
    {
        memcpy(t, m_tiles, m_size * m_tileSize);
        delete[] m_tiles;
    }
    m_tiles = t;
    m_size += tiles;
    return m_size;
}

// Flip bytes in a 16bbp pixel
uint16_t CTileSet::flipColor(const uint16_t c)
{
    return (c >> 8) + ((c & 0xff) << 8);
}