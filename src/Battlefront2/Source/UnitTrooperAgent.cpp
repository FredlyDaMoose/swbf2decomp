#include "UnitTrooperAgent.h"

#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "CommandPost.h"
#include "Controllable.h"
#include "Entity.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PathFinder.h"
#include "SquadCommands.h"
#include "UnitController.h"
#include "VehicleManager.h"

// FUNCTION: SWBF2 0x0066b1a0
const char* UnitTrooperAgent::NFINAL_GetStateString(int) const
{
    return 0;
}

extern PblVector3 gVectorZero;

enum FormationType
{
    FT_RANDOM = 0,
    FT_SINGLEFILE = 1,
    FT_DOUBLEFILE = 2,
    FT_DOUBLEFILE_RAND = 3,
    FT_CIRCLE = 4,
    FT_CONVOY = 5,
    FT_FLYINGV = 6
};

namespace ReserveManager
{
    void UnreserveReserver(const GameObject* pReserver, ReserveType eType);
    int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType);
    PblVector3 ReserveOffset(const GameObject* pTarget, const GameObject* pSelf, FormationType eFormation);
}

namespace CombatUtil
{
    float GetWeaponOptimalRange(const Controllable* pMe);
}

class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

class EntityHover
{
public:
    static PblHash rttiHashEntityHover;
};

class EntityWalker
{
public:
    static PblHash rttiHashEntityWalker;
};

namespace VehicleManager { int RequestVehiclesInRange(const PblVector3& vPos, GameObject** ppOut, float fRadius, int iMax); }

namespace
{

    GameObject* ResolveHandle(PblHandle<GameObject>& handle)
    {
        GameObject* pObj = handle.mObject;
        if (pObj)
        {
            if (pObj->mHandleId == handle.mSavedHandleId)
            {
                return pObj;
            }
            handle.mObject = 0;
            handle.mSavedHandleId = 0;
        }
        return 0;
    }

    PblHandle<GameObject> MakeHandle(GameObject* pObj)
    {
        PblHandle<GameObject> handle;
        handle.mObject = pObj;
        handle.mSavedHandleId =
            pObj ? pObj->mHandleId : 0;
        return handle;
    }
}

// GLOBAL: SWBF2 0x01f9a250
// SYNTHETIC: SWBF2 0x00406d30
// `dynamic initializer for 'UnitTrooperAgent::rttiHashUnitTrooperAgent''
PblHash UnitTrooperAgent::rttiHashUnitTrooperAgent("UnitTrooperAgent");
char UnitTrooperAgent::rttiNameUnitTrooperAgent[] = "UnitTrooperAgent";

// FUNCTION: SWBF2 0x0066a3f0
bool UnitTrooperAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitTrooperAgent || UnitAgent::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0066a420
unsigned int UnitTrooperAgent::GetDerivedRtti() const
{
    return rttiHashUnitTrooperAgent;
}

// FUNCTION: SWBF2 0x0066a430
const char* UnitTrooperAgent::GetDerivedRttiName() const
{
    return rttiNameUnitTrooperAgent;
}

// SYNTHETIC: SWBF2 0x0066a440
// UnitTrooperAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0066a490
UnitTrooperAgent::UnitTrooperAgent(UnitController* pCtrl, const CommandPost* pPost)
    : UnitAgent(pCtrl)
    , mPost(pPost)
    , mSnipeHelper(this)
    , mPatrolHelper(this)
    , mRepairHelper(this)
    , mMineHelper(this)
{
    mConvoyParent.mObject = 0;
    mConvoyParent.mSavedHandleId = 0;
    mCurState = 1;
    mNextState = 1;
}

// FUNCTION: SWBF2 0x0066a500
void UnitTrooperAgent::Kill()
{
    mSnipeHelper.Kill();
    mPatrolHelper.Kill();
    mMineHelper.Kill();
    UnitAgent::Kill();
}

