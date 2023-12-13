#ifndef __GAME_H
#define __GAME_H
#include <stdint.h>
#include "actor.h"
#include "map.h"
#include "maparch.h"

class CMap;
class CEngine;

class CGame
{
public:
    CGame();
    ~CGame();

    bool loadLevel(bool restart);
    bool move(int dir);
    void manageMonsters();
    void managePlayer();
    void animate();
    static Pos translate(const Pos p, int aim);
    void consume();
    static bool hasKey(uint8_t c);
    void addKey(uint8_t c);
    bool goalCount();

    static CMap &getMap();
    void nextLevel();
    void restartLevel();
    void restartGame();
    void setMode(int mode);
    int mode() const;
    bool isPlayerDead();
    void killPlayer();
    bool isGameOver();

    CActor &player();
    int score();
    int lives();
    int diamonds();
    int health();
    int level();
    int mode();
    bool loadMapIndex(const char *filename);
    std::string m_mapFile;

    enum
    {
        MODE_INTRO = 0,
        MODE_LEVEL = 1,
        MODE_RESTART = 2,
        MODE_GAMEOVER = 3,
        DEFAULT_LIVES = 5,
        LEVEL_BONUS = 500
    };

protected:
    IndexVector m_arch;
    int m_lives = 0;
    int m_health = 0;
    int m_level = 0;
    int m_levelCount = 0;
    int m_score = 0;
    int m_diamonds = 0;
    static uint8_t m_keys[6];

    CActor m_player;
    void clearAttr(uint8_t attr);

    // monsters
    enum
    {
        MAX_MONSTERS = 128,
        GROWBY_MONSTERS = 64,
        MAX_HEALTH = 255,
        DEFAULT_HEALTH = 64,
    };

    int m_mode;

    CActor *m_monsters;
    int m_monsterCount;
    int m_monsterMax;
    bool findMonsters();
    int addMonster(const CActor actor);
    int findMonsterAt(int x, int y);

    void addHealth(int hp);

    CEngine *m_engine;
};
#endif