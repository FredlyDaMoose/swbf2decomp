#include "UnitFlyAgent.h"

#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "Damageable.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "HintManager.h"
#include "UnitController.h"
#include "VehicleSpawn.h"
#include "Character.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "EntityCarrier.h"
#include "PathManager.h"
#include "WeaponCannonInstance.h"
#include "WeaponTowCable.h"
#include "PebbleFL/Common/PblRandom.h"

#include "UnitThreatManager.h"

// GLOBAL: SWBF2 0x01f993bc
// SYNTHETIC: SWBF2 0x00406cc0
// `dynamic initializer for 'UnitFlyAgent::rttiHashUnitFlyAgent''
PblHash UnitFlyAgent::rttiHashUnitFlyAgent("UnitFlyAgent");
char UnitFlyAgent::rttiNameUnitFlyAgent[] = "UnitFlyAgent";

namespace VehicleManager { int RequestVehiclesInRange(const PblVector3& vPos, GameObject** ppOut, float fRadius, int iMax); }

// FUNCTION: SWBF2 0x00666790
UnitFlyAgent::UnitFlyAgent(UnitController* pCtrl)
    : UnitAgent(pCtrl)
{
    mHint = 0;
    mNextHintCheckTime = -100.0f;
    mCurState = 1;
    mNextState = 1;
}

// SYNTHETIC: SWBF2 0x00666740
// UnitFlyAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006666f0
bool UnitFlyAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitFlyAgent || UnitAgent::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00666720
unsigned int UnitFlyAgent::GetDerivedRtti() const
{
    return rttiHashUnitFlyAgent;
}

// FUNCTION: SWBF2 0x00666730
const char* UnitFlyAgent::GetDerivedRttiName() const
{
    return rttiNameUnitFlyAgent;
}

// FUNCTION: SWBF2 0x006667d0
void UnitFlyAgent::EnterState()
{
    const PblVector3 vPos = mCtrl->mOwner->GetMatrix().GetTranslation();

    switch (mCurState)
    {
    case 1:
        mGotoNonOwnedCP = true;
        if (GetFlyer()->GetState() != EntityFlyer::LANDED)
        {
            SetNextState(4);
        }
        else
        {
            GetLowLevel().SetNavigator_Stationary();
        }
        break;

    case 2:
    {
        const PblVector3& vFwd = mCtrl->mOwner->GetMatrix().GetZAxis();
        PblVector3 vDest(vPos.x + vFwd.x * 200.0f, vPos.y + 0.0f, vPos.z + vFwd.z * 200.0f);
        vDest.y += 1.0f;

        AIPath* pPath = new AIPath();
        if (pPath)
        {
            pPath->Prepend(vDest, 0);
            pPath->Prepend(mCtrl->mOwner->GetMatrix().GetTranslation(), 0);
            GetLowLevel().SetNavigator_FollowPath(pPath, false, true, true);
            pPath->Release();
        }

        EntityFlyer* pFlyer = static_cast<EntityFlyer*>(mCtrl->mOwner->GetEntity());
        if (pFlyer->GetState() != EntityFlyer::FLYING)
        {
            pFlyer->TakeOff();
        }
        break;
    }

    case 3:
        if (IsTransport() && mGotoNonOwnedCP)
        {
            mWaitTime = GameLoop::GetMissionTime() + 10.0f;
            GetLowLevel().SetNavigator_Stationary();
        }
        else
        {
            mWaitTime = 4.0f;
            GetLowLevel().SetNavigator_Stationary();
        }
        break;

    case 4:
    {
        const int iTeam = static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject()->GetTeam();
        int iTries = 0;
        bool bGood;
        PblVector3 vDest;
        do
        {
            CommandPostManager* pManager = CommandPostManager::Instance();
            const CommandPost* pPost =
                pManager->FindPostFromIndex(AIUtil::GetRandomInt(0, pManager->GetCount() - 1));
            if (IsTransport())
            {
                if (mGotoNonOwnedCP)
                {
                    bGood = !pPost->CanTeamSpawnHere(iTeam);
                }
                else
                {
                    bGood = pPost->CanTeamSpawnHere(iTeam);
                }
            }
            else
            {
                bGood = true;
            }
            vDest = AIUtil::GetCommandPostPos(pPost);
            ++iTries;
        } while (!bGood &&
                 (vDest.x - vPos.x) * (vDest.x - vPos.x) + (vDest.y - vPos.y) * (vDest.y - vPos.y) +
                         (vDest.z - vPos.z) * (vDest.z - vPos.z) < 10000.0f &&
                 iTries < 10);

        vDest.y += 20.0f;
        GetLowLevel().SetNavigator_Goto(vDest, 0, false, false);
        if (GetFlyer()->GetState() != EntityFlyer::FLYING)
        {
            GetFlyer()->TakeOff();
        }
        break;
    }

    case 5:
    {
        mHint->Reserve(static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject());
        PblMatrix mLand = mHint->BuildMatrix();
        VehicleSpawn::gCalcBeginLandPos(*GetFlyer()->mClass, &mLand);
        const PblVector3 vLand = mLand.GetTranslation();

        const float fDist = GetFlyer()->GetMaxSpeed() * 2.0f;
        PblVector3 vBack = AIUtil::GetSafeNormalXZ(mHint->GetZAxis());
        vBack.y = -0.2f;
        const PblVector3 vApproach(vLand.x - vBack.x * fDist, vLand.y - vBack.y * fDist,
                                   vLand.z - vBack.z * fDist);

        AIPath* pPath = new AIPath();
        if (pPath)
        {
            pPath->Prepend(vLand, 0);
            pPath->Prepend(vApproach, 0);
            pPath->Prepend(mCtrl->mOwner->GetMatrix().GetTranslation(), 0);
            GetLowLevel().SetNavigator_FollowPath(pPath, false, true, true);
            pPath->Release();
        }
        GetLowLevel().mLandAtDest = true;
        break;
    }

    case 6:
        if (mGotoNonOwnedCP)
        {
            AIUtil::DeployCargo(static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject(), 0);
            if (!IsTransport())
            {
                mHint->SetReserveDelay(static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject(), 20.0f);
                mCtrl->mOwner->SetExitVehicle(mHint->GetCommandPost());
                mCtrl->mOwner->mCharacter->SetVehicleBlockTime(GameLoop::GetMissionTime() + 15.0f);
                return;
            }
        }
        mGotoNonOwnedCP = !mGotoNonOwnedCP;
        SetNextState(3);
        break;
    }
}

