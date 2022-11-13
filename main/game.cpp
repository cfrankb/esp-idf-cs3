#include <cstring>
#include <mutex>
#include <string>
#include "game.h"
#include "display.h"
#include "esphelpers.h"
#include "tileset.h"
#include "map.h"
#include "buffer.h"
#include "joystick.h"
#include "level.h"
#include "actor.h"
#include "sprtypes.h"
#include "tilesdata.h"
#include "animzdata.h"
#include "font.h"
#include "colors.h"

#define elif else if
#define TILESIZE 16
//#define __SPEED_TEST__

CFont font;
CMap map(30, 30);
CDisplay display;
CTileSet tiles(TILESIZE, TILESIZE);
CTileSet animzTiles(TILESIZE, TILESIZE);
CTileSet playerTiles(TILESIZE, TILESIZE);
CBuffer buffer(CONFIG_WIDTH, TILESIZE, 1024 * 2);

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

uint8_t tileReplacement[256];

uint8_t CGame::m_keys[6];

CGame::CGame()
{
    CTileSet::toggleFlipColors(true);
    m_monsterMax = MAX_MONSTERS;
    m_monsters = new CActor[m_monsterMax];
    m_monsterCount = 0;
    m_health = 0;
}

CGame::~CGame()
{
    if (m_monsters)
    {
        delete[] m_monsters;
    }
}

CMap &CGame::getMap()
{
    return map;
}

bool CGame::move(int aim)
{
    if (m_player.canMove(aim))
    {
        m_player.move(aim);
        consume();
        return true;
    }

    return false;
}

void CGame::consume()
{
    uint8_t pu = m_player.getPU();
    const TileDef def = getTileDef(pu);

    if (def.type == TYPE_PICKUP)
    {
        m_score += def.score;
        m_player.setPU(TILES_BLANK);
        addHealth(def.health);
    }
    else if (def.type == TYPE_KEY)
    {
        m_score += def.score;
        m_player.setPU(TILES_BLANK);
        addKey(pu);
        addHealth(def.health);
    }
    else if (def.type == TYPE_DIAMOND)
    {
        m_score += def.score;
        m_player.setPU(TILES_BLANK);
        --m_diamonds;
        addHealth(def.health);
    }
    else if (def.type == TYPE_SWAMP)
    {
        addHealth(-1);
    }

    // trigger key
    int x = m_player.getX();
    int y = m_player.getY();
    u_int8_t attr = map.getAttr(x, y);
    if (attr != 0)
    {
        map.setAttr(x, y, 0);
        clearAttr(attr);
    }
}

bool CGame::init()
{
    initSpiffs();
    initJoystick();
    display.init();

    tiles.read("/spiffs/tiles.mcz");
    animzTiles.read("/spiffs/animz.mcz");
    playerTiles.read("/spiffs/annie.mcz");

    if (!font.read("/spiffs/font.bin"))
    {
        printf("failed to read font\n");
    }
    return true;
}

void CGame::nextLevel()
{
    printf("nextLevel\n");
    m_score += 500 + m_health;
    ++m_level;
    loadLevel();
}

bool CGame::loadLevel()
{
    printf("loading level: %d ...\n", m_level + 1);
    std::lock_guard<std::mutex> lk(g_mutex);
    setMode(MODE_INTRO);

    char target[20];
    char fpath[64];
    sprintf(target, "level%.2d", m_level + 1);
    std::string fname = findLevel(target);
    sprintf(fpath, "/spiffs/%s", fname.c_str());
    std::string error;
    if (!fetchLevel(map, fpath, error))
    {
        return false;
    }
    printf("level loaded\n");

    memset(tileReplacement, NO_ANIMZ, sizeof(tileReplacement));

    Pos pos = map.findFirst(TILES_ANNIE2);
    printf("Player at: %d %d\n", pos.x, pos.y);
    // map.read("/spiffs/level01.dat");
    m_player = CActor(pos, TYPE_PLAYER, AIM_DOWN);
    m_diamonds = map.count(TILES_DIAMOND);
    memset(m_keys, 0, sizeof(m_keys));

    m_health = DEFAULT_HEALTH;

    findMonsters();

    return true;
}

void CGame::animate()
{
    for (int i = 0; i < sizeof(animzSeq) / sizeof(AnimzSeq); ++i)
    {
        AnimzSeq &seq = animzSeq[i];
        int j = seq.srcTile;
        tileReplacement[j] = seq.startSeq + seq.index;
        ++seq.index;
        seq.index = seq.index % seq.count;
    }
}

