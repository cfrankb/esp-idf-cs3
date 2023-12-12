#include <cstring>
#include <string>
#include "game.h"
#include "map.h"
#include "joystick.h"
#include "level.h"
#include "actor.h"
#include "sprtypes.h"
#include "tilesdata.h"

CMap map(30, 30);

uint8_t CGame::m_keys[6];

CGame::CGame()
{
    m_monsterMax = MAX_MONSTERS;
    m_monsters = new CActor[m_monsterMax];
    m_monsterCount = 0;
    m_health = 0;
    m_lives = DEFAULT_LIVES;
    m_score = 0;
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
    uint8_t attr = map.getAttr(x, y);
    if (attr != 0)
    {
        map.setAttr(x, y, 0);
        clearAttr(attr);
    }
}

void CGame::nextLevel()
{
    printf("nextLevel\n");
    m_score += 500 + m_health;
    ++m_level;
    ++m_levelCount;
    if (!loadLevel(false))
    {
        m_level = 0;
        loadLevel(false);
    }
}

void CGame::restartLevel()
{
    loadLevel(true);
}

bool CGame::loadLevel(bool restart)
{
    printf("loading level: %d ...\n", m_level + 1);
    // std::lock_guard<std::mutex> lk(g_mutex);
    setMode(restart ? MODE_RESTART : MODE_INTRO);

    char target[20];
    sprintf(target, "level%.2d", m_level + 1);
    std::string fname = findLevel(target);
    if (fname[0] == '\0')
    {
        // no level found
        return false;
    }
    const char *path = "spiffs";
    char fpath[strlen(path) + fname.length() + 5];
    sprintf(fpath, "/%s/%s", path, fname.c_str());
    std::string error;
    if (!fetchLevel(map, fpath, error))
    {
        printf("error: %s\n", error.c_str());
        // unable to load level
        return false;
    }
    printf("level loaded\n");

    Pos pos = map.findFirst(TILES_ANNIE2);
    printf("Player at: %d %d\n", pos.x, pos.y);
    m_player = CActor(pos, TYPE_PLAYER, CActor::AIM_DOWN);
    m_diamonds = map.count(TILES_DIAMOND);
    memset(m_keys, 0, sizeof(m_keys));
    m_health = DEFAULT_HEALTH;
    findMonsters();

    return true;
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
    uint8_t dirs[] = {CActor::AIM_UP, CActor::AIM_DOWN, CActor::AIM_LEFT, CActor::AIM_RIGHT};
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
                // apply health damages
                addHealth(def.health);
            }

            int aim = actor.findNextDir();
            if (aim != CActor::AIM_NONE)
            {
                actor.move(aim);
            }
        }
        else if (def.type == TYPE_DRONE)
        {
            int aim = actor.getAim();
            if (aim < CActor::AIM_LEFT)
            {
                aim = CActor::AIM_LEFT;
            }
            if (actor.isPlayerThere(actor.getAim()))
            {
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
            for (uint8_t i = 0; i < sizeof(dirs); ++i)
            {
                Pos p = CGame::translate(Pos{actor.getX(), actor.getY()}, dirs[i]);
                uint8_t c = map.at(p.x, p.y);
                const TileDef &defT = getTileDef(c);
                if (defT.type == TYPE_PLAYER)
                {
                    // apply damage from config
                    addHealth(def.health);
                }
                else if (defT.type == TYPE_SWAMP)
                {
                    map.set(p.x, p.y, TILES_VAMPLANT);
                    newMonsters.push_back(CActor(p.x, p.y, TYPE_VAMPLANT));
                    break;
                }
                else if (defT.type == TYPE_MONSTER)
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
    for (auto const &monster : newMonsters)
    {
        addMonster(monster);
    }
}

void CGame::managePlayer()
{
    uint16_t joy = readJoystick();
    joy && ((joy & JOY_UP && move(CActor::AIM_UP)) ||
            (joy & JOY_DOWN && move(CActor::AIM_DOWN)) ||
            (joy & JOY_LEFT && move(CActor::AIM_LEFT)) ||
            (joy & JOY_RIGHT && move(CActor::AIM_RIGHT)));
}

Pos CGame::translate(const Pos p, int aim)
{
    Pos t = p;

    switch (aim)
    {
    case CActor::AIM_UP:
        if (t.y > 0)
        {
            --t.y;
        }
        break;
    case CActor::AIM_DOWN:
        if (t.y < map.hei() - 1)
        {
            ++t.y;
        }
        break;
    case CActor::AIM_LEFT:
        if (t.x > 0)
        {
            --t.x;
        }
        break;
    case CActor::AIM_RIGHT:
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

void CGame::clearAttr(uint8_t attr)
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

bool CGame::isPlayerDead()
{
    return m_health == 0;
}

void CGame::killPlayer()
{
    m_lives = m_lives ? m_lives - 1 : 0;
}

bool CGame::isGameOver()
{
    return m_lives == 0;
}

void CGame::restartGane()
{
    m_level = 0;
    m_score = 0;
    m_lives = DEFAULT_LIVES;
    loadLevel(false);
}

CActor &CGame::player()
{
    return m_player;
}

int CGame::score()
{
    return m_score;
}

int CGame::lives()
{
    return m_lives;
}

int CGame::diamonds()
{
    return m_diamonds;
}

int CGame::health()
{
    return m_health;
}

int CGame::level()
{
    return m_level;
}

int CGame::mode()
{
    return m_mode;
}