#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"

class RedJoystick;
class RedKeyboard;
class RedMouse;

enum RedInputEventType
{
    KEYDOWN = 0,
    KEYUP = 1,
    KEYCHAR = 2,
    JOYSTICKMOVE = 3,
    JOYSTICKBUTTON = 4,
    MOUSEBUTTONDOWN = 5,
    MOUSEBUTTONUP = 6,
    MOUSEBUTTONDBLCLK = 7,
    MOUSEBUTTONDBLCLKUP = 8,
    MOUSEMOVE = 9,
    MOUSEAXIS = 10
};

// VTABLE: SWBF2 0x00783e20

class RedInputEventHandler
{
public:

    // FUNCTION: SWBF2 0x006efde0
    RedInputEventHandler();

    // SYNTHETIC: SWBF2 0x006efe20
    // RedInputEventHandler::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x006efe10
    virtual ~RedInputEventHandler();

    // FUNCTION: SWBF2 0x00527f00
    virtual void ButtonEvent(unsigned int, float) {}

    // FUNCTION: SWBF2 0x00529ac0
    virtual void KeyboardEvent(RedInputEventType, RedKeyboard*, unsigned char,
                               unsigned char, unsigned short) {}

    // FUNCTION: SWBF2 0x00527f10
    virtual void JoystickEvent(RedInputEventType, RedJoystick*) {}

    // FUNCTION: SWBF2 0x00527f20
    virtual void MouseEvent(RedInputEventType, RedMouse*) {}

    // FUNCTION: SWBF2 0x006efe50
    static void AddEventHandler(RedInputEventHandler* pHandler);

    // FUNCTION: SWBF2 0x006eff00
    static void RemoveEventHandler(RedInputEventHandler* pHandler);

    // FUNCTION: SWBF2 0x006efeb0
    static void PostKeyboardEvent(RedInputEventType eType, RedKeyboard* pKeyboard,
                                  unsigned char ucKey, unsigned char ucAscii,
                                  unsigned short usFlags);

    PblList<RedInputEventHandler>::Node m_node;

    static PblList<RedInputEventHandler> m_handlerList;
};

 
 
