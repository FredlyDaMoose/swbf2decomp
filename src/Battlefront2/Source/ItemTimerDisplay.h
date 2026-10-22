#pragma once

#include <stddef.h>

#include "Thread.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"

class EntityItem;

namespace ItemTimerDisplay
{

// VTABLE: SWBF2 0x007a3914
class DisplayThread : public Thread
{
public:

    DisplayThread() {}

    // SYNTHETIC: SWBF2 0x0056a7b0
    // ItemTimerDisplay::DisplayThread::`scalar deleting destructor'
    virtual ~DisplayThread();

    virtual bool Update(float fDeltaTime);

    int mJoystickIndex;
    int mItemTimerCount;
    EntityItem* mItemArray[8];
    RedScreenGroupElement mItemGroup[8];
    RedTextElement mItemText[8];
};

void Create();
void Destroy();
void Init();
void Reset();
void AddItemTimer(EntityItem* pItem);
void RemoveItemTimer(EntityItem* pItem);
void HideAll();

}

 
 
 
 
 
