#pragma once

#include <stddef.h>

#include "Thread.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class PblSphere;

// VTABLE: SWBF2 0x007a3bb8 Thread
// VTABLE: SWBF2 0x007a3b94 RedOmniLight
class LightFlash : public Thread, public RedOmniLight
{
public:
    static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    LightFlash(const PblSphere& sphere, const RedColor& color, float fLifeTime);
    virtual ~LightFlash();

    virtual bool Update(float fDeltaTime);

    RedColor m_color0;
    float m_fLifeTime;
    float m_fTimer;

    static MemoryPool sMemoryPool;
};
 
 
 
 
