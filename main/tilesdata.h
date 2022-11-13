//////////////////////////////////////////////////
// autogenerated

#ifndef _TILES__HDR_H
#define _TILES__HDR_H

#include <stdint.h>

typedef struct
{
    uint8_t ch;
    uint8_t type;
    uint8_t score;
    int8_t health;
    uint8_t hidden;
    const char * basename;
} TileDef;
uint8_t getChTile(uint8_t i) ;
const TileDef * getTileDefs();
const TileDef & getTileDef(int i);

#define TILES_BLANK          0x00
#define TILES_STOP           0x01
#define TILES_ANNIE2         0x02
#define TILES_WALLS93        0x03
#define TILES_WALLS93_2      0x04
#define TILES_WALLS93_3      0x05
#define TILES_WALLS93_4      0x06
#define TILES_WALLS93_5      0x07
#define TILES_WALLS93_6      0x08
#define TILES_WALLS93_7      0x09
#define TILES_ROCK1          0x0a
#define TILES_ROCK2          0x0b
#define TILES_ROCK3          0x0c
#define TILES_WALL_BRICK     0x0d
#define TILES_WALL_BRICK_2   0x0e
#define TILES_WALL_BRICK_3   0x0f
#define TILES_WALL_BRICK_4   0x10
#define TILES_WALL_BRICK_5   0x11
#define TILES_WALL_BRICK_6   0x12
#define TILES_WALL_BRICK_7   0x13
#define TILES_COLORWALLS     0x14
#define TILES_COLORWALLS_2   0x15
#define TILES_COLORWALLS_3   0x16
#define TILES_COLORWALLS_4   0x17
#define TILES_COLORWALLS_5   0x18
#define TILES_COLORWALLS_6   0x19
#define TILES_COLORWALLS_7   0x1a
#define TILES_COLORWALLS_8   0x1b
#define TILES_COLORWALLS_9   0x1c
#define TILES_COLORWALLS_A   0x1d
#define TILES_COLORWALLS_B   0x1e
#define TILES_COLORWALLS_C   0x1f
#define TILES_COLORWALLS_D   0x20
#define TILES_COLORWALLS_E   0x21
#define TILES_COLORWALLS_F   0x22
#define TILES_COLORWALLS_10  0x23
#define TILES_PINETREE       0x24
#define TILES_PLANTS         0x25
#define TILES_PLANTS_2       0x26
#define TILES_PLANTS_3       0x27
#define TILES_PLANTS_4       0x28
#define TILES_PLANTS_5       0x29
#define TILES_PLANTS_6       0x2a
#define TILES_PLANTS_7       0x2b
#define TILES_THISWAY        0x2c
#define TILES_THISWAY_2      0x2d
#define TILES_FRUIT1         0x2e
#define TILES_NECKLESS       0x2f
#define TILES_APPLE          0x30
#define TILES_BOOK           0x31
#define TILES_BALLON1        0x32
#define TILES_CAROTTE        0x33
#define TILES_CAROTTE_2      0x34
#define TILES_CHEST          0x35
#define TILES_STATUE         0x36
#define TILES_TOMB           0x37
#define TILES_MUSHROOM       0x38
#define TILES_SMALL_MUSH     0x39
#define TILES_SMALL_MUSH_2   0x3a
#define TILES_SMALL_MUSH_3   0x3b
#define TILES_SMALL_MUSH_4   0x3c
#define TILES_POT            0x3d
#define TILES_POT_2          0x3e
#define TILES_JAR1           0x3f
#define TILES_WATERMEL       0x40
#define TILES_BOULDER        0x41
#define TILES_BOULDER_2      0x42
#define TILES_MAGICBOX       0x43
#define TILES_MAGICBOT       0x44
#define TILES_LIGHTBUL       0x45
#define TILES_1ST_AID        0x46
#define TILES_ROPE           0x47
#define TILES_SHIELD         0x48
#define TILES_VIALS          0x49
#define TILES_VIALS_2        0x4a
#define TILES_VIALS_3        0x4b
#define TILES_POIRE          0x4c
#define TILES_CLOVER         0x4d
#define TILES_PUMPKIN        0x4e
#define TILES_FLOWERS        0x4f
#define TILES_FLOWERS_2      0x50
#define TILES_HEARTDOOR      0x51
#define TILES_HEARTKEY       0x52
#define TILES_GRAYDOOR       0x53
#define TILES_GRAYKEY        0x54
#define TILES_POPDOOR        0x55
#define TILES_POPKEY         0x56
#define TILES_REDDOOR        0x57
#define TILES_REDKEY         0x58
#define TILES_YELDOOR        0x59
#define TILES_YELKEY         0x5a
#define TILES_SWAMP          0x5b
#define TILES_DIAMOND        0x5c
#define TILES_TRIFORCE       0x5d
#define TILES_ORB            0x5e
#define TILES_TNTSTICK       0x5f
#define TILES_ALPHA          0x60
#define TILES_BLUEGHOS       0x61
#define TILES_DEICO          0x62
#define TILES_FORCEF94       0x63
#define TILES_FORCEFIH       0x64
#define TILES_FORCEFIV       0x65
#define TILES_INSECT1        0x66
#define TILES_LUTIN          0x67
#define TILES_MANKA          0x68
#define TILES_MAXKILLER      0x69
#define TILES_OCTOPUS        0x6a
#define TILES_OOO            0x6b
#define TILES_TEDDY93        0x6c
#define TILES_VAMPLANT       0x6d
#define TILES_YAHOO          0x6e
#define TILES_YIGA           0x6f
#define TILES_YELKILLER      0x70
#define TILES_WHTEWORM       0x71
#define TILES_ETURTLE        0x72
#define TILES_DRAGO          0x73
#define TILES_BIRD           0x74

#endif
