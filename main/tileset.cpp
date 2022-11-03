#include "tileset.h"
#include <cstring>
#include <cstdio>
#include "colors.h"

static const char SIG[] = "MCXZ";
static const uint16_t VERSION = 0;

bool CTileSet::m_enableFlipColors = false;

CTileSet::CTileSet(int width, int height, int count)
{
    m_height = height;
    m_width = width;
    m_size = count;
    m_tiles = count ? new uint16_t[m_height * m_width * m_size] : nullptr;
    if (count)
    {
        memset(m_tiles, 0, m_height * m_width * m_size * sizeof(uint16_t));
    }
}

CTileSet::~CTileSet()
{
    forget();
}

uint16_t *CTileSet::operator[](int i)
{
    return m_tiles + i * m_height * m_width;
}

void CTileSet::set(int i, const uint16_t *pixels)
{
    int offset = m_height * m_width * i;
    int tileSize = m_height * m_width * sizeof(uint16_t);
    memcpy(m_tiles + offset, pixels, tileSize);
}

int CTileSet::add(const uint16_t *tile)
{
    int unitSize = m_height * m_width;
    int blocksize = unitSize * sizeof(uint16_t);
    uint16_t *t = new uint16_t[unitSize * (m_size + 1)];
    if (m_tiles != nullptr)
    {
        memcpy(t, m_tiles, blocksize * m_size);
        delete[] m_tiles;
    }
    memcpy(t + unitSize * m_size, tile, blocksize);
    m_tiles = t;

    return ++m_size;
}

bool CTileSet::read(const char *fname)
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

        if (version > VERSION)
        {
            printf("wrong version\n");
            return false;
        }

        m_width = m_height = m_size = 0;
        fread(&m_width, 1, 1, sfile);
        fread(&m_height, 1, 1, sfile);
        fread(&m_size, 4, 1, sfile);

        if (m_size)
        {
            m_tiles = new uint16_t[m_height * m_width * m_size];
            fread(m_tiles, m_height * m_width * m_size * sizeof(uint16_t), 1, sfile);
            if (m_enableFlipColors)
            {
                for (int i = 0; i < m_height * m_width * m_size; ++i)
                {
                    m_tiles[i] = flipColor(m_tiles[i]);
                }
            }
        }
        fclose(sfile);
    }
    return sfile != nullptr;
}

bool CTileSet::write(const char *fname)
{
    FILE *tfile = fopen(fname, "wb");
    if (tfile)
    {
        fwrite(SIG, strlen(SIG), 1, tfile);
        fwrite(&VERSION, sizeof(VERSION), 1, tfile);
        fwrite(&m_width, 1, 1, tfile);
        fwrite(&m_height, 1, 1, tfile);
        fwrite(&m_size, 4, 1, tfile);
        if (m_size)
        {
            fwrite(m_tiles, m_height * m_width * m_size * sizeof(uint16_t), 1, tfile);
        }
        fclose(tfile);
    }
    return tfile != nullptr;
}

void CTileSet::forget()
{
    if (m_tiles)
    {
        delete[] m_tiles;
        m_tiles = nullptr;
    }
    m_size = 0;
}

int CTileSet::size()
{
    return m_size;
}

// extend
// extend size of tileset by x tiles
int CTileSet::extendBy(int tiles)
{
    //     printf("size:%d  %d x %d \n", m_size, m_height, m_width);
    //
    int unitSize = m_height * m_width;
    int blocksize = unitSize * sizeof(uint16_t);
    uint16_t *t = new uint16_t[unitSize * (m_size + tiles)];
    if (m_tiles != nullptr)
    {
        memcpy(t, m_tiles, blocksize * m_size);
        delete[] m_tiles;
    }
    m_tiles = t;
    m_size += tiles;
    return m_size;
}

void CTileSet::toggleFlipColors(bool b)
{
    m_enableFlipColors = b;
}