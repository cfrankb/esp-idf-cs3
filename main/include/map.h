#ifndef __MAP_H
#define __MAP_H

#include <stdint.h>
#include <cstdio>
#include <unordered_map>
#include <string>

typedef std::unordered_map<uint16_t, uint8_t> AttrMap;

typedef struct
{
    uint16_t x;
    uint16_t y;
} Pos;

class CMap
{
public:
    CMap(uint16_t len = 0, uint16_t hei = 0, uint8_t t = 0);
    ~CMap();
    uint8_t &at(int x, int y);
    uint8_t *row(int y);
    void set(int x, int y, uint8_t t);
    bool read(const char *fname);
    bool write(const char *fname);
    bool read(FILE *sfile);
    bool write(FILE *tfile);
    void forget();
    int len() const;
    int hei() const;
    bool resize(uint16_t len, uint16_t hei, bool fast);
    const Pos findFirst(uint8_t tileId);
    int count(uint8_t tileId);
    void clear(uint8_t ch = 0);
    uint8_t getAttr(const uint8_t x, const uint8_t y);
    void setAttr(const uint8_t x, const uint8_t y, const uint8_t a);
    int size();
    const char *lastError();
    CMap &operator=(const CMap &map);
    bool fromMemory(uint8_t *mem);
    bool fromStream(FILE *sfile, uint16_t len, uint16_t hei);

    enum : uint16_t
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        MAX = RIGHT,
        NOT_FOUND = 0xffff
    };
    void shift(int aim);
    void debug();

protected:
    uint16_t m_len;
    uint16_t m_hei;
    uint8_t *m_map;
    int m_size;
    AttrMap m_attrs;
    std::string m_lastError;
    static uint16_t toKey(const uint8_t x, const uint8_t y);
};

#endif
