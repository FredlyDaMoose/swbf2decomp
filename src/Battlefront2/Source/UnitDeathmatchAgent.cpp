#include "UnitDeathmatchAgent.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "CombatUtil.h"
#include "ConnectivityGraph.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PathFinder.h"
#include "Team.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

#include <d3dx9math.h>
#include "PebbleFL/Common/PblHashTable.h"

// GLOBAL: SWBF2 0x01f9938c
// SYNTHETIC: SWBF2 0x00406c90
// `dynamic initializer for 'UnitDeathmatchAgent::rttiHashUnitDeathmatchAgent''
PblHash UnitDeathmatchAgent::rttiHashUnitDeathmatchAgent("UnitDeathmatchAgent");
char UnitDeathmatchAgent::rttiNameUnitDeathmatchAgent[] = "UnitDeathmatchAgent";

// FUNCTION: SWBF2 0x006653a0
bool UnitDeathmatchAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitDeathmatchAgent || uRtti == UnitAgent::rttiHashUnitAgent ||
           uRtti == Agent::rttiHashAgent;
}

// FUNCTION: SWBF2 0x006653d0
unsigned int UnitDeathmatchAgent::GetDerivedRtti() const
{
    return rttiHashUnitDeathmatchAgent;
}

// FUNCTION: SWBF2 0x006653e0
const char* UnitDeathmatchAgent::GetDerivedRttiName() const
{
    return rttiNameUnitDeathmatchAgent;
}

// FUNCTION: SWBF2 0x00665b50
const char* UnitDeathmatchAgent::NFINAL_GetStateString(int) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00665a50
int UnitDeathmatchAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_Grenade:
    case AI::EVT_Damaged:
    case AI::EVT_Vision_CanSeeEnemy:
    case AI::EVT_Audible_CanHearEnemy:
    case AI::EVT_EndInvisibility:
        if (mCurState != 4 && UnitAgent::EventHandler(pInfo) == 1 &&
            pInfo->mNextCmd.GetCmdType() == CMD_COMBAT)
        {
            mCombatCmd = pInfo->mNextCmd;
            SetNextState(4);
            pInfo->mNextCmd = UnitCmdInfo(CMD_NONE, PblHandle<GameObject>(),
                                          PblVector3(0, 0, 0), PblHandle<GameObject>());
            return 1;
        }
        return 0;

    default:
        return UnitAgent::EventHandler(pInfo);
    }
}

// SYNTHETIC: SWBF2 0x006653f0
// UnitDeathmatchAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00665450
UnitDeathmatchAgent::UnitDeathmatchAgent(UnitController* pCtrl)
    : UnitAgent(pCtrl),
      mCombatHelper(this),
      mNextJumpTime(0.0f)
{
    mCurState = STATE_START;
    mNextState = STATE_RunPoint;
}

// FUNCTION: SWBF2 0x006654a0
void UnitDeathmatchAgent::Kill()
{
    mCombatHelper.Kill();
    UnitAgent::Kill();
}

