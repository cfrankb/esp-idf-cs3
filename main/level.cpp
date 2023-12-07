#include <cstring>
#include <stdio.h>
#include <dirent.h>
#include "level.h"
#include "map.h"
#include "tilesdata.h"

#define CS3_MAP_WIDTH 64
#define CS3_MAP_HEIGHT 64
#define CS3_MAP_OFFSET 7

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
    unsigned int j = 0;
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
    printf("parsing tiles.map: %zull\n", strlen(p));
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
        uint8_t ch = std::stoi(list[3], 0, 16);
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
    }
    printf("maxRows: %d, maxCols:%d\n", maxRows, maxCols);

    map.resize(maxCols, maxRows, true);
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
    TILES_BLANK + 0x600, // 0x31
    TILES_BLANK,         // 0x32
    TILES_BLANK,         // 0x33
    TILES_BLANK,         // 0x34
    TILES_YAHOO,         // 0x35
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
    map.resize(CS3_MAP_WIDTH, CS3_MAP_HEIGHT, true);
    uint8_t *p = data + CS3_MAP_OFFSET;
    for (int y = 0; y < CS3_MAP_HEIGHT; ++y)
    {
        for (int x = 0; x < CS3_MAP_WIDTH; ++x)
        {
            uint8_t oldTile = *p;
            if (oldTile >= sizeof(convTable) / sizeof(convTable[0]))
            {
                printf("oldTile: %d\n", oldTile);
                oldTile = 0;
            }
            const uint16_t data = convTable[oldTile];
            const uint8_t attr = static_cast<uint8_t>(data >> 8);
            const uint8_t tile = static_cast<uint8_t>(data & 0xff);
            map.set(x, y, tile);
            map.setAttr(x, y, attr);
            ++p;
        }
    }

    delete[] data;
    return true;
}

bool fetchLevel(CMap &map, const char *fname, std::string &error)
{
    char tmp[strlen(fname) + 128];
    printf("fetching: %s\n", fname);

    FILE *sfile = fopen(fname, "rb");
    if (!sfile)
    {
        sprintf(tmp, "can't open file: %s", fname);
        error = tmp;
        return false;
    }

    char sig[4];
    fread(sig, sizeof(sig), 1, sfile);
    if (memcmp(sig, "MAPZ", 4) == 0)
    {
        fclose(sfile);
        printf("level is MAPZ\n");
        return map.read(fname);
    }

    fseek(sfile, 0, SEEK_END);
    int size = ftell(sfile);
    const int cs3LevelSize = CS3_MAP_WIDTH * CS3_MAP_HEIGHT + CS3_MAP_OFFSET;
    if (size == cs3LevelSize)
    {
        fclose(sfile);
        printf("level is cs3\n");
        return convertCs3Level(map, fname);
    }

    fclose(sfile);
    printf("level is map\n");
    return processLevel(map, fname);
}

std::string findLevel(const char *target)
{
    DIR *dir;
    char *fname = nullptr;

    struct dirent *diread;
    if ((dir = opendir("/spiffs")) != nullptr)
    {
        while ((diread = readdir(dir)) != nullptr)
        {
            if (strstr(diread->d_name, target))
            {
                fname = diread->d_name;
                break;
            }
        }
        closedir(dir);
    }
    else
    {
        perror("opendir");
    }
    return std::string(fname ? fname : "");
}