// FUNCTION: SWBF2 0x00666cf0
void UnitFlyAgent::ExitState()
{
}

// FUNCTION: SWBF2 0x00667520
const char* UnitFlyAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00667460
const EntityFlyer* UnitFlyAgent::GetFlyer() const
{
    if (!mCtrl->mOwner)
    {
        return 0;
    }
    return static_cast<const EntityFlyer*>(mCtrl->mOwner);
}

// FUNCTION: SWBF2 0x00667480
EntityFlyer* UnitFlyAgent::GetFlyer()
{
    if (!mCtrl->mOwner)
    {
        return 0;
    }
    return static_cast<EntityFlyer*>(mCtrl->mOwner);
}

// FUNCTION: SWBF2 0x00666e80
bool UnitFlyAgent::IsTransport() const
{
    const GameObject* pResult = static_cast<const Trackable*>(mCtrl->mOwner)->GetGameObject();
    return pResult->GetGameObjectClass()->GetVehicleType() == 5;
}

// FUNCTION: SWBF2 0x00667440
float UnitFlyAgent::GetUpdateRate() const
{
    if (GetFlyer()->mbCirclingPoint)
    {
        return 0.5f;
    }
    return UnitAgent::GetUpdateRate();
}

// FUNCTION: SWBF2 0x00666ea0
bool UnitFlyAgent::IsNearLandingPt()
{
    if (GameLoop::GetMissionTime() <= mNextHintCheckTime)
    {
        return false;
    }

    mHint = 0;
    mNextHintCheckTime = GameLoop::GetMissionTime() + 3.0f;

    const PblMatrix& mat = mCtrl->mOwner->GetMatrix();
    const float* pPos = reinterpret_cast<const float*>(&mat) + 12;

    const BaseHint* candidates[1];
    const int iCount = HintManager::RequestLandHints(mCtrl, *reinterpret_cast<const PblVector3*>(pPos), 80, 1,
                                                     candidates, mGotoNonOwnedCP);
    if (iCount <= 0)
    {
        return false;
    }

    mHint = const_cast<BaseHint*>(candidates[AIUtil::GetRandomInt(0, iCount - 1)]);
    mHint->Reserve(static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject());
    return true;
}

