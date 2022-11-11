#include "map.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

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
    m_size = 0;
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
            m_lastError = "signature mismatch";
            printf("%s\n", m_lastError.c_str());
            return false;
        }
        uint16_t ver;
        fread(&ver, sizeof(VERSION), 1, sfile);
        if (ver > VERSION)
        {
            m_lastError = "bad version";
            printf("%s\n", m_lastError.c_str());
            return false;
        }
        uint8_t len;
        uint8_t hei;
        fread(&len, sizeof(len), 1, sfile);
        fread(&hei, sizeof(hei), 1, sfile);
        resize(len, hei, true);
        fread(m_map, len * hei, 1, sfile);
        m_attrs.clear();
        uint16_t attrCount = 0;
        fread(&attrCount, sizeof(attrCount), 1, sfile);
        for (int i=0; i < attrCount; ++i) {
            uint8_t x;
            uint8_t y;
            uint8_t a;
            fread(&x, sizeof(x), 1, sfile);
            fread(&y, sizeof(y), 1, sfile);
            fread(&a, sizeof(a), 1, sfile);
            setAttr(x,y,a);
        }
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
        uint16_t attrCount =  m_attrs.size();
        fwrite(&attrCount, sizeof(attrCount), 1, tfile);
        for (auto& it: m_attrs) {
            uint16_t key = it.first;
            uint8_t x = key & 0xff;
            uint8_t y = key >> 8;
            uint8_t a = it.second;
            fwrite(&x, sizeof(x), 1, tfile);
            fwrite(&y, sizeof(y), 1, tfile);
            fwrite(&a, sizeof(a), 1, tfile);
        }
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

bool CMap::resize(int len, int hei, bool fast)
{
    if (fast) {
        if (len * hei > m_size)
        {
            forget();
            m_map = new uint8_t[len * hei];
            if (m_map == nullptr)
            {
                m_lastError = "resize fail";
                printf("%s\n", m_lastError.c_str());
                return false;
            }
            m_size = len * hei;
        }
        m_attrs.clear();
    } else {
        uint8_t * newMap = new uint8_t[len * hei];
        memset(newMap, 0, len * hei * sizeof(newMap[0]));
        for (int y=0; y < hei; ++y) {
            memcpy(newMap + y * len,
                   m_map + y * m_len,
                   std::min(static_cast<uint8_t>(len), m_len));
        }
        delete [] m_map;
        m_map = newMap;
        m_size = len * hei;
    }
    m_len = len;
    m_hei = hei;
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
    if (m_attrs.size() > 0 && m_attrs.count(key) != 0)
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

const char *CMap::lastError()
{
    return m_lastError.c_str();
}

int CMap::size() {
    return m_size;
}

CMap & CMap::operator=(const CMap map)
{
    forget();
    m_len = map.m_len;
    m_hei = map.m_hei;
    m_map = new uint8_t[m_len * m_hei];
    memcpy(m_map, map.m_map, sizeof(m_map[0]) * map.m_len*map.m_hei);
    m_attrs = map.m_attrs;
    m_size = map.m_len*map.m_hei;
    return *this;
}
