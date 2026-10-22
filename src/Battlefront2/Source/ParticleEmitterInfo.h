#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblRef.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// VTABLE: SWBF2 0x007ad36c
class ParticleEmitterInfoData : public PblRef
{
public:

    ParticleEmitterInfoData()
        : mNext(0), mPrev(0)
    {
    }

    static void* operator new(size_t size)
    {
        return sMemoryPool.Allocate((unsigned int)size);
    }

    static void operator delete(void* p)
    {
        sMemoryPool.Free(p);
    }

    static ParticleEmitterInfoData* Create();

    PblMatrix mMatrix;
    PblVector3 mVelocity;
    PblVector3 mInitialVelocity;
    float mStartTime;
    float mLifeTime;
    ParticleEmitterInfoData* mNext;
    ParticleEmitterInfoData* mPrev;

    static MemoryPool sMemoryPool;
};
 
 
 
 
 
