#include "UnitBoardAgent.h"
#include "AILowLevel.h"
#include "AttackHelper.h"
#include "CollisionObject.h"
#include "ConnectivityGraph.h"
#include "Controllable.h"
#include "EntityCarrier.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "UnitController.h"
#include "VOHelper.h"
#include "PebbleFL/Common/PblMatrix.h"

class Obstacle;

namespace ReserveManager
{
bool IsReservedBy(const GameObject* pObj, const GameObject* pReserver, ReserveType eType);
int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType);
void Reserve(const GameObject* pObj, const GameObject* pReserver, ReserveType eType);
void UnreserveReserver(const GameObject* pReserver, ReserveType eType);
}

namespace AIUtil
{
Obstacle* GetGameObjectBarrier(GameObject* pObj);
}

namespace AI
{
enum VOType
{
    VOT_JustBoardedVehicle = 38
};
namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);
}
}

namespace AIUtil
{
GameObject* GetAlivePtr(PblHandle<GameObject> const& hObj, bool bIncludeBuildings);
}

namespace
{

unsigned int GetHandleId(const GameObject* pObj)
{
    return pObj->mHandleId;
}

GameObject* ResolveHandle(PblHandle<GameObject>& handle)
{
    GameObject* pObj = handle.mObject;
    if (pObj)
    {
        if (GetHandleId(pObj) == handle.mSavedHandleId)
        {
            return pObj;
        }
        handle.mObject = 0;
        handle.mSavedHandleId = 0;
    }
    return 0;
}

const CollisionObject* GetCollisionObject(const GameObject* pObj)
{
    return static_cast<const CollisionObject*>(pObj);
}

const PblVector3& GetTrans(const PblMatrix& m)
{
    return *reinterpret_cast<const PblVector3*>(&m.m[3][0]);
}

float DistSq(const PblVector3& a, const PblVector3& b)
{
    float fX = a.x - b.x;
    float fY = a.y - b.y;
    float fZ = a.z - b.z;
    return fX * fX + fY * fY + fZ * fZ;
}
}

// GLOBAL: SWBF2 0x01f99354
// SYNTHETIC: SWBF2 0x00406c20
// `dynamic initializer for 'UnitBoardAgent::rttiHashUnitBoardAgent''
PblHash UnitBoardAgent::rttiHashUnitBoardAgent("UnitBoardAgent");
char UnitBoardAgent::rttiNameUnitBoardAgent[] = "UnitBoardAgent";

// FUNCTION: SWBF2 0x00661f80
bool UnitBoardAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitBoardAgent || UnitAgent::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00661fb0
unsigned int UnitBoardAgent::GetDerivedRtti() const
{
    return rttiHashUnitBoardAgent;
}

// FUNCTION: SWBF2 0x00661fc0
const char* UnitBoardAgent::GetDerivedRttiName() const
{
    return rttiNameUnitBoardAgent;
}

// FUNCTION: SWBF2 0x00661fd0
bool UnitBoardAgent::ShouldQueueCurCmd(const UnitCmdInfo& cmd) const
{
    return false;
}

// SYNTHETIC: SWBF2 0x00661fe0
// UnitBoardAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00662010
UnitBoardAgent::UnitBoardAgent(UnitController* pCtrl, GameObject* pVehicle, int iVOResponseHandle)
    : UnitAgent(pCtrl)
{
    mVehicle.mObject = 0;
    mVehicle.mSavedHandleId = 0;
    mVehicle.mObject = pVehicle;
    mVehicle.mSavedHandleId =
        pVehicle ? pVehicle->mHandleId : 0;
    mVOResponseHandle = iVOResponseHandle;
    mCurState = 1;
    mNextState = 2;
}

// FUNCTION: SWBF2 0x006624d0
const char* UnitBoardAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}

// FUNCTION: SWBF2 0x006624e0
int UnitBoardAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
        if (ResolveHandle(pInfo->mGameObject) == ResolveHandle(mVehicle) || !GetMe() || !ResolveHandle(mVehicle))
        {
            return 0;
        }
        if (DistSq(GetCollisionObject(ResolveHandle(mVehicle))->GetPosition(), GetTrans(GetMe()->GetMatrix())) <=
            DistSq(GetCollisionObject(ResolveHandle(pInfo->mGameObject))->GetPosition(), GetTrans(GetMe()->GetMatrix())))
        {
            return 0;
        }
        if (UnitAgent::EventHandler(pInfo) == 1)
        {
            return 1;
        }
        return 0;

    case AI::EVT_Grenade:
    case AI::EVT_Damaged:
    case AI::EVT_Vision_CanSeeEnemy:
    case AI::EVT_Audible_CanHearEnemy:
    {
        GameObject* pVehicle = AIUtil::GetAlivePtr(mVehicle, false);
        if (pVehicle && RunForVehicle(pVehicle))
        {
            return 0;
        }
        break;
    }
    }
    return UnitAgent::EventHandler(pInfo);
}

