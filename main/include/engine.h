#ifndef _engine__h
#define _engine__h

#include <mutex>

class CGame;
class CAnimator;

class CEngine
{
public:
    static CEngine *getEngine();
    ~CEngine();

    std::mutex &mutex();
    CGame &game();
    void drawScreen();
    void drawLevelIntro();
    void mainLoop(int ticks);

    enum
    {
        PLAYER_FRAMES = 8,
        NO_ANIMZ = 255,
    };

protected:
    CEngine();
    CAnimator *m_animator = nullptr;
    int m_playerFrameOffset = 0;
    int m_healthRef = 0;
    CGame *m_game = nullptr;
    bool init();
};

#endif