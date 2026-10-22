#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class Character;
class CommandPost;
class GameObject;

namespace AI {

enum AIGoalType
{
    AIGOAL_NONE = 0,
    AIGOAL_CONQUEST = 1,
    AIGOAL_DESTROY = 2,
    AIGOAL_DEFEND = 3,
    AIGOAL_DEATHMATCH = 4,
    AIGOAL_CTFOFFENSE = 5,
    AIGOAL_CTFDEFENSE = 6,
    AIGOAL_FOLLOW = 7,
    AIGOAL_COUNT = 8
};

enum GoalTargetType
{
    GTT_NONE = 0,
    GTT_POS = 1,
    GTT_CHARACTER = 2,
    GTT_GAMEOBJECT = 3,
    GTT_FLAG = 4
};

struct GoalTarget
{
    GoalTarget& operator=(const GoalTarget& other);
    GoalTarget()
    {
        mGameObj.mObject = 0;
        mGameObj.mSavedHandleId = 0;
    }

    GoalTarget(const PblVector3& vPos);
    GoalTarget(const GameObject* pObject);

    GoalTargetType mType;
    PblVector3 mPos;
    Character* mChr;
    PblHandle<const GameObject> mGameObj;

    bool GetLocation(PblVector3& vPos);

    const GameObject* GetGameObject();
};

// VTABLE: SWBF2 0x00796b6c
class AIGoal
{
public:

    AIGoal(AIGoalType eType, unsigned char uTeam, float fWeight);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~AIGoal();
    virtual void AddUnit(Character* pChr);
    virtual void Update();
    virtual CommandPost* GetSpawnLocation(const Character* pChr) const;

protected:
    virtual void Assign(Character* pChr);

    void SetEnabled(bool bEnabled);

public:

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    void UnAssignAll();

    unsigned char GetTeam() { return mTeam; }

    static PblHash rttiHashAIGoal;
    static char rttiNameAIGoal[];
    static MemoryPool sMemoryPool;

    PblList<Character> mUnitList;
    float mWeight;
    bool mEnabled;
    unsigned char mTeam;
};

}

 
 
 
 
 
