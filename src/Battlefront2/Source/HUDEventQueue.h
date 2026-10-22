#pragma once

#include "HUDEvent.h"
#include "Thread.h"

namespace HUD {

struct DelayedEvent
{
    Event e;
    float activationTime;
};

extern DelayedEvent gEventList[32];

void RemoveAllEvents();

class EventQueue : public Thread
{
public:

    EventQueue();
    virtual ~EventQueue();
    virtual bool Update(float fDeltaTime);

    static void Create();

    static void Destroy();

    static void AddEvent(const Event* pEvent, float fDelay);

    static EventQueue sEventQueue;
};

 

}

 