// FUNCTION: SWBF2 0x006654c0
void UnitDeathmatchAgent::EnterState()
{
    switch (mCurState)
    {
    case STATE_START:
        if (ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1))
        {
            SetNextState((PblRandom::GetGlobalInt() & 1) ? STATE_RunPoint : STATE_RunEnemy);
        }
        else
        {
            SetNextState(STATE_Stationary);
        }
        break;

    case STATE_RunPoint:
        mCtrl->GetLowLevel().SetNavigator_Goto(AIUtil::GetRandomAccessiblePoint(mCtrl), 0, false, false);
        break;

    case STATE_RunEnemy:
    {
        Character* pChar = mCtrl->mOwner->mCharacter;
        const Team* pTeam = Team::sTeams[pChar ? pChar->GetTeamNumber() : 3];
        int iEnemyTeam = pTeam->mAffiliation[1] < 0 ? 1 : 0;
        if (pTeam->mAffiliation[2] < 0)
        {
            iEnemyTeam = iEnemyTeam ? (int)(PblRandom::GetGlobalInt() & 1) + 1 : 2;
        }

        Character* pEnemy = Team::sTeams[iEnemyTeam]->GetRandomLiveCharacter();
        if (!pEnemy || !pEnemy->GetControllable())
        {
            break;
        }
        if (!CombatUtil::EngageInCombat(UnitControllerPtr(mCtrl->mOwner->mCtrl),
                                        pEnemy->GetControllable()->GetGameObject(), true))
        {
            SetNextState(STATE_RunPoint);
            break;
        }

        PblVector3 vDest = pEnemy->GetControllable()->GetGameObject()->GetPosition();
        if (GetMe()->GetGameObject()->GetGameObjectClass()->mAIType == ConnectivityGraph::CGT_FLAG_HUGE)
        {
            const PblVector3& vMyPos = GetMe()->GetMatrix().GetTranslation();
            PblVector3 vDelta(vDest.x - vMyPos.x, vDest.y - vMyPos.y, vDest.z - vMyPos.z);
            PblVector3 vDir;
            D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vDelta);
            float fRadius = GetMe()->GetGameObject()->GetCollisionObject()->GetCollisionRadius();
            vDest.x -= vDir.x * fRadius;
            vDest.y -= vDir.y * fRadius;
            vDest.z -= vDir.z * fRadius;
        }
        else
        {
            PblVector3 vPush = PblRandom::GetGlobalVector3();
            vPush.x = vPush.x * 10.0f + vDest.x;
            vPush.y = vPush.y * 10.0f + vDest.y;
            vPush.z = vPush.z * 10.0f + vDest.z;
            PathFinder::PushValidPoint(vDest, vPush, GetMe()->GetGameObject());
            vDest = vPush;
        }
        mCtrl->GetLowLevel().SetNavigator_Goto(vDest, 0, false, false);
        break;
    }

    case STATE_Combat:
        if (mCombatCmd.GetCmdType() == CMD_NONE)
        {
            mCombatCmd = UnitCombatInfo(UnitCombatInfo::ENGAGE);
        }
        mCombatHelper.Init(static_cast<const UnitCombatInfo&>(mCombatCmd));
        break;

    case STATE_Stationary:
        mCtrl->GetLowLevel().SetNavigator_Stationary();
        break;
    }
}

// FUNCTION: SWBF2 0x006657e0
void UnitDeathmatchAgent::ExitState()
{
    if (mCurState == STATE_Combat)
    {
        mCombatHelper.Clean();
    }
}

// FUNCTION: SWBF2 0x006657f0
void UnitDeathmatchAgent::DoJetJumpStuff()
{
    if (!GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }

    EntitySoldier* pSoldier = static_cast<EntitySoldier*>(GetMe()->GetEntity());
    PblVector3 vJumpDir(0.0f, 0.0f, 0.0f);
    if (pSoldier->mClass->mJetFuelRechargeRate <= 0.0f)
    {
        return;
    }

    if (pSoldier->GetJetFuel() > pSoldier->mClass->GetJetFuelMinBorder())
    {
        if (mCtrl->GetLowLevel().mJetJumping)
        {
            return;
        }
        if (GameLoop::GetMissionTime() <= mNextJumpTime)
        {
            return;
        }
        mNextJumpTime += 5.0f;
        if (!(PblRandom::GetGlobalInt() & 1))
        {
            return;
        }

        PblVector3 vStart = GetMe()->GetMatrix().GetTranslation();
        PblVector3 vEnd(vStart.x + vJumpDir.x, vStart.y + 40.0f, vStart.z + vJumpDir.z);
        const GameObject* pIgnore = GetMe()->GetGameObject();
        if (!CollisionManager::RayTest(vStart, vEnd, &pIgnore, 1, 0x9a))
        {
            mCtrl->GetLowLevel().SetJetJump(vJumpDir);
        }
    }
    else
    {
        mNextJumpTime = GameLoop::GetMissionTime() + 5.0f + PblRandom::GetGlobalFloat() * 10.0f;
    }
}

// FUNCTION: SWBF2 0x00665990
void UnitDeathmatchAgent::UpdateStatePostVision(float dt)
{
    if (mCurState != STATE_Combat)
    {
        reinterpret_cast<AttackHelper*>(mAttackHelper)->Update();
    }

    switch (mCurState)
    {
    case STATE_RunPoint:
    case STATE_RunEnemy:
        mCtrl->GetLowLevel().SetHeadingDefault();
        mCtrl->GetLowLevel().SetHeight(HEIGHT_STAND);
        DoJetJumpStuff();
        if (!mCtrl->GetLowLevel().IsNavigatorInProgress())
        {
            SetNextState(STATE_START);
        }
        break;

    case STATE_Combat:
        mCombatHelper.FSMUpdate(dt, false);
        if (mCombatHelper.IsDone())
        {
            SetNextState(STATE_START);
        }
        break;
    }
}
