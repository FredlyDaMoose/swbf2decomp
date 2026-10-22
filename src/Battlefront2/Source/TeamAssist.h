#pragma once

#include <stddef.h>
#include "Thread.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class Character;
class GameObject;
class TeamAssistManager;

class TeamAssist
{
    friend class TeamAssistManager;

public:

    static void* operator new(size_t size)
    {
        return sMemoryPool.Allocate((unsigned int)size);
    }
    static void operator delete(void* p)
    {
        sMemoryPool.Free(p);
    }

    ~TeamAssist();

    static MemoryPool sMemoryPool;

    static unsigned int sColorHash;

private:

    TeamAssist(TeamAssist* pNext, int type, Character* pRequestor, float fTimer);

    bool Update(float fDeltaTime);

    TeamAssist* mNext;
    int mType;
    GameObject* mRequestor;
    float mTimer;
    int mTeamMask;
};
 

// VTABLE: SWBF2 0x007af57c
class TeamAssistManager : public Thread
{
public:

    enum Code
    {
        None = -1,
        Spotted = 0,
        NeedMedic = 1,
        NeedRepair = 2,
        NeedAmmo = 3,
        NeedPickup = 4,
        NeedBackup = 5,
        AttackPosition = 6,
        DefendPosition = 7,
        Max = 8
    };

    TeamAssistManager();
    virtual ~TeamAssistManager();
    virtual bool Update(float fDeltaTime);

    static void Create();
    static void Destroy();

    static void AddRequest(Code code, Character* pCharacter);

    static TeamAssistManager* sInstance;
    static float sDuration;

private:

    void DeleteRest(TeamAssist* pAssist);

    TeamAssist* mList;
};
 
