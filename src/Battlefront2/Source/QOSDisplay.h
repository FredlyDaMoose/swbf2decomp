#pragma once

#include <stddef.h>

#include "Thread.h"

namespace QOSDisplay
{
void Create();
void Destroy();
void InitClient();
void InitHost();
void Reset();
}

// VTABLE: SWBF2 0x007adfd0
class QOSDisplayThread : public Thread
{
public:

    QOSDisplayThread();

    // SYNTHETIC: SWBF2 0x0062da70
    // QOSDisplayThread::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);
    virtual bool DisplayIcon(float fDeltaTime, int iUpdates, int iLag);

    float timer;
    int lastClientUpdateCount;
    unsigned int lastUpdates[64];
    unsigned int lastFills[64];
    unsigned int lastInBytes;
    unsigned int lastOutBytes;
};
 
 
 
 
 
 
 
