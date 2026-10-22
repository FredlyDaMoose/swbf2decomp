#include "UnitWaitSecondaryAgent.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "Controllable.h"
#include "Entity.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

namespace ReserveManager
{
int GetReservationCountForObject(const GameObject* pObj, int iTeam, ReserveType eType);
}

namespace AIUtil { GameObject* FindFirstAIInVehicle(GameObject* pVehicle); }

namespace AI
{
enum VOType
{
    VOT_VehicleWaitingImpatiently = 39
};
namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);
}
}

// GLOBAL: SWBF2 0x01f9a25c
// SYNTHETIC: SWBF2 0x00406d60
// `dynamic initializer for 'UnitWaitSecondaryAgent::rttiHashUnitWaitSecondaryAgent''
PblHash UnitWaitSecondaryAgent::rttiHashUnitWaitSecondaryAgent("UnitWaitSecondaryAgent");
char UnitWaitSecondaryAgent::rttiNameUnitWaitSecondaryAgent[] = "UnitWaitSecondaryAgent";

// FUNCTION: SWBF2 0x0066b670
bool UnitWaitSecondaryAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitWaitSecondaryAgent || UnitAgent::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0066b6a0
unsigned int UnitWaitSecondaryAgent::GetDerivedRtti() const
{
    return rttiHashUnitWaitSecondaryAgent;
}

// FUNCTION: SWBF2 0x0066b6b0
const char* UnitWaitSecondaryAgent::GetDerivedRttiName() const
{
    return rttiNameUnitWaitSecondaryAgent;
}

// FUNCTION: SWBF2 0x0066b6c0
bool UnitWaitSecondaryAgent::ShouldQueueCurCmd(const UnitCmdInfo& cmd) const
{
    return false;
}

// SYNTHETIC: SWBF2 0x0066b6d0
// UnitWaitSecondaryAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0066b700
UnitWaitSecondaryAgent::UnitWaitSecondaryAgent(UnitController* pCtrl, float fWaitTime, bool bArg)
    : UnitAgent(pCtrl)
{
    mCurState = 1;
    mWaitUntilMissionTime = GameLoop::GetMissionTime() + fWaitTime;
    mPlayerWaitTimeout = GameLoop::GetMissionTime() + 15.0f;
    mImpatientSoundTime = GameLoop::GetMissionTime() + AIUtil::GetRandomFloat(7.0f, 10.0f);
    mNextState = 2;
}

// FUNCTION: SWBF2 0x0066b780
void UnitWaitSecondaryAgent::EnterState()
{
    if (mCurState == 2)
    {
        ((AILowLevel*)mCtrl->mLowLevel)->SetNavigator_Stationary();
    }
}

// FUNCTION: SWBF2 0x0066b7a0
void UnitWaitSecondaryAgent::ExitState()
{
}

// FUNCTION: SWBF2 0x0066b7b0
GameObject* UnitWaitSecondaryAgent::IsPlayerLookingAtVehicle()
{
    const PblMatrix& mMe = mCtrl->mOwner->GetMatrix();
    const float* pMyPos = &mMe.m[3][0];
    int iTeam = mCtrl->mOwner->mCharacter->mTeamNumber;

    for (int i = 0; i < Character::sCharacterLimit; ++i)
    {
        Character* pCharacter = Character::GetPtr(i);
        if (!pCharacter || pCharacter->mPlayerId < 0 || !pCharacter->GetControllable())
        {
            continue;
        }
        if (pCharacter->mTeamNumber != iTeam || pCharacter->mVehicle || pCharacter->mRemote)
        {
            continue;
        }

        const PblMatrix& mHim = pCharacter->mUnit->GetMatrix();
        PblVector3 vDelta;
        vDelta.x = pMyPos[0] - mHim.m[3][0];
        vDelta.y = pMyPos[1] - mHim.m[3][1];
        vDelta.z = pMyPos[2] - mHim.m[3][2];
        float fDistSq = vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z;
        if (fDistSq > 150.0f * 150.0f)
        {
            continue;
        }
        if (fDistSq < 12.0f * 12.0f)
        {
            return pCharacter->mUnit->GetGameObject();
        }

        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vDelta);
        if (mHim.m[2][0] * vDir.x + mHim.m[2][1] * vDir.y + mHim.m[2][2] * vDir.z > 0.866f)
        {
            return pCharacter->mUnit->GetGameObject();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0066ba80
int UnitWaitSecondaryAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
    case AI::EVT_EndInvisibility:
        return AI::EVT_NotHandled;

    case AI::EVT_Grenade:
        if (mCtrl->mOwner->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            TakeoffNow();
            return AI::EVT_NotHandled;
        }
        break;

    case AI::EVT_Damaged:
        if (mCtrl->mOwner->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            TakeoffNow();
            PlayDamageEventVO(pInfo);
            return AI::EVT_NotHandled;
        }
        break;

    case AI::EVT_Vision_CanSeeEnemy:
        if (mCtrl->mOwner->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            return AI::EVT_NotHandled;
        }
        break;

    case AI::EVT_Audible_CanHearEnemy:
        if (mCtrl->mOwner->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            TakeoffNow();
            return AI::EVT_Handled;
        }
        break;
    }
    return UnitAgent::EventHandler(pInfo);
}

// FUNCTION: SWBF2 0x0066bbb0
const char* UnitWaitSecondaryAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}

// FUNCTION: SWBF2 0x0066b960
void UnitWaitSecondaryAgent::UpdateStatePostVision(float dt)
{
    reinterpret_cast<AttackHelper*>(mAttackHelper)->Update();
    if (mCurState != 2)
    {
        return;
    }

    Character* pCharacter = mCtrl->mOwner->mCharacter;
    if (!pCharacter)
    {
        SetNextState(100);
        return;
    }
    Controllable* pVehicle = pCharacter->mVehicle;
    if (!pVehicle)
    {
        SetNextState(100);
        return;
    }

    GameObject* pVehicleObj = pVehicle->GetGameObject();
    GameObject* pPlayer = IsPlayerLookingAtVehicle();
    if (pPlayer)
    {
        if (mImpatientSoundTime > 0.0f && GameLoop::GetMissionTime() > mImpatientSoundTime)
        {
            mImpatientSoundTime = 0.0f;
            AI::VOHelper::Play(AI::VOT_VehicleWaitingImpatiently, AIUtil::FindFirstAIInVehicle(pVehicleObj), pPlayer, pPlayer, 0.0f);
        }
        if (mPlayerWaitTimeout > GameLoop::GetMissionTime())
        {
            return;
        }
    }

    if (AIUtil::CountUnitsInVehicle(pVehicle) == pVehicleObj->GetControllableList().GetCount() ||
        (GameLoop::GetMissionTime() > mWaitUntilMissionTime &&
         !ReserveManager::GetReservationCountForObject(pVehicleObj, mCtrl->mOwner->mCharacter->mTeamNumber, RT_BOARD)))
    {
        SetNextState(100);
    }
}
