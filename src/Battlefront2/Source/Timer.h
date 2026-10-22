#pragma once

#include "Thread.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class Timer : public Thread, public PblHandled
{
public:

    Timer(const char* pcName);
    virtual ~Timer();
    virtual bool Update(float fDeltaTime);

    const char* GetName();

    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    // FUNCTION: SWBF2 0x0058aa70
    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }

    // FUNCTION: SWBF2 0x0058aab0
    void SetRate(float fRate) { mRate = fRate; }

    // FUNCTION: SWBF2 0x0058aa90
    void SetTime(float fTime) { mTime = fTime; }

    // FUNCTION: SWBF2 0x0055e320
    float GetTime() const { return mTime; }

    // FUNCTION: SWBF2 0x0058aaa0
    float GetRate() const { return mRate; }

    // FUNCTION: SWBF2 0x00657b60
    bool IsRunning() const { return const_cast<Timer*>(this)->IsThreadActive(); }

    void Start();
    void Stop();

    static void Clear();

    static Timer* Find(const char* pcName);

    unsigned int mId;
    float mTime;
    float mRate;

    static int sMapNumEntries;
    static PblHashTable<Timer, 64> sMap;
    static MemoryPool sMemoryPool;
};
