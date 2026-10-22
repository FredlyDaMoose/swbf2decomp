#include "AIGoalManager.h"

#include <string.h>

#include "GameLoop.h"
#include "Character.h"
#include "ConquestGoal.h"
#include "DestroyGoal.h"
#include "DefendGoal.h"
#include "DeathmatchGoal.h"
#include "CTFGoal.h"
#include "CTFGoalD.h"
#include "FollowGoal.h"

#include "ListPool.h"
#include "TreeGrid.h"
#include "PebbleFL/Common/PblHeap.h"
#include "NetGame.h"
#include "TeamManager.h"

namespace AI {

// FUNCTION: SWBF2 0x0043a1c0
void AIGoalManager::Create()
{
    mInstance = new AIGoalManager();
}

// FUNCTION: SWBF2 0x0043a360
void AIGoalManager::ReassignIfNeeded(Character* pChr)
{
    RemoveUnit(pChr);
    AssignUnit(pChr);
}

// FUNCTION: SWBF2 0x0043a380
void AIGoalManager::RemoveUnit(Character* pChr)
{
    PblList<Character>* pList = pChr->GetGoalNode()->GetList();
    if (pList)
    {
        pList->Remove(pChr->GetGoalNode());
    }
}

// FUNCTION: SWBF2 0x0043a3c0
AIGoal* AIGoalManager::FindGoal(Character* pChr)
{
    for (int i = 0; i < MAX_GOALS; i++)
    {
        if (mGoals[i])
        {
            for (PblList<Character>::Itor it(mGoals[i]->mUnitList); (Character*)it; ++it)
            {
                if ((Character*)it == pChr)
                {
                    return mGoals[i];
                }
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0043a410
void AIGoalManager::MakeSureUnitHasGoal(Character* pChr)
{
    if (pChr->mPlayerId >= 0)
    {
        return;
    }
    if (pChr->GetGoalNode()->GetList())
    {
        AIGoal* pGoal = FindGoal(pChr);
        if (pChr->mTeamNumber != pGoal->GetTeam())
        {
            pGoal->mUnitList.Remove(pChr->GetGoalNode());
        }
    }
    if (!pChr->GetGoalNode()->GetList())
    {
        AssignUnit(pChr);
    }
}

// FUNCTION: SWBF2 0x0043a480
CommandPost* AIGoalManager::GetSpawnLocation(Character* pChr)
{
    MakeSureUnitHasGoal(pChr);
    AIGoal* pGoal = FindGoal(pChr);
    if (pGoal)
    {
        return pGoal->GetSpawnLocation(pChr);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0043a4b0
AIGoalManager::AIGoalManager()
{
    ActivateThread(0, PER_LOCAL_TURN);
    memset(mGoals, 0, sizeof(mGoals));
}

// FUNCTION: SWBF2 0x0043a540
AIGoalManager::~AIGoalManager()
{
}

// FUNCTION: SWBF2 0x0043a550
AIGoal* AIGoalManager::AddGoal(unsigned char uTeam, AIGoalType eType, float fWeight,
                               const GoalTarget* pTarget)
{
    if (fWeight <= 0.0f)
    {
        return 0;
    }

    int i = 0;
    while (mGoals[i])
    {
        if (++i >= MAX_GOALS)
        {
            return 0;
        }
    }

    switch (eType)
    {
    case AIGOAL_CONQUEST:
        mGoals[i] = new ConquestGoal(uTeam, fWeight);
        break;
    case AIGOAL_DESTROY:
        if (pTarget && pTarget->mType != GTT_NONE)
        {
            mGoals[i] = new DestroyGoal(uTeam, fWeight, *pTarget);
        }
        break;
    case AIGOAL_DEFEND:
        if (pTarget && pTarget->mType != GTT_NONE)
        {
            mGoals[i] = new DefendGoal(uTeam, fWeight, *pTarget);
        }
        break;
    case AIGOAL_DEATHMATCH:
        mGoals[i] = new DeathmatchGoal(uTeam, fWeight);
        break;
    case AIGOAL_CTFOFFENSE:
        mGoals[i] = new CTFGoal(uTeam, fWeight, *pTarget);
        break;
    case AIGOAL_CTFDEFENSE:
        mGoals[i] = new CTFGoalD(uTeam, fWeight, *pTarget);
        break;
    case AIGOAL_FOLLOW:
        if (pTarget && (pTarget->mType == GTT_CHARACTER || pTarget->mType == GTT_GAMEOBJECT))
        {
            mGoals[i] = new FollowGoal(uTeam, fWeight, *pTarget);
        }
        break;
    }
    return mGoals[i];
}

// FUNCTION: SWBF2 0x0043a760
void AIGoalManager::DeleteGoal(AIGoal* pGoal)
{
    for (int i = 0; i < MAX_GOALS; i++)
    {
        if (mGoals[i] == pGoal)
        {
            delete mGoals[i];
            mGoals[i] = 0;
            return;
        }
    }
}

// FUNCTION: SWBF2 0x0043a7a0
void AIGoalManager::ClearAllGoals(int iTeam)
{
    for (int i = 0; i < MAX_GOALS; i++)
    {
        if (mGoals[i] && mGoals[i]->GetTeam() == iTeam)
        {
            mGoals[i]->UnAssignAll();
            delete mGoals[i];
            mGoals[i] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x0043a7f0
bool AIGoalManager::Update(float)
{
    if (mNextUpdateTime <= GameLoop::GetMissionTime())
    {
        mNextUpdateTime = GameLoop::GetMissionTime() + 1.0f;

        for (int i = 0; i < Character::sCharacterLimit; i++)
        {
            MakeSureUnitHasGoal(Character::GetPtr(i));
        }

        for (int i = 0; i < MAX_GOALS; i++)
        {
            if (mGoals[i])
            {
                mGoals[i]->Update();
            }
        }
    }
    return true;
}

}

// FUNCTION: SWBF2 0x0043a230
void AI::AIGoalManager::Destroy()
{
    delete mInstance;
    mInstance = 0;
}

// GLOBAL: SWBF2 0x01e2fdac
AI::AIGoalManager* AI::AIGoalManager::mInstance;
