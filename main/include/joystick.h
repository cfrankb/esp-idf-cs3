#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>

enum JoyDir
{
    JOY_UP = 0x01,
    JOY_DOWN = 0x02,
    JOY_LEFT = 0x04,
    JOY_RIGHT = 0x08,
    JOY_BUTTON = 0x10,
    JOY_A_BUTTON = 0x20,
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
uint16_t readJoystick();

#endif