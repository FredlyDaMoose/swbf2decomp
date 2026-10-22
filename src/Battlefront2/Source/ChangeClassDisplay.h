#pragma once

#include "Thread.h"

// VTABLE: SWBF2 0x00797110
class ChangeClassDisplay : public Thread
{
public:

    ChangeClassDisplay();

    virtual ~ChangeClassDisplay();

    // SYNTHETIC: SWBF2 0x004502d0
    // ChangeClassDisplay::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    static void Create();

    static void Destroy();

    static ChangeClassDisplay* sInstance[1];
};
 