// FUNCTION: SWBF2 0x0066a540
float UnitTrooperAgent::GetUpdateRate() const
{
    return 2.0f;
}

// FUNCTION: SWBF2 0x0066a550
void UnitTrooperAgent::EnterState()
{
    if (!(const CommandPost*)mPost)
    {
        if (mCurState != 100)
        {
            SetNextState(100);
        }
        return;
    }

    switch (mCurState)
    {
    case STATE_START:
        if (AIUtil::IsNearPost(mCtrl->mOwner, mPost, gVectorZero))
        {
            SetNextState(STATE_Patrol);
        }
        else
        {
            SetNextState(STATE_Goto);
            SquadCommands::SquadCommandOrder order(
                SquadCommands::SC_FOLLOW, mCtrl->mOwner->mCharacter,
                MakeHandle(static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject()));
            SquadCommands::AIShoutOrder(order, 0.0f);
        }
        break;

    case STATE_Goto:
        if (mRepairHelper.CheckForRecipientAndInit())
        {
            SetNextState(STATE_Aid);
            break;
        }
        mSnipeHelper.ResetHintCheckTime();
        mSrcPos = *(const PblVector3*)&mCtrl->mOwner->GetMatrix().m[3][0];
        mCPPos = AIUtil::GetCommandPostPos(mPost);
        mCPOffset = AIUtil::GetCommandPostOffset(mPost);
        mCPTime = GameLoop::GetMissionTime();
        GetLowLevel().SetNavigator_Goto(
            PblVector3(mCPPos.x + mCPOffset.x, mCPPos.y + mCPOffset.y, mCPPos.z + mCPOffset.z), 0, false,
            false);
        break;

    case STATE_Patrol:
        if (mRepairHelper.CheckForRecipientAndInit())
        {
            SetNextState(STATE_Aid);
        }
        else
        {
            mPatrolHelper.HELPERInit(mPost);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0066a740
void UnitTrooperAgent::ExitState()
{
    switch (mCurState)
    {
    case STATE_Convoy:
        ReserveManager::UnreserveReserver(static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject(), RT_FORMATION);
        break;
    case STATE_Patrol:
        mPatrolHelper.HELPERClean();
        break;
    case STATE_MineLaying:
        mMineHelper.HELPERClean();
        break;
    case STATE_Snipe:
        mSnipeHelper.HELPERClean();
        break;
    }
}

// FUNCTION: SWBF2 0x0066a7a0
bool UnitTrooperAgent::CheckForVehicleConvoy()
{
    GameObject* pMe = static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject();
    if (!pMe->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return false;
    }

    const PblVector3& myPos = *(const PblVector3*)&mCtrl->mOwner->GetMatrix().m[3][0];
    PblVector3 myDest = GetLowLevel().GetGlobalDest();
    float dx = myPos.x - myDest.x;
    float dy = myPos.y - myDest.y;
    float dz = myPos.z - myDest.z;
    if (dx * dx + dy * dy + dz * dz < 150.0f * 150.0f)
    {
        return false;
    }

    GameObject* nearby[10];
    int iCount = VehicleManager::RequestVehiclesInRange(myPos, nearby, 50.0f, 10);
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObj = nearby[i];
        if (pMe == pObj)
        {
            continue;
        }
        if (!pObj->IsRtti(EntityHover::rttiHashEntityHover)
            && !pObj->IsRtti(EntityWalker::rttiHashEntityWalker))
        {
            continue;
        }
        if (pObj->GetMaxSpeed() > pMe->GetMaxSpeed())
        {
            continue;
        }
        if (!pObj->GetControllable())
        {
            continue;
        }
        UnitController* pCtrl = UnitControllerPtr(pObj->GetControllable()->mCtrl);
        if (!pCtrl)
        {
            continue;
        }

        PblVector3 vDest = reinterpret_cast<AILowLevel*>(pCtrl->mLowLevel)->GetGlobalDest();
        float ex = vDest.x - myDest.x;
        float ey = vDest.y - myDest.y;
        float ez = vDest.z - myDest.z;
        if (ex * ex + ey * ey + ez * ez > 50.0f * 50.0f)
        {
            continue;
        }
        if (ReserveManager::GetOpenReservableSlots(pObj, pMe->GetTeam(), RT_FORMATION) > 0)
        {
            mConvoyParent = MakeHandle(pObj);
            SetNextState(STATE_Convoy);
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0066a980
void UnitTrooperAgent::UpdateStatePostVision(float dt)
{
    if (!(const CommandPost*)mPost)
    {
        if (mCurState != 100)
        {
            SetNextState(100);
        }
        return;
    }

    AttackHelper& attack = *reinterpret_cast<AttackHelper*>(mAttackHelper);
    switch (mCurState)
    {
    case STATE_START:
    case STATE_Goto:
    case STATE_Convoy:
    case STATE_Patrol:
    case 100:
        attack.Update();
        break;
    case STATE_Snipe:
        attack.UpdateTarget();
        attack.Aim();
        break;
    }

    switch (mCurState)
    {
    case STATE_Goto:
    {
        if (CheckForVehicleConvoy())
        {
            break;
        }
        if (mSnipeHelper.LookForSnipeHintAndInit())
        {
            SetNextState(STATE_Snipe);
            break;
        }
        if (mRepairHelper.CheckForRecipientAndInit())
        {
            SetNextState(STATE_Aid);
            break;
        }
        if (GetLowLevel().IsAtDest() && GetLowLevel().IsNavigatorFailedPath())
        {
            SetNextState(100);
            break;
        }
        if (AIUtil::IsNearPost(mCtrl->mOwner, mPost, mCPOffset) || GetLowLevel().IsAtDest())
        {
            SetNextState(STATE_Patrol);
            break;
        }

        if (AIUtil::IsVehicle(static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject()))
        {
            const PblVector3& pos = *(const PblVector3*)&mCtrl->mOwner->GetMatrix().m[3][0];
            PblVector3 vDest = GetLowLevel().GetGlobalDest();
            float dx = pos.x - vDest.x;
            float dy = pos.y - vDest.y;
            float dz = pos.z - vDest.z;
            if (mCtrl->mOwner->GetCurWpn())
            {
                float fRange = CombatUtil::GetWeaponOptimalRange(mCtrl->mOwner);
                if (fRange * fRange > dx * dx + dy * dy + dz * dz)
                {
                    const PblVector3& from = *(const PblVector3*)&mCtrl->mOwner->GetMatrix().m[3][0];
                    if (!PathFinder::TestTrivialStraightLineHit(
                            from, GetLowLevel().GetGlobalDest(),
                            static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject()))
                    {
                        GetLowLevel().Navigator_Wait(5.0f);
                    }
                }
            }
        }

        if (AIUtil::HasCPMoved(mPost, mCtrl->mOwner, mCPPos, mCPTime, mSrcPos))
        {
            SetNextState(STATE_Goto);
        }
        else if (mMineHelper.CheckForHintsAndInit())
        {
            SetNextState(STATE_MineLaying);
        }
        else
        {
            FaceTarget(HEIGHT_STAND);
        }
        break;
    }

    case STATE_Convoy:
    {
        if (!ResolveHandle(mConvoyParent) || !ResolveHandle(mConvoyParent)->GetControllable()
            || !ResolveHandle(mConvoyParent)->GetControllable()->mCtrl)
        {
            SetNextState(STATE_Goto);
            break;
        }
        float fDist = (AIUtil::GetCollisionRadius(static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject())
                       + AIUtil::GetCollisionRadius(ResolveHandle(mConvoyParent)))
                      * 2.5f;
        PblVector3 vOffset = ReserveManager::ReserveOffset(
            ResolveHandle(mConvoyParent), static_cast<Trackable*>(mCtrl->mOwner)->GetGameObject(),
            FT_CONVOY);
        PblVector3 vFollow(vOffset.x * fDist, vOffset.y * fDist, vOffset.z * fDist * 0.3f);
        GetLowLevel().SetNavigator_FollowTarget(ResolveHandle(mConvoyParent), vFollow);
        break;
    }

    case STATE_Patrol:
    {
        if (mCtrl->mOwner->GetEntity()->IsRtti(EntityWalker::rttiHashEntityWalker)
            && mCtrl->mOwner->GetEntity()->GetEntityClass()->mId == 0x6fbcdecc)
        {
            GetLowLevel().SetNavigator_Stationary();
            break;
        }
        if (mRepairHelper.CheckForRecipientAndInit())
        {
            SetNextState(STATE_Aid);
            break;
        }
        mPatrolHelper.FSMUpdate(dt, false);
        if (mPatrolHelper.IsDone())
        {
            SetNextState(100);
            break;
        }
        ControllableHeight eHeight = mPatrolHelper.HELPERGetHeight();
        if (mPatrolHelper.HELPERGetOverrideDefaultFacePos())
        {
            FaceTarget(eHeight, mPatrolHelper.HELPERGetDefaultFacePos());
        }
        else
        {
            FaceTarget(eHeight);
        }
        break;
    }

    case STATE_MineLaying:
        mMineHelper.FSMUpdate(dt, false);
        if (mRepairHelper.CheckForRecipientAndInit())
        {
            SetNextState(STATE_Aid);
        }
        else if (mMineHelper.IsDone())
        {
            SetNextState(STATE_Goto);
        }
        else
        {
            GetLowLevel().SetHeadingDefault();
            GetLowLevel().SetHeight(HEIGHT_STAND);
        }
        break;

    case STATE_Aid:
        if (!mRepairHelper.Update(dt))
        {
            SetNextState(STATE_Goto);
        }
        break;

    case STATE_Snipe:
        mSnipeHelper.FSMUpdate(dt, false);
        if (mSnipeHelper.IsDone())
        {
            SetNextState(STATE_Goto);
        }
        else
        {
            attack.Fire();
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0066af80
int UnitTrooperAgent::EventHandler(AI::EventInfo* pInfo)
{
    bool bInterrupt;

    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
        switch (mCurState)
        {
        case STATE_Patrol:
            bInterrupt = mPatrolHelper.HELPERVehicleNearbyInterrupt(ResolveHandle(pInfo->mGameObject));
            break;
        case STATE_MineLaying:
            bInterrupt = mMineHelper.HELPERVehicleNearbyInterrupt(ResolveHandle(pInfo->mGameObject));
            break;
        case STATE_Snipe:
            bInterrupt = mSnipeHelper.HELPERVehicleNearbyInterrupt(ResolveHandle(pInfo->mGameObject));
            break;
        default:
            return UnitAgent::EventHandler(pInfo);
        }
        if (!bInterrupt)
        {
            return 0;
        }
        return UnitAgent::EventHandler(pInfo);

    case AI::EVT_Grenade:
        switch (mCurState)
        {
        case STATE_Patrol:
            bInterrupt = mPatrolHelper.HELPERGrenadeEventInterrupt();
            break;
        case STATE_MineLaying:
            bInterrupt = mMineHelper.HELPERGrenadeEventInterrupt();
            break;
        case STATE_Snipe:
            bInterrupt = mSnipeHelper.HELPERGrenadeEventInterrupt();
            break;
        case STATE_Goto:
            if (AreNearbyUnitsInCombat())
            {
                return 0;
            }
            return UnitAgent::EventHandler(pInfo);
        default:
            return UnitAgent::EventHandler(pInfo);
        }
        if (bInterrupt)
        {
            return UnitAgent::EventHandler(pInfo);
        }

    case AI::EVT_Damaged:
    {
        const AI::EventInfo_Damaged* pDamaged = static_cast<const AI::EventInfo_Damaged*>(pInfo);
        switch (mCurState)
        {
        case STATE_Patrol:
            bInterrupt = mPatrolHelper.HELPERDamagedInterrupt(pDamaged->mIsShot, pInfo->mPos);
            break;
        case STATE_MineLaying:
            bInterrupt = mMineHelper.HELPERDamagedInterrupt(pDamaged->mIsShot, pInfo->mPos);
            break;
        case STATE_Snipe:
            bInterrupt = mSnipeHelper.HELPERDamagedInterrupt(pDamaged->mIsShot, pInfo->mPos);
            break;
        case STATE_Goto:
            bInterrupt = !AreNearbyUnitsInCombat();
            break;
        default:
            return UnitAgent::EventHandler(pInfo);
        }
        if (bInterrupt)
        {
            return UnitAgent::EventHandler(pInfo);
        }
        DamagedEventPlayerCentricness(pInfo);
        PlayDamageEventVO(pInfo);
        return 0;
    }

    case AI::EVT_Vision_CanSeeEnemy:
        switch (mCurState)
        {
        case STATE_Patrol:
            bInterrupt = mPatrolHelper.HELPERSeeEnemyInterrupt(ResolveHandle(pInfo->mGameObject));
            break;
        case STATE_MineLaying:
            bInterrupt = mMineHelper.HELPERSeeEnemyInterrupt(ResolveHandle(pInfo->mGameObject));
            break;
        case STATE_Snipe:
            bInterrupt = mSnipeHelper.HELPERSeeEnemyInterrupt(ResolveHandle(pInfo->mGameObject));
            break;
        case STATE_Goto:
            if (AreNearbyUnitsInCombat())
            {
                return 0;
            }
            return UnitAgent::EventHandler(pInfo);
        default:
            return UnitAgent::EventHandler(pInfo);
        }
        if (bInterrupt)
        {
            return UnitAgent::EventHandler(pInfo);
        }

    case AI::EVT_Audible_CanHearEnemy:
        switch (mCurState)
        {
        case STATE_Patrol:
            bInterrupt = mPatrolHelper.HELPERHearEnemyInterrupt(ResolveHandle(pInfo->mGameObject));
            break;
        case STATE_MineLaying:
            bInterrupt = mMineHelper.HELPERHearEnemyInterrupt(ResolveHandle(pInfo->mGameObject));
            break;
        case STATE_Snipe:
            bInterrupt = mSnipeHelper.HELPERHearEnemyInterrupt(ResolveHandle(pInfo->mGameObject));
            break;
        case STATE_Goto:
            if (AreNearbyUnitsInCombat())
            {
                return 0;
            }
            return UnitAgent::EventHandler(pInfo);
        default:
            return UnitAgent::EventHandler(pInfo);
        }
        if (!bInterrupt)
        {
            return 0;
        }
        return UnitAgent::EventHandler(pInfo);

    default:
        return UnitAgent::EventHandler(pInfo);
    }
}

// FUNCTION: SWBF2 0x0066e390
int VehicleManager::RequestVehiclesInRange(const PblVector3& vPos, GameObject** ppOut, float fRadius, int iMax)
{
    const float fRadiusSq = fRadius * fRadius;
    int iCount = 0;
    for (int i = 0; i < VehicleManager::sVehicles->GetLength(); ++i)
    {
        GameObject* pObj = ResolveHandle((*VehicleManager::sVehicles)[i]);
        const PblMatrix& m = static_cast<const CollisionObject*>(pObj)->GetMatrix();
        const float dx = m.m[3][0] - vPos.x;
        const float dy = m.m[3][1] - vPos.y;
        const float dz = m.m[3][2] - vPos.z;
        if (dx * dx + dy * dy + dz * dz < fRadiusSq)
        {
            ppOut[iCount++] = pObj;
            if (iCount >= iMax)
            {
                break;
            }
        }
    }
    return iCount;
}