void CGame::drawLevelIntro()
{
    char t[32];
    sprintf(t, "LEVEL %.2d", m_level + 1);

    int x = (CONFIG_WIDTH - strlen(t) * 8) / 2;
    int y = (CONFIG_HEIGHT - 8) / 2;
    display.fill(BLACK);
    buffer.fill(BLACK);
    buffer.drawFont(x, 0, font, t, WHITE);
    display.drawBuffer(0, y, buffer);
}

void CGame::drawScreen()
{
    std::lock_guard<std::mutex> lk(g_mutex);

#ifdef __SPEED_TEST__
    int64_t t0 = esp_timer_get_time();
#endif
    const int cols = CONFIG_WIDTH / TILESIZE;
    const int rows = CONFIG_HEIGHT / TILESIZE;
    const int lmx = std::max(0, m_player.getX() - cols / 2);
    const int lmy = std::max(0, m_player.getY() - rows / 2);
    const int mx = std::min(lmx, map.len() > cols ? map.len() - cols : 0);
    const int my = std::min(lmy, map.hei() > rows ? map.hei() - rows : 0);

    uint16_t *tiledata;
    for (int y = 0; y < rows; ++y)
    {
        if (y + my >= map.hei())
        {
            break;
        }

        for (int x = 0; x < cols; ++x)
        {
            int i = map.at(x + mx, y + my);
            if (i == TILES_ANNIE2)
            {
                tiledata = playerTiles[m_player.getAim() * 4 + x % 3];
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
            sprintf(tmp, "SCORE %.8d ", m_score);
            buffer.drawFont(0, 0, font, tmp, WHITE);
            bx += strlen(tmp);
            sprintf(tmp, "DIAMONDS %.2d", m_diamonds);
            buffer.drawFont(bx * 8, 0, font, tmp, YELLOW);
        }
        else if (y == rows - 1)
        {
            buffer.drawRect(
                Rect{.x = 4, .y = 4, .width = std::min(m_health / 2, CONFIG_WIDTH - 4), .height = 8}, LIME);
        }
        display.drawBuffer(0, y * TILESIZE, buffer);
    }

#ifdef __SPEED_TEST__
    int64_t t1 = esp_timer_get_time();
    printf("time: %ld\n", (long)(t1 - t0) / 1000);
#endif
}

bool CGame::findMonsters()
{
    m_monsterCount = 0;
    for (int y = 0; y < map.hei(); ++y)
    {
        for (int x = 0; x < map.len(); ++x)
        {
            uint8_t c = map.at(x, y);
            const TileDef &def = getTileDef(c);
            if (def.type == TYPE_MONSTER ||
                def.type == TYPE_VAMPLANT ||
                def.type == TYPE_DRONE)
            {
                addMonster(CActor(x, y, def.type));
            }
        }
    }
    printf("%d monsters found.\n", m_monsterCount);
    return true;
}

int CGame::addMonster(const CActor actor)
{
    if (m_monsterCount >= m_monsterMax)
    {
        m_monsterMax += GROWBY_MONSTERS;
        CActor *t = new CActor[m_monsterMax];
        memcpy(reinterpret_cast<void *>(t), m_monsters, m_monsterCount * sizeof(CActor));
        delete[] m_monsters;
        m_monsters = t;
    }
    m_monsters[m_monsterCount++] = actor;
    return m_monsterCount;
}

int CGame::findMonsterAt(int x, int y)
{
    for (int i = 0; i < m_monsterCount; ++i)
    {
        const CActor &actor = m_monsters[i];
        if (actor.getX() == x && actor.getY() == y)
        {
            return i;
        }
    }
    return -1;
}

void CGame::manageMonsters()
{
    u_int8_t dirs[] = {AIM_UP, AIM_DOWN, AIM_LEFT, AIM_RIGHT};
    std::vector<CActor> newMonsters;

    for (int i = 0; i < m_monsterCount; ++i)
    {
        CActor &actor = m_monsters[i];
        uint8_t c = map.at(actor.getX(), actor.getY());
        const TileDef &def = getTileDef(c);
        if (def.type == TYPE_MONSTER)
        {
            if (actor.isPlayerThere(actor.getAim()))
            {
                // attack player
                // apply health damages
                addHealth(def.health);
            }

            int aim = actor.findNextDir();
            if (aim != AIM_NONE)
            {
                actor.move(aim);
            }
        }
        else if (def.type == TYPE_DRONE)
        {
            int aim = actor.getAim();
            if (aim < AIM_LEFT)
            {
                aim = AIM_LEFT;
            }
            if (actor.isPlayerThere(actor.getAim()))
            {
                // attack player
                // apply health damages
                addHealth(def.health);
            }
            if (actor.canMove(aim))
            {
                actor.move(aim);
            }
            else
            {
                aim ^= 1;
            }
            actor.setAim(aim);
        }
        else if (def.type == TYPE_VAMPLANT)
        {
            for (u_int8_t i = 0; i < sizeof(dirs); ++i)
            {
                Pos p = CGame::translate(Pos{actor.getX(), actor.getY()}, dirs[i]);
                uint8_t c = map.at(p.x, p.y);
                const TileDef &defT = getTileDef(c);
                if (defT.type == TYPE_PLAYER)
                {
                    // apply damage from config
                    addHealth(def.health);
                }
                elif (defT.type == TYPE_SWAMP)
                {
                    map.set(p.x, p.y, TILES_VAMPLANT);
                    newMonsters.push_back(CActor(p.x, p.y, TYPE_VAMPLANT));
                    break;
                }
                elif (defT.type == TYPE_MONSTER)
                {
                    int j = findMonsterAt(p.x, p.y);
                    if (j == -1)
                        continue;
                    CActor &m = m_monsters[j];
                    m.setType(TYPE_VAMPLANT);
                    map.set(p.x, p.y, TILES_VAMPLANT);
                    break;
                }
            }
        }
    }

    // moved here to avoid reallocation while using a reference
    for (auto const monster : newMonsters)
    {
        addMonster(monster);
    }
}

void CGame::managePlayer()
{
    int joy;
    int aim;
    readJoystick(joy, aim);
    joy && ((joy & JOY_UP && move(AIM_UP)) ||
            (joy & JOY_DOWN && move(AIM_DOWN)) ||
            (joy & JOY_LEFT && move(AIM_LEFT)) ||
            (joy & JOY_RIGHT && move(AIM_RIGHT)));
}

Pos CGame::translate(const Pos p, int aim)
{
    Pos t = p;

    switch (aim)
    {
    case AIM_UP:
        if (t.y > 0)
        {
            --t.y;
        }
        break;
    case AIM_DOWN:
        if (t.y < map.hei() - 1)
        {
            ++t.y;
        }
        break;
    case AIM_LEFT:
        if (t.x > 0)
        {
            --t.x;
        }
        break;
    case AIM_RIGHT:
        if (t.x < map.len() - 1)
        {
            ++t.x;
        }
    }

    return t;
}

bool CGame::hasKey(uint8_t c)
{
    for (int i = 0; i < sizeof(m_keys); ++i)
    {
        if (m_keys[i] == c)
        {
            return true;
        }
    }
    return false;
}

void CGame::addKey(uint8_t c)
{
    for (int i = 0; i < sizeof(m_keys); ++i)
    {
        if (m_keys[i] == c)
        {
            break;
        }
        if (m_keys[i] == '\0')
        {
            m_keys[i] = c;
            break;
        }
    }
}

bool CGame::goalCount()
{
    return m_diamonds;
}

void CGame::clearAttr(u_int8_t attr)
{
    for (int y = 0; y < map.hei(); ++y)
    {
        for (int x = 0; x < map.len(); ++x)
        {
            const uint8_t tileAttr = map.getAttr(x, y);
            if (tileAttr == attr)
            {
                const uint8_t tile = map.at(x, y);
                const TileDef def = getTileDef(tile);
                if (def.type == TYPE_DIAMOND)
                {
                    --m_diamonds;
                }
                map.set(x, y, TILES_BLANK);
                map.setAttr(x, y, 0);
            }
        }
    }
}

void CGame::addHealth(int hp)
{
    if (hp > 0)
    {
        m_health = std::min(m_health + hp, static_cast<int>(MAX_HEALTH));
    }
    else if (hp < 0)
    {
        m_health = std::max(m_health + hp, 0);
    }
}

void CGame::setMode(int mode)
{
    m_mode = mode;
}

int CGame::mode() const
{
    return m_mode;
};