// FUNCTION: SWBF2 0x00666d00
void UnitFlyAgent::UpdateStatePostVision(float dt)
{
    if (GetLowLevel().IsStuck() == 2)
    {
        GameObject* pObject = static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject();
        static_cast<Damageable*>(pObject)
            ->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
    }

    reinterpret_cast<AttackHelper*>(mAttackHelper)->Update();

    switch (mCurState)
    {
    case 1:
        SetNextState(3);
        break;
    case 2:
        if (!IsRoofOverhead())
        {
            SetNextState(4);
        }
        break;
    case 3:
        if (GameLoop::GetMissionTime() > mWaitTime)
        {
            if (IsRoofOverhead())
            {
                SetNextState(2);
            }
            else
            {
                SetNextState(4);
            }
        }
        break;
    case 4:
        if (IsNearLandingPt())
        {
            SetNextState(5);
        }
        else if (GetLowLevel().IsAtDest())
        {
            SetNextState(4);
        }
        else if (!IsNearATAT())
        {
            GetFlyer()->mbCirclingPoint = false;
        }
        break;
    case 5:
        mHint->Reserve(static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject());
        if (GetFlyer()->mState == 0)
        {
            SetNextState(6);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00666f30
bool UnitFlyAgent::IsNearATAT()
{
    if (AIUtil::CountUnitsInVehicle(mCtrl->mOwner) != 2)
    {
        return false;
    }

    EntityFlyer* pFlyer = GetFlyer();
    if (!pFlyer->mbCirclingPoint)
    {
        if (pFlyer->mAIEng.ai.mReactionThinkTime > 0.0f)
        {
            return false;
        }
        pFlyer->mAIEng.ai.mReactionThinkTime = 4.75f;
        if (PblRandom::GetGlobalFloat() > pFlyer->mClass->mCircleATATFrequency)
        {
            return false;
        }
    }

    const PblVector3 vPos = mCtrl->mOwner->GetMatrix().GetTranslation();
    GameObject* pNearby[20];
    const int iCount = VehicleManager::RequestVehiclesInRange(vPos, pNearby, 200.0f, 20);

    bool bFree = true;
    for (int i = 0; i < iCount; ++i)
    {
        Controllable* pATAT = pNearby[i]->GetControllable();
        if (!pATAT || !AIUtil::IsATAT(pATAT))
        {
            continue;
        }

        bFree = true;
        for (int j = 0; j < iCount; ++j)
        {
            GameObject* pObject = pNearby[j];
            if (!pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer))
            {
                continue;
            }
            EntityFlyer* pOther = static_cast<EntityFlyer*>(pObject->GetEntity());
            if (pOther != GetFlyer() && pOther->mTowCableWalker &&
                pOther->GetTowCableWalker()->GetGameObject() ==
                    static_cast<Trackable*>(pATAT)->GetGameObject())
            {
                bFree = false;
            }
        }

        const PblVector3 vATAT = pATAT->GetMatrix().GetTranslation();
        if (pFlyer->mTowCableWalker)
        {
            const PblVector3 vDelta(pFlyer->mPointToCircle.x - vATAT.x, pFlyer->mPointToCircle.y - vATAT.y,
                                    pFlyer->mPointToCircle.z - vATAT.z);
            if (vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z > 2500.0f)
            {
                bFree = false;
            }
        }

        if (bFree)
        {
            const PblVector3& vFwd = pATAT->GetMatrix().GetZAxis();
            const bool bWasCircling = pFlyer->mbCirclingPoint;
            pFlyer->mbCirclingPoint = true;
            pFlyer->mPointToCircle = PblVector3(vATAT.x + vFwd.x * 0.0f, vATAT.y + vFwd.y * 0.0f,
                                                vATAT.z + vFwd.z * 0.0f);
            if (!bWasCircling)
            {
                pFlyer->EntityPathFollower::Reset(pFlyer->mClass);
                pFlyer->CirclePoint(40.0f);
                return true;
            }

            for (PblList<Controllable>::Itor it = pFlyer->GetControllableList(); it; ++it)
            {
                for (int k = 0; k < it->GetWeaponCount(); ++k)
                {
                    if (!it->GetWeaponPtr(k)->IsRtti(WeaponTowCable::rttiHashWeaponTowCable))
                    {
                        continue;
                    }
                    UnitController* pGunner = UnitControllerPtr(it->mCtrl);
                    if (pGunner)
                    {
                        reinterpret_cast<AI::UnitThreatManager*>(pGunner->mThreatManager)->AddThreat(static_cast<Trackable*>(pATAT)->GetGameObject());
                        pGunner->ForceSetAttackHelperTarget(static_cast<Trackable*>(pATAT)->GetGameObject());
                    }
                }
            }
            return true;
        }
    }

    if (!GetFlyer()->mbCirclingPoint || !GetFlyer()->GetTowCableWalker() || !bFree)
    {
        GetFlyer()->mbCirclingPoint = false;
    }
    return false;
}

// FUNCTION: SWBF2 0x006673b0
bool UnitFlyAgent::IsRoofOverhead() const
{
    const PblVector3 vStart = *(const PblVector3*)&mCtrl->mOwner->GetMatrix().m[3][0];
    const PblVector3 vEnd(vStart.x + 0.0f, vStart.y + 100.0f, vStart.z + 0.0f);
    const GameObject* pIgnore = static_cast<const Trackable*>(mCtrl->mOwner)->GetGameObject();
    return CollisionManager::RayTest(vStart, vEnd, &pIgnore, 1, 0x9a);
}

// FUNCTION: SWBF2 0x006674a0
int UnitFlyAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
    case AI::EVT_Grenade:
    case AI::EVT_Audible_CanHearEnemy:
        return 0;
    case AI::EVT_Damaged:
        PlayDamageEventVO(pInfo);
        return 0;
    case AI::EVT_Vision_CanSeeEnemy:
        if (mCurState == 5 || mCurState == 2 || IsTransport())
        {
            return 0;
        }
        return UnitAgent::EventHandler(pInfo);
    default:
        return UnitAgent::EventHandler(pInfo);
    }
}
