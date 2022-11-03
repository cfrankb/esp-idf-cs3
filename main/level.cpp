#include <cstring>
#include <stdio.h>
#include "level.h"
#include "map.h"
#include "tilesdata.h"

std::string str2upper(const std::string in)
{
    char t[in.length() + 1];
    strcpy(t, in.c_str());
    char *p = t;
    while (*p)
    {
        if (*p == '.')
        {
            break;
        }
        *p = toupper(*p);
        ++p;
    }
    return t;
}

void splitString(const std::string str, StringVector &list)
{
    int i = 0;
    int j = 0;
    while (j < str.length())
    {
        if (isspace(str[j]))
        {
            list.push_back(str.substr(i, j - i));
            while (isspace(str[j]) && j < str.length())
            {
                ++j;
            }
            i = j;
            continue;
        }
        ++j;
    }
    list.push_back(str.substr(i, j - i));
}

uint8_t *readFile(const char *fname)
{
    FILE *sfile = fopen(fname, "rb");
    uint8_t *data = nullptr;
    if (sfile)
    {
        fseek(sfile, 0, SEEK_END);
        int size = ftell(sfile);
        fseek(sfile, 0, SEEK_SET);
        data = new uint8_t[size + 1];
        data[size] = 0;
        fread(data, size, 1, sfile);
        fclose(sfile);
    }
    else
    {
        printf("failed to read:%s\n", fname);
    }
    return data;
}

bool getChMap(const char *mapFile, char *chMap)
{
    uint8_t *data = readFile(mapFile);
    if (data == nullptr)
    {
        printf("cannot read %s\n", mapFile);
        return false;
    }

    char *p = reinterpret_cast<char *>(data);
    printf("parsing tiles.map: %d\n", strlen(p));
    int i = 0;

    while (p && *p)
    {
        char *n = strstr(p, "\n");
        if (n)
        {
            *n = 0;
            ++n;
        }
        StringVector list;
        splitString(std::string(p), list);
        u_int8_t ch = std::stoi(list[3], 0, 16);
        p = n;
        chMap[ch] = i;
        ++i;
    }

    delete[] data;
    return true;
}

bool processLevel(CMap &map, const char *fname)
{
    printf("reading file: %s\n", fname);
    uint8_t *data = readFile(fname);
    if (data == nullptr)
    {
        // delete[] data;
        printf("failed read: %s\n", fname);
        return false;
    }

    // get level size
    char *ps = reinterpret_cast<char *>(data);
    int maxRows = 0;
    int maxCols = 0;
    while (ps)
    {
        ++maxRows;
        char *u = strstr(ps, "\n");
        if (u)
        {
            *u = 0;
            maxCols = std::max(static_cast<int>(strlen(ps) + 1), maxCols);
            *u = '\n';
        }

        ps = u ? u + 1 : nullptr;
        //   printf("maxrows %d\n", maxRows);
    }
    printf("maxRows: %d, maxCols:%d\n", maxRows, maxCols);

    map.resize(maxCols, maxRows);
    map.clear();

    // convert ascii to map
    uint8_t *p = data;
    int x = 0;
    int y = 0;
    while (*p)
    {
        uint8_t c = *p;
        ++p;
        if (c == '\n')
        {
            ++y;
            x = 0;
            continue;
        }

        uint8_t m = getChTile(c);
        if (c != ' ' && m == 0)
        {
            printf("undefined %c found at %d %d.\n", c, x, y);
        }
        map.set(x, y, m);
        ++x;
    }
    delete[] data;
    return true;
}

const uint16_t convTable[] = {
    TILES_BLANK,
    TILES_WALL_BRICK,
    TILES_ANNIE2,
    TILES_STOP,
    TILES_DIAMOND,
    TILES_NECKLESS,
    TILES_CHEST,
    TILES_TRIFORCE,
    TILES_BOULDER,
    TILES_HEARTKEY,
    TILES_HEARTDOOR,
    TILES_GRAYKEY,
    TILES_GRAYDOOR,
    TILES_REDKEY,
    TILES_REDDOOR,
    TILES_POPKEY,
    TILES_POPDOOR,
    TILES_WALLS93,
    TILES_WALLS93_2,
    TILES_WALLS93_3,
    TILES_WALLS93_4,
    TILES_WALLS93_5,
    TILES_WALLS93_6,
    TILES_WALLS93_7,
    TILES_FLOWERS_2,
    TILES_PINETREE,
    TILES_ROCK1,
    TILES_ROCK2,
    TILES_ROCK3,
    TILES_TOMB,
    TILES_SWAMP,
    TILES_VAMPLANT,
    TILES_INSECT1,
    TILES_TEDDY93,
    TILES_OCTOPUS,
    TILES_BLANK,
    TILES_BLANK,
    TILES_BLANK,
    TILES_DIAMOND + 0x100,
    TILES_WALLS93_2 + 0x100,
    TILES_DIAMOND + 0x200,
    TILES_WALLS93_2 + 0x200,
    TILES_DIAMOND + 0x300,
    TILES_WALLS93_2 + 0x300,
    TILES_DIAMOND + 0x400,
    TILES_BLANK + 0x400,
    TILES_DIAMOND + 0x500,
    TILES_BLANK + 0x500,
    TILES_DIAMOND + 0x600,
    TILES_BLANK + 0x600,
};

bool convertCs3Level(CMap &map, const char *fname)
{
    uint8_t *data = readFile(fname);
    if (data == nullptr)
    {
        printf("failed read: %s\n", fname);
        return false;
    }

    map.clear();
    map.resize(64, 64);
    uint8_t *p = data + 7;
    for (int y = 0; y < 64; ++y)
    {
        for (int x = 0; x < 64; ++x)
        {
            const u_int8_t oldTile = *p;
            const u_int16_t data = convTable[oldTile];
            const u_int8_t attr = static_cast<u_int8_t>(data >> 8);
            const u_int8_t tile = static_cast<u_int8_t>(data);
            map.set(x, y, tile);
            map.setAttr(x, y, attr);
            ++p;
        }
    }

    delete[] data;
    return true;
}
