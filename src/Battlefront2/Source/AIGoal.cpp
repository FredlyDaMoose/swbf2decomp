#include "AIGoal.h"
#include "AIGoalManager.h"
#include "AIUtil.h"
#include "Character.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "Controllable.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Team.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

// GLOBAL: SWBF2 0x01eb01f4
// SYNTHETIC: SWBF2 0x004021e0
// `dynamic initializer for 'AI::AIGoal::rttiHashAIGoal''
PblHash AI::AIGoal::rttiHashAIGoal("AIGoal");
char AI::AIGoal::rttiNameAIGoal[] = "AIGoal";

// GLOBAL: SWBF2 0x01eb0410
// SYNTHETIC: SWBF2 0x004021f0
// `dynamic initializer for 'AI::AIGoal::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766d40
// `dynamic atexit destructor for 'AI::AIGoal::sMemoryPool''
MemoryPool AI::AIGoal::sMemoryPool("AIGoal", 0x20);

// FUNCTION: SWBF2 0x00439ab0
bool AI::AIGoal::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashAIGoal;
}

// FUNCTION: SWBF2 0x00439ad0
unsigned int AI::AIGoal::GetDerivedRtti() const
{
    return rttiHashAIGoal;
}

// FUNCTION: SWBF2 0x00439ae0
const char* AI::AIGoal::GetDerivedRttiName() const
{
    return rttiNameAIGoal;
}

// FUNCTION: SWBF2 0x00439b50
bool AI::GoalTarget::GetLocation(PblVector3& vPos)
{
    switch (mType)
    {
    case GTT_GAMEOBJECT:
        if (!mGameObj)
        {
            return false;
        }
        vPos = mGameObj->GetTargetPoint(PblVector3(0.0f, 0.0f, 0.0f), PblVector3(0.0f, 1.0f, 0.0f), -1);
        return true;
    case GTT_CHARACTER:
    {
        const Controllable* pCtrl = mChr->GetControllable();
        if (!pCtrl)
        {
            return false;
        }
        vPos = *(const PblVector3*)&pCtrl->GetMatrix().m[3][0];
        return true;
    }
    case GTT_POS:
        vPos = mPos;
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00439c90
const GameObject* AI::GoalTarget::GetGameObject()
{
    if (mType == GTT_GAMEOBJECT)
    {
        return mGameObj;
    }
    if (mType == GTT_CHARACTER)
    {
        const Controllable* pCtrl = mChr->GetControllable();
        if (pCtrl)
        {
            return pCtrl->GetGameObject();
        }
    }
    return 0;
}

// SYNTHETIC: SWBF2 0x00439af0
// AI::AIGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00439d10
AI::AIGoal::AIGoal(AIGoalType eType, unsigned char uTeam, float fWeight)
    : mWeight(fWeight), mEnabled(true), mTeam(uTeam)
{
}

// FUNCTION: SWBF2 0x00439d50
AI::AIGoal::~AIGoal()
{
    mUnitList.RemoveAll();
}

// FUNCTION: SWBF2 0x00439d60
void AI::AIGoal::UnAssignAll()
{
    for (PblList<Character>::Itor itor(mUnitList); itor; ++itor)
    {
        Controllable* pCtrl = itor->GetControllable();
        UnitController* pController = pCtrl ? UnitControllerPtr(pCtrl->mCtrl) : 0;
        if (pController)
        {
            CmdInfo cmd = UnitCmdInfo(CMD_WAIT, PblHandle<GameObject>(), PblVector3(0.0f, 0.0f, 0.0f),
                                      PblHandle<GameObject>());
            pController->ForceCommand(cmd);
        }
    }
    mUnitList.RemoveAll();
}

// FUNCTION: SWBF2 0x00439e80
void AI::AIGoal::AddUnit(Character* pChr)
{
    PblList<Character>::Node* pNode = pChr->GetGoalNode();
    if (pNode->GetList() != &mUnitList)
    {
        if (pNode->GetList())
        {
            pNode->GetList()->Remove(pNode);
        }
        mUnitList.Add(pNode, pChr);
    }
}

// FUNCTION: SWBF2 0x00439ee0
void AI::AIGoal::Update()
{
    for (PblList<Character>::Itor itor(mUnitList); itor; ++itor)
    {
        Controllable* pCtrl = itor->GetControllable();
        if (!pCtrl)
        {
            continue;
        }
        UnitController* pController = UnitControllerPtr(pCtrl->mCtrl);
        if (!pController)
        {
            continue;
        }
        int iCmd = pController->GetCurCmdType();
        if (iCmd == CMD_NONE || iCmd == CMD_WAIT || pController->IsAgentDone())
        {
            Assign(itor);
        }
    }
}

// FUNCTION: SWBF2 0x00439f70
void AI::AIGoal::Assign(Character* pChr)
{
}

// FUNCTION: SWBF2 0x00439f80
void AI::AIGoal::SetEnabled(bool bEnabled)
{
    if (mEnabled == true && !bEnabled)
    {
        UnAssignAll();
    }
    mEnabled = bEnabled;
}

// FUNCTION: SWBF2 0x00439fb0
CommandPost* AI::AIGoal::GetSpawnLocation(const Character* pChr) const
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }

    float fTotalWeight = 0.0f;
    int iTeam = pChr->mTeamNumber;
    for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
    {
        CommandPost* pPost = *itor;
        if (pPost->CanTeamSpawnHere(iTeam))
        {
            fTotalWeight += pPost->mAISpawnWeight;
        }
    }
    if (fTotalWeight <= 0.0f)
    {
        return 0;
    }

    float fPick = AIUtil::GetRandomFloat(0.0f, fTotalWeight - 1.0f);
    for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
    {
        CommandPost* pPost = *itor;
        if (pPost->CanTeamSpawnHere(iTeam))
        {
            fPick -= pPost->mAISpawnWeight;
            if (fPick <= 0.0f)
            {
                return pPost;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0043a250
void AI::AIGoalManager::AssignUnit(Character* pChr)
{
    float fWeights[MAX_GOALS];
    int iNumGoals = 0;
    float fTotalWeight = 0.0f;
    for (int i = 0; i < MAX_GOALS; i++)
    {
        AIGoal* pGoal = mGoals[i];
        if (pGoal && pGoal->GetTeam() == pChr->mTeamNumber && pGoal->mEnabled)
        {
            ++iNumGoals;
            fWeights[i] = pGoal->mWeight;
            fTotalWeight += pGoal->mWeight;
        }
        else
        {
            fWeights[i] = 0.0f;
        }
    }

    if (iNumGoals == 0 || fTotalWeight == 0.0f)
    {
        return;
    }
    if (fTotalWeight <= 0.0f)
    {
        fTotalWeight = 1.0f;
    }

    int iBest = -1;
    float fBestRatio = 1e11f;
    int iTeam = pChr->mTeamNumber;
    int iNumMembers = Team::GetPtr(iTeam)->mMemberCount;
    for (int i = 0; i < MAX_GOALS; i++)
    {
        AIGoal* pGoal = mGoals[i];
        if (pGoal && pGoal->GetTeam() == iTeam && pGoal->mEnabled)
        {
            float fRatio = (float)pGoal->mUnitList.GetCount() /
                           (fWeights[i] / fTotalWeight * (float)iNumMembers);
            if (fRatio < fBestRatio)
            {
                fBestRatio = fRatio;
                iBest = i;
            }
        }
    }
    if (iBest >= 0)
    {
        mGoals[iBest]->AddUnit(pChr);
    }
}
