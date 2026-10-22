#pragma once

#include "Battlefront2/Source/FLJoystick.h"

struct UiBindKeys
{
    eGUIINPUT_TYPE mKeyA;
    eGUIINPUT_TYPE mKeyB;
};

extern const int RedKeyTo_BindID[144];

extern bool gInvertMouse;

unsigned short __cdecl RawInputToActionKey(unsigned short usRawInput);
unsigned char __cdecl ActionKeyToRawInput(unsigned short usActionKey);
