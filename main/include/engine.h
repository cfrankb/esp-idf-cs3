#ifndef _engine__h
#define _engine__h

#include <mutex>

class CGame;

class CEngine
{
public:
    CEngine();
    ~CEngine();

    std::mutex &mutex();
    CGame &game();
    void drawScreen();
    void drawLevelIntro();
    void animate();

    enum
    {
        NO_ANIMZ = 255,
    };

protected:
    CGame *m_game = nullptr;
    bool init();
};

#endif