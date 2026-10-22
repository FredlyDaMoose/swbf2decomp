#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#include <stddef.h>

// VTABLE: SWBF2 0x00772ca8
class RedKeyboard
{
public:

    RedKeyboard();

    virtual ~RedKeyboard();

    // SYNTHETIC: SWBF2 0x006c60d0
    // RedKeyboard::`scalar deleting destructor'

    static void StaticInit();
    void Init(unsigned char ucPortNum);
    void Insert();

    void Remove();
    void Cleanup();
    void Update(float fDeltaTime);
    bool IsKeyDown(unsigned char ucKey);
    unsigned short GetFlags();
    unsigned char pcGetAsciiValue(unsigned int ui);

    bool pcGetKeyName(unsigned int uiKey, unsigned short* pusName, unsigned int uiSize);

protected:

    unsigned short pcGetFlags();

public:

    bool m_keyState[256];
    bool m_connected;
    unsigned char m_portNum;
    IDirectInputDevice8A* pc_pKeyboardDevice;
    unsigned char pc_DownKeys[256];
    unsigned char pc_PressedKeys[256];
    unsigned char pc_ReleasedKeys[256];
    char pc_curRepeatCode;
    float pc_keyRepeatElapsedTime;
    float pc_nextKeyRepeatPress;
    bool pc_mCapslockMode;
};
 
 
 
 
 
