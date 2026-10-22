#pragma once

bool __fastcall HasNameDisplayPilotType(const void* pCharacter);

#include <stddef.h>

#include "Thread.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"

// VTABLE: SWBF2 0x007aa76c
class NameDisplayThread : public Thread
{
public:
    NameDisplayThread();
    virtual ~NameDisplayThread();
    virtual bool Update(float fDeltaTime);

    int mJoystickIndex;
    RedScreenGroupElement mPlayerNames[64];
    RedTextElement mPlayerNamesText[64];
};
 
 
 
 

namespace NameDisplay
{
void Create();
void Destroy();
void Init();
void Reset();
}

extern NameDisplayThread* sNameDisplayThread[1];
