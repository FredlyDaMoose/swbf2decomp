#pragma once

#include "Thread.h"

// VTABLE: SWBF2 0x007af300
class StatIconDisplay : public Thread
{
public:

    StatIconDisplay();

    virtual ~StatIconDisplay();

    // SYNTHETIC: SWBF2 0x00650870
    // StatIconDisplay::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    static void Create();

    static void Destroy();

    static StatIconDisplay* sInstance[1];
};
 
