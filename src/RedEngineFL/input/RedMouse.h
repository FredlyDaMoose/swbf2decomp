#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#include <stddef.h>

// VTABLE: SWBF2 0x007832a4
class RedMouse
{
public:

    RedMouse();

    virtual ~RedMouse();

    // SYNTHETIC: SWBF2 0x006da2b0
    // RedMouse::`scalar deleting destructor'

    void Init(unsigned char ucPortNum);
    void Insert();
    bool Exists();
    void Remove();

    void Acquire();
    void Cleanup();
    void SetCursorPos(int iX, int iY);
    void Update(float fDeltaTime);

    unsigned char m_portNum;
    bool m_connected;
    int m_dX;
    int m_dY;
    int m_dZ;
    int m_X;
    int m_Y;
    int m_Z;
    unsigned int m_uiDownButtons;
    unsigned int m_uiPressedButtons;
    unsigned int m_uiReleasedButtons;

    void* pCursorImage;

    unsigned int mUninitializedStorage;
    IDirectInputDevice8A* pc_device;
    DIMOUSESTATE2 pc_state;
};
 
 
 
 
