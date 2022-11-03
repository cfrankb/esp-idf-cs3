#include "map.h"
#include <cstring>
#include <cstdio>

static const char SIG[] = "MAPZ";
static const uint16_t VERSION = 0;

CMap::CMap(int len, int hei, uint8_t t)
{
    m_size = 0;
    m_len = len;
    m_hei = hei;
    m_map = (m_len * m_hei) != 0 ? new uint8_t[m_len * m_hei] : nullptr;
    if (m_map)
    {
        m_size = m_len * m_hei;
        memset(m_map, t, m_size * sizeof(m_map[0]));
    }
};

CMap::~CMap()
{
    forget();
};

uint8_t &CMap::at(int x, int y)
{
    if (x >= m_len)
    {
        printf("x [%d] greater than m_len\n", x);
    }
    if (y >= m_hei)
    {
        printf("y [%d] greater than m_hei\n", y);
    }
    return *(m_map + x + y * m_len);
}

uint8_t *CMap::row(int y)
{
    return m_map + y * m_len;
}

void CMap::set(int x, int y, uint8_t t)
{
    at(x, y) = t;
}

void CMap::forget()
{
    if (m_map != nullptr)
    {
        delete[] m_map;
        m_map = nullptr;
        m_size = 0;
    }
    m_len = 0;
    m_hei = 0;
}

bool CMap::read(const char *fname)
{
    FILE *sfile = fopen(fname, "rb");
    if (sfile)
    {
        read(sfile);
        fclose(sfile);
    }
    return sfile != nullptr;
}

bool CMap::write(const char *fname)
{
    FILE *tfile = fopen(fname, "wb");
    if (tfile)
    {
        write(tfile);
        fclose(tfile);
    }
    return tfile != nullptr;
}

bool CMap::read(FILE *sfile)
{
    if (sfile)
    {
        char sig[strlen(SIG)];
        fread(sig, strlen(SIG), 1, sfile);
        if (memcmp(sig, SIG, strlen(SIG)) != 0)
        {
            printf("signature mismatch\n");
            return false;
        }
        uint16_t ver;
        fread(&ver, sizeof(VERSION), 1, sfile);
        if (ver > VERSION)
        {
            printf("bad version\n");
            return false;
        }
        uint8_t len;
        uint8_t hei;
        fread(&len, sizeof(len), 1, sfile);
        fread(&hei, sizeof(hei), 1, sfile);
        resize(len, hei);
        fread(m_map, len * hei, 1, sfile);
        m_attrs.clear();
    }
    return sfile != nullptr;
}

bool CMap::write(FILE *tfile)
{
    if (tfile)
    {
        fwrite(SIG, strlen(SIG), 1, tfile);
        fwrite(&VERSION, sizeof(VERSION), 1, tfile);
        fwrite(&m_len, sizeof(uint8_t), 1, tfile);
        fwrite(&m_hei, sizeof(uint8_t), 1, tfile);
        fwrite(m_map, m_len * m_hei, 1, tfile);
    }
    return tfile != nullptr;
}

int CMap::len() const
{
    return m_len;
}
int CMap::hei() const
{
    return m_hei;
}

bool CMap::resize(int len, int hei)
{
    if (len * hei > m_size)
    {
        forget();
        m_map = new uint8_t[len * hei];
        if (m_map == nullptr)
        {
            printf("resize fail\n");
            return false;
        }
        m_size = len * hei;
    }
    m_len = len;
    m_hei = hei;
    m_attrs.clear();
    return true;
}

const Pos CMap::findFirst(uint8_t tileId)
{
    for (uint8_t y = 0; y < m_hei; ++y)
    {
        for (uint8_t x = 0; x < m_len; ++x)
        {
            if (at(x, y) == tileId)
            {
                return Pos{x, y};
            }
        }
    }
    return Pos{0xff, 0xff};
}

int CMap::count(uint8_t tileId)
{
    int count = 0;
    for (uint8_t y = 0; y < m_hei; ++y)
    {
        for (uint8_t x = 0; x < m_len; ++x)
        {
            if (at(x, y) == tileId)
            {
                ++count;
            }
        }
    }
    return count;
}

void CMap::clear(uint8_t ch)
{
    if (m_map && m_len * m_hei > 0)
    {
        for (int i = 0; i < m_len * m_hei; ++i)
        {
            m_map[i] = ch;
        }
    }
    m_attrs.clear();
}

uint8_t CMap::getAttr(const uint8_t x, const uint8_t y)
{
    const uint16_t key = x + (y << 8);
    if (m_attrs.count(key) != 0)
    {
        return m_attrs[key];
    }
    else
    {
        return 0;
    }
}
void CMap::setAttr(const uint8_t x, const uint8_t y, const uint8_t a)
{
    const uint16_t key = x + (y << 8);
    if (a == 0)
    {
        m_attrs.erase(key);
    }
    else
    {
        m_attrs[key] = a;
    }
}