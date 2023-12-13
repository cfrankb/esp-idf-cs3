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

CActor &CGame::player()
{
    return m_player;
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
    const uint8_t pu = m_player.getPU();
    const TileDef &def = getTileDef(pu);

    if (def.type == TYPE_PICKUP)
    {
        addPoints(def.score);
        m_player.setPU(TILES_BLANK);
        addHealth(def.health);
    }
    else if (def.type == TYPE_KEY)
    {
        addPoints(def.score);
        m_player.setPU(TILES_BLANK);
        addKey(pu);
        addHealth(def.health);
    }
    else if (def.type == TYPE_DIAMOND)
    {
        addPoints(def.score);
        m_player.setPU(TILES_BLANK);
        --m_diamonds;
        addHealth(def.health);
    }
    else if (def.type == TYPE_SWAMP)
    {
        addHealth(def.health);
    }

    // apply flags
    if (def.flags & FLAG_EXTRA_LIFE)
    {
        addLife();
    }

    if (def.flags & FLAG_GODMODE)
    {
        m_godModeTimer = GODMODE_TIMER;
    }

    if (def.flags & FLAG_EXTRA_SPEED)
    {
        m_extraSpeedTimer = EXTRASPEED_TIMER;
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
    m_godModeTimer = 0;
    m_extraSpeedTimer = 0;
    loadLevel(true);
}

bool CGame::loadLevel(bool restart)
{
    printf("loading level: %d ...\n", m_level + 1);
    setMode(restart ? MODE_RESTART : MODE_INTRO);

    FILE *sfile = fopen(m_mapFile.c_str(), "rb");
    if (!sfile)
    {
        printf("can't read %s", m_mapFile.c_str());
        return false;
    }

    printf("maparch opened - levels: %d\n", m_arch.size());
    int i = m_level % m_arch.size();
    if (fseek(sfile, m_arch[i], SEEK_SET) != 0)
    {
        printf("can't seek %s to level %d", m_mapFile.c_str(), i + 1);
        return false;
    }
    if (!map.read(sfile))
    {
        printf("can't read map: %s\n", map.lastError());
        return false;
    }
    fclose(sfile);

    printf("level loaded\n");

    Pos pos = map.findFirst(TILES_ANNIE2);
    printf("Player at: %d %d\n", pos.x, pos.y);
    m_player = CActor(pos, TYPE_PLAYER, AIM_DOWN);
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

void CGame::manageMonsters(int ticks)
{
    const int speedCount = 9;
    bool speeds[speedCount];
    for (uint32_t i = 0; i < sizeof(speeds); ++i)
    {
        speeds[i] = i ? (ticks % i) == 0 : true;
    }

    uint8_t dirs[] = {AIM_UP, AIM_DOWN, AIM_LEFT, AIM_RIGHT};
    std::vector<CActor> newMonsters;

    for (int i = 0; i < m_monsterCount; ++i)
    {
        CActor &actor = m_monsters[i];
        uint8_t cs = map.at(actor.getX(), actor.getY());
        const TileDef &def = getTileDef(cs);
        if (!speeds[def.speed])
        {
            continue;
        }
        if (def.type == TYPE_MONSTER)
        {
            if (actor.isPlayerThere(actor.getAim()))
            {
                // apply health damages
                addHealth(def.health);
                if (def.ai & AI_STICKY)
                {
                    continue;
                }
            }

            int aim = actor.findNextDir();
            if (aim != AIM_NONE)
            {
                actor.move(aim);
                if (!(def.ai & AI_ROUND))
                {
                    continue;
                }
            }
            for (uint8_t i = 0; i < sizeof(dirs); ++i)
            {
                if (actor.isPlayerThere(dirs[i]))
                {
                    // apply health damages
                    addHealth(def.health);
                    if (def.ai & AI_FOCUS)
                    {
                        actor.setAim(dirs[i]);
                    }
                    break;
                }
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
                const uint8_t ct = map.at(p.x, p.y);
                const TileDef &defT = getTileDef(ct);
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
    m_godModeTimer = m_godModeTimer > 0 ? m_godModeTimer - 1 : 0;
    m_extraSpeedTimer = m_extraSpeedTimer > 0 ? m_extraSpeedTimer - 1 : 0;
    auto const pu = m_player.getPU();
    if (pu == TILES_SWAMP)
    {
        // apply health damage
        const TileDef &def = getTileDef(pu);
        addHealth(def.health);
    }

    uint16_t joy = readJoystick();

    // move player
    uint8_t mask = 1;
    uint8_t aims[] = {AIM_UP, AIM_DOWN, AIM_LEFT, AIM_RIGHT};
    for (uint8_t i = 0; i < AIM_COUNT; ++i)
    {
        uint8_t aim = aims[i];
        if ((joy & mask) && move(aim))
        {
            break;
        }
        mask += mask;
    }
}

Pos CGame::translate(const Pos &p, int aim)
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
    for (uint32_t i = 0; i < sizeof(m_keys); ++i)
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
    for (uint32_t i = 0; i < sizeof(m_keys); ++i)
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
                const TileDef &def = getTileDef(tile);
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
    else if (hp < 0 && !m_godModeTimer)
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

void CGame::restartGame()
{
    m_level = 0;
    m_score = 0;
    m_lives = DEFAULT_LIVES;
    m_nextLife = SCORE_LIFE;
    m_godModeTimer = 0;
    m_extraSpeedTimer = 0;
    loadLevel(false);
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

uint8_t *CGame::keys()
{
    return m_keys;
}

void CGame::addPoints(int points)
{
    m_score += points;
    if (m_score >= m_nextLife)
    {
        m_nextLife += SCORE_LIFE;
        addLife();
    }
}

void CGame::addLife()
{
    m_lives = std::min(m_lives + 1, static_cast<int>(MAX_LIVES));
}

int CGame::godModeTimer()
{
    return m_godModeTimer;
}

int CGame::playerSpeed()
{
    return m_extraSpeedTimer ? FAST_PLAYER_SPEED : DEFAULT_PLAYER_SPEED;
}

void CGame::getMonsters(CActor *&monsters, int &count)
{
    monsters = m_monsters;
    count = m_monsterCount;
}

CActor &CGame::getMonster(int i)
{
    return m_monsters[i];
}

bool CGame::loadMapIndex(const char *filename)
{
    printf("getting mapIndex\n");
    m_mapFile = filename;
    if (!CMapArch::indexFromFile(filename, m_arch))
    {
        printf("failed to get mapIndex from %s\n", filename);
        return false;
    }
    return true;
}