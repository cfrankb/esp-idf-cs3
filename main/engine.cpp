
#include <cstring>
#include <string>
#include <stdint.h>
#include "engine.h"
#include "game.h"
#include "colors.h"
#include "esphelpers.h"
#include "buffer.h"
#include "font.h"
#include "display.h"
#include "tileset.h"
#include "tilesdata.h"
#include "joystick.h"
#include "animzdata.h"
#include "maparch.h"

#define TILESIZE 16
#define DMA_BUFFER_LIMIT 2048 // 4092
// #define __SPEED_TEST__

CBuffer buffer(CONFIG_WIDTH, TILESIZE, DMA_BUFFER_LIMIT);
CFont font;
CDisplay display;
CTileSet tiles(TILESIZE, TILESIZE);
CTileSet animzTiles(TILESIZE, TILESIZE);
CTileSet playerTiles(TILESIZE, TILESIZE);
uint8_t tileReplacement[256];
std::mutex g_mutex;

typedef struct
{
    uint8_t srcTile;
    uint8_t startSeq;
    uint8_t count;
    uint8_t index;
} AnimzSeq;

AnimzSeq animzSeq[] = {
    {TILES_DIAMOND, ANIMZ_DIAMOND, 13, 0},
    {TILES_INSECT1, ANIMZ_INSECT1, 2, 0},
    {TILES_SWAMP, ANIMZ_SWAMP, 2, 0},
    {TILES_ALPHA, ANIMZ_ALPHA, 2, 0},
    {TILES_FORCEF94, ANIMZ_FORCEF94, 8, 0},
    {TILES_VAMPLANT, ANIMZ_VAMPLANT, 2, 0},
    {TILES_ORB, ANIMZ_ORB, 4, 0},
    {TILES_TEDDY93, ANIMZ_TEDDY93, 2, 0},
    {TILES_LUTIN, ANIMZ_LUTIN, 2, 0},
    {TILES_OCTOPUS, ANIMZ_OCTOPUS, 2, 0},
    {TILES_TRIFORCE, ANIMZ_TRIFORCE, 4, 0},
    {TILES_YAHOO, ANIMZ_YAHOO, 2, 0},
    {TILES_YIGA, ANIMZ_YIGA, 2, 0},
    {TILES_YELKILLER, ANIMZ_YELKILLER, 2, 0},
    {TILES_MANKA, ANIMZ_MANKA, 2, 0},
    {TILES_MAXKILLER, ANIMZ_MAXKILLER, 2, 0},
    {TILES_WHTEWORM, ANIMZ_WHTEWORM, 2, 0},
};

CEngine::CEngine()
{
    init();
}

CEngine::~CEngine()
{
    if (m_game)
    {
        delete m_game;
    }
}

CGame &CEngine::game()
{
    return *m_game;
}

void CEngine::drawLevelIntro()
{
    char t[32];
    switch (m_game->mode())
    {
    case CGame::MODE_INTRO:
        sprintf(t, "LEVEL %.2d", m_game->level() + 1);
        break;
    case CGame::MODE_RESTART:
        sprintf(t, "LIVES LEFT %.2d", m_game->lives());
        break;
    case CGame::MODE_GAMEOVER:
        strcpy(t, "GAME OVER");
    };

    int x = (CONFIG_WIDTH - strlen(t) * 8) / 2;
    int y = (CONFIG_HEIGHT - 8) / 2;
    display.fill(BLACK);
    buffer.fill(BLACK);
    buffer.drawFont(x, 0, font, t, WHITE);
    display.drawBuffer(0, y, buffer);
}

void CEngine::drawScreen()
{
    std::lock_guard<std::mutex> lk(g_mutex);
    CMap &map = m_game->getMap();
    CActor &player = m_game->player();

#ifdef __SPEED_TEST__
    int64_t t0 = timer_gettime();
#endif
    const int cols = CONFIG_WIDTH / TILESIZE;
    const int rows = CONFIG_HEIGHT / TILESIZE;
    const int lmx = std::max(0, player.getX() - cols / 2);
    const int lmy = std::max(0, player.getY() - rows / 2);
    const int mx = std::min(lmx, map.len() > cols ? map.len() - cols : 0);
    const int my = std::min(lmy, map.hei() > rows ? map.hei() - rows : 0);

    uint16_t *tiledata;
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            int i = y + my >= map.hei() ? TILES_BLANK : map.at(x + mx, y + my);
            if (i == TILES_ANNIE2)
            {
                tiledata = playerTiles[player.getAim() * 4 + x % 3];
            }
            else
            {
                if (i == TILES_STOP)
                {
                    i = TILES_BLANK;
                }
                int j = tileReplacement[i];
                tiledata = j == NO_ANIMZ ? tiles[i] : animzTiles[j];
            }
            buffer.drawTile32(x * TILESIZE, 0, tiledata);
        }
        if (y == 0)
        {
            char tmp[32];
            int bx = 0;
            int offsetY = 0;
            sprintf(tmp, "%.8d ", m_game->score());
            buffer.drawFont(0, offsetY, font, tmp, WHITE);
            bx += strlen(tmp);
            sprintf(tmp, "DIAMONDS %.2d ", m_game->diamonds());
            buffer.drawFont(bx * 8, offsetY, font, tmp, YELLOW);
            bx += strlen(tmp);
            sprintf(tmp, "LIVES %.2d ", m_game->lives());
            buffer.drawFont(bx * 8, offsetY, font, tmp, PURPLE);
        }
        else if (y == rows - 1)
        {
            buffer.drawRect(
                Rect{.x = 4, .y = 4, .width = std::min(m_game->health() / 2, CONFIG_WIDTH - 4), .height = 8}, LIME);
        }
        display.drawBuffer(0, y * TILESIZE, buffer);
    }

#ifdef __SPEED_TEST__
    int64_t t1 = timer_gettime();
    printf("time: %ld\n", (long)(t1 - t0) / 1000);
#endif
}

bool CEngine::init()
{
    m_game = new CGame();

    CTileSet::toggleFlipColors(true);
    initSpiffs();
    initJoystick();
    display.init();
    memset(tileReplacement, NO_ANIMZ, sizeof(tileReplacement));
    m_game->loadMapIndex("/spiffs/levels.mapz");

    tiles.read("/spiffs/tiles.mcz");
    animzTiles.read("/spiffs/animz.mcz");
    playerTiles.read("/spiffs/annie.mcz");

    if (!font.read("/spiffs/font.bin"))
    {
        printf("failed to read font\n");
        return false;
    }

    return true;
}

std::mutex &CEngine::mutex()
{
    return g_mutex;
}

void CEngine::animate()
{
    for (int i = 0; i < sizeof(animzSeq) / sizeof(AnimzSeq); ++i)
    {
        AnimzSeq &seq = animzSeq[i];
        int j = seq.srcTile;
        tileReplacement[j] = seq.startSeq + seq.index;
        seq.index = seq.index < seq.count - 1 ? seq.index + 1 : 0;
    }
}