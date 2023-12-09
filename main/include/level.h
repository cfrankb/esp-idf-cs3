#ifndef __LEVEL_H__
#define __LEVEL_H__
#include <stdint.h>
#include <vector>
#include <string>
class CMap;

std::string str2upper(const std::string in);
typedef std::vector<std::string> StringVector;
uint8_t *readFile(const char *fname);
bool processLevel(CMap &map, const char *fname);
bool convertCs3Level(CMap &map, const char *fname);
void splitString(const std::string str, StringVector &list);
bool getChMap(const char *mapFile, char *chMap);
bool fetchLevel(CMap &map, const char *fname, std::string &error);
std::string findLevel(const char *target);
#endif
