#pragma once

#include <stddef.h>

#include "Thread.h"
#include "AIGoal.h"

class Character;
class CommandPost;

namespace AI {

// VTABLE: SWBF2 0x00796b90
class AIGoalManager : public Thread
{
public:
    enum { MAX_GOALS = 20 };

    static void Create();
    static void Destroy();
    static AIGoalManager* Instance() { return mInstance; }

    virtual ~AIGoalManager();

    // SYNTHETIC: SWBF2 0x0043a510
    // AI::AIGoalManager::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    AIGoal* AddGoal(unsigned char uTeam, AIGoalType eType, float fWeight,
                    const GoalTarget* pTarget);
    void DeleteGoal(AIGoal* pGoal);
    void ClearAllGoals(int iTeam);

    AIGoal* FindGoal(Character* pChr);

    void ReassignIfNeeded(Character* pChr);

    void RemoveUnit(Character* pChr);
    CommandPost* GetSpawnLocation(Character* pChr);

private:
    AIGoalManager();

public:

    void MakeSureUnitHasGoal(Character* pChr);

    void AssignUnit(Character* pChr);

    float mNextUpdateTime;
    AIGoal* mGoals[MAX_GOALS];

    static AIGoalManager* mInstance;
};

}

 
 
 
