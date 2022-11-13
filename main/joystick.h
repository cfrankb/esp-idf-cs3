#ifndef JOYSTICK_H
#define JOYSTICK_H

enum JoyDir
{
    JOY_UP = 1,
    JOY_DOWN = 2,
    JOY_LEFT = 4,
    JOY_RIGHT = 8,
    JOY_BUTTON = 16,
    JOY_NONE = 0
};

enum JoyAim
{
    AIM_UP = 0,
    AIM_DOWN = 1,
    AIM_LEFT = 2,
    AIM_RIGHT = 3,
    AIM_NONE = -1
};

bool initJoystick();
bool readJoystick(int &joy, int &aim);

#endif