// FUNCTION: SWBF2 0x00662070
void UnitBoardAgent::EnterState()
{
    mStateEnterTime = GameLoop::GetMissionTime();
    switch (mCurState)
    {
    case 2:
    {
        GameObject* pVehicle = AIUtil::GetAlivePtr(mVehicle, false);
        if (pVehicle)
        {
            if (!ReserveManager::IsReservedBy(pVehicle, GetMe()->GetGameObject(), RT_BOARD))
            {
                if (ReserveManager::GetOpenReservableSlots(pVehicle, GetMe()->GetGameObject()->mTeam, RT_BOARD) <= 0)
                {
                    SetNextState(100);
                    return;
                }
                ReserveManager::Reserve(pVehicle, GetMe()->GetGameObject(), RT_BOARD);
            }
            if (ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1) &&
                !pVehicle->IsRtti(EntityFlyer::rttiHashEntityFlyer) && !(pVehicle->GetMaxSpeed() < 0.1f))
            {
                GetLowLevel().SetNavigator_FollowTarget(pVehicle, PblVector3(0, 0, 0));
            }
            else
            {
                const PblMatrix& rMatrix = pVehicle->GetCollisionObject()->GetMatrix();
                GetLowLevel().SetNavigator_Goto(rMatrix.GetTranslation(), AIUtil::GetGameObjectBarrier(pVehicle),
                                                false, false);
            }
            return;
        }
        SetNextState(100);
        break;
    }
    case 3:
    {
        GameObject* pVehicle = AIUtil::GetAlivePtr(mVehicle, false);
        if (pVehicle)
        {
            ReserveManager::Reserve(pVehicle, GetMe()->GetGameObject(), RT_BOARD);
            GetLowLevel().SetNavigator_Stationary();
        }
        AI::VOHelper::CancelQueuedSound(mVOResponseHandle);
        if (pVehicle->GetControllableList().Begin()->GetPlayerId() >= 0)
        {
            AI::VOHelper::Play(AI::VOT_JustBoardedVehicle, GetMe()->GetGameObject(), 0, pVehicle,
                               GameLoop::GetMissionTime() + 0.5f);
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x00662260
void UnitBoardAgent::ExitState()
{
    if (mCurState == 2 || mCurState == 3)
    {
        ReserveManager::UnreserveReserver(GetMe()->GetGameObject(), RT_BOARD);
    }
}

// FUNCTION: SWBF2 0x00662290
void UnitBoardAgent::UpdateStatePostVision(float dt)
{
    if (mCurState == 100)
    {
        return;
    }
    if ((mCurState == 2 && GameLoop::GetMissionTime() > mStateEnterTime + 30.0f) ||
        (mCurState == 3 && GameLoop::GetMissionTime() > mStateEnterTime + 10.0f))
    {
        SetNextState(100);
        return;
    }

    reinterpret_cast<AttackHelper*>(mAttackHelper)->Update();
    GameObject* pVehicle = AIUtil::GetAlivePtr(mVehicle, false);
    if (pVehicle &&
        (!pVehicle->IsRtti(EntityFlyer::rttiHashEntityFlyer) ||
         reinterpret_cast<const EntityFlyer*>(pVehicle)->GetState() == EntityFlyer::LANDED) &&
        GetMe()->CanEnterVehicle(pVehicle) &&
        ReserveManager::IsReservedBy(pVehicle, GetMe()->GetGameObject(), RT_BOARD))
    {
        ReserveManager::Reserve(pVehicle, GetMe()->GetGameObject(), RT_BOARD);
        switch (mCurState)
        {
        case 2:
        {
            if (!GetLowLevel().IsNavigatorInProgress())
            {
                SetNextState(2);
                return;
            }
            const CollisionObject* pMe = GetMe()->GetGameObject()->GetCollisionObject();
            const CollisionObject* pVehicleCol = pVehicle->GetCollisionObject();
            float fRange = pMe->GetCollisionRadius() + pVehicleCol->GetCollisionRadius() + 1.5f;
            if (fRange * fRange > DistSq(pMe->GetCollisionCenter(), pVehicleCol->GetCollisionCenter()))
            {
                SetNextState(3);
            }
            GetLowLevel().SetHeading(pVehicle);
            break;
        }
        case 3:
            GetLowLevel().SetBoardVehicle(mVehicle);
            GetLowLevel().SetHeading(pVehicle);
            break;
        }
        return;
    }

    ReserveManager::UnreserveReserver(GetMe()->GetGameObject(), RT_BOARD);
    SetNextState(100);
}
