#ifndef CMAPARCH_H
#define CMAPARCH_H

#include <vector>
#include <string>
#include <inttypes.h>

class CMap;

typedef std::vector<long> IndexVector;

class CMapArch
{
public:
    CMapArch();
    virtual ~CMapArch();

    int size();
    const char* lastError();
    void forget();
    int add(CMap *map);
    CMap* removeAt(int i);
    void insertAt(int i, CMap *map);
    CMap *at(int i);
    bool read(const char *filename);
    bool extract(const char *filename);
    bool write(const char *filename);
    const char *signature();
    void removeAll();
    static bool indexFromFile(const char *filename, IndexVector & index);
    static bool indexFromMemory(uint8_t *ptr, IndexVector & index);

protected:
    void allocSpace();
    enum {
        GROW_BY = 5
    };
    int m_size;
    int m_max;
    CMap **m_maps;
    std::string m_lastError;
};

#endif // CMAPARCH_H
