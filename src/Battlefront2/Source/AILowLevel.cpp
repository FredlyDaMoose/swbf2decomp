#include <float.h>
#include <math.h>
#include <stddef.h>

#include "AILowLevel.h"
#include "AIDifficulty.h"
#include "AIUtil.h"
#include "Character.h"
#include "Controllable.h"
#include "CollisionObject.h"
#include "ConnectivityGraph.h"
#include "Entity.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "GameLoop.h"
#include "Navigator.h"
#include "NavigatorFollowTarget.h"
#include "NavigatorGraphFollower.h"
#include "NavigatorPathFollower.h"
#include "NavigatorSlide.h"
#include "PathFinder.h"
#include "PathFollower.h"
#include "PathManager.h"
#include "SquadCommands.h"
#include "Team.h"
#include "Thread.h"
#include "UnitController.h"
#include "Weapon.h"
#include "WeaponCannon.h"
#include "WeaponCannonInstance.h"
#include "WeaponGrenade.h"
#include "WeaponLauncher.h"
#include "WeaponMeleeInstance.h"
#include "WeaponRepair.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"

namespace
{
    unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }
}

class AmmoCounter
{
public:
    int GetNumRounds() const;

    AmmoCounterClass* m_pClass;
    float mDefaultMaxClips;
    float mMaxClips;
    float mNumClips;
    float mCurClip;
};

inline float Weapon::GetHeat() const
{
    return 1.0f - m_pEnergyBar->GetEnergy() / m_pEnergyBar->GetEnergyMax();
}

class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

class EntityDroideka
{
public:
    static PblHash rttiHashEntityDroideka;
};

class WeaponDispenser
{
public:
    static PblHash rttiHashWeaponDispenser;
};

namespace CombatUtil
{
    bool IsSniperWpn(const Weapon* pWpn);
    bool TestWpnConstraintsObj(Controllable* pMe, const GameObject* pTarget);
}

namespace
{

    void AI_ForceDeflectState(WeaponMelee* pMelee, bool bDeflect)
    {
        pMelee->m_bAI_Deflect = bDeflect;
    }

    int GetTeam(const GameObject* pObject)
    {
        return pObject->GetTeam();
    }

    unsigned int GetHealthType(const GameObject* pObject)
    {
        return pObject->mHealthType;
    }

    const CollisionObject* GetCollisionObject(const GameObject* pObject)
    {
        return pObject->GetCollisionObject();
    }
}

void* Navigator::operator new(unsigned int uiSize)
{
    return sMemoryPool.Allocate(uiSize);
}

void* AIPath::operator new(unsigned int uiSize)
{
    return sMemoryPool.Allocate(uiSize);
}

// FUNCTION: SWBF2 0x0043ab20
AILowLevel::AILowLevel(UnitController* pOwner)
    : mDoIdle(SoldierAnimType::IDLE),
      mOwner(pOwner),
      mNavigator(0),
      mHeight(HEIGHT_STAND),
      mExitCP(0),
      mAIJetJumpDir(0.0f, 0.0f, 0.0f),
      mMinControlSpeed(0.0f),
      mFoldStatus(FOLD_FOLD)
{
    mRoll = false;
    mRolling = false;
    mDoJetJump = false;
    mDoJump = false;
    mJumping = false;
    mJetJumpStop = false;
    mInCombat = false;
    mAllowHoverStrafe = false;
    mLandAtDest = false;
    mImmediateFire = false;
    mFireUponAim = false;
    ClearControllable();
}

// FUNCTION: SWBF2 0x0043abe0
AILowLevel::~AILowLevel()
{
    DeleteNavigator();
}

// FUNCTION: SWBF2 0x0043abf0
void AILowLevel::ClearControllable()
{
    DeleteNavigator();
    mOwner->mOwner->mTargetInfo.mIsAiming = false;
    mLandAtDest = false;
    mFireUponAim = false;
    mInCombat = false;
    mAllowHoverStrafe = false;
    mRoll = false;
    mDoJetJump = false;
    mDoJump = false;
    mJetJumpStop = false;
    mImmediateFire = false;
    mRollDir = PblVector3(0.0f, 0.0f, 0.0f);
    mExitVehicle = false;
    mFlyerIgnoreBarriers = true;
    mAIJetJumpDir = PblVector3(0.0f, 0.0f, 0.0f);
    mFlyerIgnoreSafeHeight = false;
    mDoIdle = SoldierAnimType::IDLE;
    mFoldStatus = FOLD_FOLD;
    mMinControlSpeed = 0.0f;
    mTarget.mObject = 0;
    mTarget.mSavedHandleId = 0;
    mIsCharging = false;
    mIsAlert = false;
    mFastTurn = false;
    mNextSniperFireTime = 0.0f;
    mGrenadeOffTimer = -1.0f;
    mNextWeaponIndex = -1;
    mTriggerSingleWaitUntil = 0.0f;
    mDest.mHasDest = false;
    mHeight = HEIGHT_STAND;
    mDesiredHoverHeight = FLT_MAX;
    mHeadLookState = AI::HEAD_LOOK_OFF;
    mHeadLook_TurnToTime = 0.0f;
    mHeadLook_StayTime = 0.0f;
    mHeadLook_TurnBackTime = 0.0f;
    mHeading.mObj.mObject = 0;
    mHeading.mObj.mSavedHandleId = 0;
    SetHeadingDefault();
}

// FUNCTION: SWBF2 0x0043ad10
void AILowLevel::ResetControls()
{
    mRoll = false;
    mDoJetJump = false;
    mDoJump = false;
    mJetJumpStop = false;
    mInCombat = false;
    mAllowHoverStrafe = false;
    mImmediateFire = false;
    mFireUponAim = false;
    mRollDir = PblVector3(0.0f, 0.0f, 0.0f);
    mAIJetJumpDir = PblVector3(0.0f, 0.0f, 0.0f);
    mDoIdle = SoldierAnimType::IDLE;
    mMinControlSpeed = 0.0f;
    mTarget.mObject = 0;
    mTarget.mSavedHandleId = 0;
    mIsAlert = false;
    mFastTurn = false;
    mNextWeaponIndex = -1;
    mDesiredHoverHeight = FLT_MAX;
}

// FUNCTION: SWBF2 0x0043ada0
void AILowLevel::Initialize()
{
    DeleteNavigator();
    mHeading.mObj.mObject = 0;
    mHeading.mObj.mSavedHandleId = 0;
    SetHeadingDefault();
    mRolling = false;
    mJumping = false;
    mJetJumping = false;
    mJetJumpStop = false;
    mHeight = HEIGHT_STAND;
    mFoldStatus = FOLD_FOLD;
    mDest.mHasDest = false;
    mMinControlSpeed = 0.0f;
    mFlyerIgnoreBarriers = true;
    mFlyerIgnoreSafeHeight = false;
    mBoardVehicle = false;
    mExitVehicle = false;
    mLandAtDest = false;
    mExitCP = 0;
    mTriggerSingleWaitUntil = 0.0f;
    mHeadLookState = AI::HEAD_LOOK_OFF;
    mHeadLook_TurnToTime = 0.0f;
    mHeadLook_StayTime = 0.0f;
    mHeadLook_TurnBackTime = 0.0f;
    mDesiredHoverHeight = FLT_MAX;
}

// FUNCTION: SWBF2 0x0043ae30
void AILowLevel::SetDest(const PblVector3& pos, const PblVector3& originalPos,
                         const PblVector3& direction, SafeHeight safeHeight,
                         float fTaperFactor, float fMaxSpeed)
{
    mDest.mHasDest = true;
    mDest.mPos = pos;
    mDest.mOriginalPos = originalPos;
    mDest.mDirection = direction;
    mDest.mSafeHeightEnforce = safeHeight;
    mDest.mTaperFactor = fTaperFactor;
    mDest.mMaxSpeed = fMaxSpeed;
}

// FUNCTION: SWBF2 0x0043ae90
void AILowLevel::SetDestCurrent()
{
    SetDest(*(const PblVector3*)&GetMatrix().m[3][0],
            *(const PblVector3*)&GetMatrix().m[3][0],
            *(const PblVector3*)&GetMatrix().m[2][0],
            SH_FULL, -1.0f, 1.0f);
    mAllowHoverStrafe = true;
}

// FUNCTION: SWBF2 0x0043aee0
unsigned char AILowLevel::GetPathQueueDepth()
{
    if (mNavigator)
    {
        PathFollower* pFollower = mNavigator->GetPathFollower();
        if (pFollower)
        {
            return pFollower->GetPathQueueDepth();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0043af00
bool AILowLevel::HasPath() const
{
    return mNavigator && mNavigator->GetPathFollower();
}

// FUNCTION: SWBF2 0x0043af20
bool AILowLevel::IsAtDest() const
{
    if (!mNavigator)
    {
        return true;
    }
    return mNavigator->IsDone();
}

// FUNCTION: SWBF2 0x0043af40
int AILowLevel::IsStuck() const
{
    return mNavigator && mNavigator->IsStuck();
}

// FUNCTION: SWBF2 0x0043af60
void AILowLevel::SetHeight(ControllableHeight eHeight)
{
    mHeight = eHeight;
}

// FUNCTION: SWBF2 0x0043af70
const PblMatrix& AILowLevel::GetMatrix() const
{
    return mOwner->mOwner->GetMatrix();
}

// FUNCTION: SWBF2 0x0043af80
void AILowLevel::SetHeading(const GameObject* pObj)
{
    mHeading.mType = ControllableHeading::HEADING_OBJ;
    mHeading.mObj.mObject = pObj;
    mHeading.mObj.mSavedHandleId = pObj ? GameObjectHandleId(pObj) : 0;
}

// FUNCTION: SWBF2 0x0043afb0
void AILowLevel::SetHeading(const PblVector3& dest, float fWander)
{
    mHeading.mType = ControllableHeading::HEADING_POS;
    mHeading.mDest = dest;
    mHeading.mDestWander = fWander;
}

// FUNCTION: SWBF2 0x0043afe0
void AILowLevel::SetHeadingDefault()
{
    mHeading.mType = ControllableHeading::HEADING_DEFAULT;
}

// FUNCTION: SWBF2 0x0043aff0
void AILowLevel::SetBoardVehicle(GameObject* pVehicle)
{
    mVehicle.mObject = pVehicle;
    mVehicle.mSavedHandleId = pVehicle ? GameObjectHandleId(pVehicle) : 0;
    mBoardVehicle = true;
}

// FUNCTION: SWBF2 0x0043b030
void AILowLevel::SetExitVehicle(const CommandPost* pCP)
{
    mExitVehicle = true;
    mExitCP = pCP;
}

// FUNCTION: SWBF2 0x0043b050
void AILowLevel::ProcessSniperWeapon(float dt, ControllableHeading* pHeading, bool* pFire)
{
    if (*pFire)
    {
        float fTime = GameLoop::GetMissionTime();
        if (mNextSniperFireTime > fTime)
        {
            *pFire = false;
        }
        else
        {
            mNextSniperFireTime = fTime + AIUtil::GetRandomFloat(2.0f, 3.0f);
        }
    }
}

// FUNCTION: SWBF2 0x0043b0b0
void AILowLevel::ProcessChargeWpn(const Weapon* pWpn, bool* pFire)
{
    bool bLobbed = pWpn->GetOrdnanceGravity() != 0.0f;
    if (*pFire)
    {
        if (bLobbed && mGrenadeOffTimer + 4.0f > GameLoop::GetMissionTime())
        {
            *pFire = false;
        }
        else if (!mIsCharging)
        {
            mIsCharging = true;
            mChargeLimit = pWpn->mStateLimit;
            if (mOwner->mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier)
                && !pWpn->IsRtti(WeaponRepair::rttiHashWeaponRepair))
            {
                float fTop = AIDifficulty::GetChargeWeaponTop(mOwner);
                float fLimit = pWpn->mStateLimit;
                mChargeLimit = AIUtil::GetRandomFloat(AIDifficulty::GetChargeWeaponBot(mOwner) * fLimit,
                                                      fLimit * fTop);
            }
            *pFire = false;
            return;
        }
    }

    if (mIsCharging)
    {
        if (pWpn->mState == Weapon::CHARGE && pWpn->GetStateTimer() >= mChargeLimit)
        {
            *pFire = false;
            mIsCharging = false;
            mGrenadeOffTimer = GameLoop::GetMissionTime();
        }
        else
        {
            *pFire = true;
        }
    }
}

// FUNCTION: SWBF2 0x0043b1e0
void AILowLevel::ProcessBarrageWpn(const Weapon* pWpn, bool* pFire)
{
    if (!*pFire)
    {
        mBarrageState = STATE_UNDEFINED;
        return;
    }

    float fHeat = pWpn->GetHeat();
    int iAmmo = pWpn->m_pAmmoCounter->GetNumRounds();

    if (mBarrageState == STATE_UNDEFINED)
    {
        mBarrageHeat = 0.0f;
        mBarrageBeginDelay = -100.0f;
        mBarrageBeginAmmo = 0;
        mBarrageTotal = 0;
        mBarrageState = STATE_SETUP;
    }

    if (mBarrageState == STATE_SETUP)
    {
        int iMin = pWpn->mClass->GetBarrageMin();
        int iMax = pWpn->mClass->GetBarrageMax();
        float fRand = AIUtil::GetRandomFloat(0.0f, (float)(iMax - iMin));
        mBarrageState = STATE_WAITING;
        int iTotal = (int)(fRand * AIDifficulty::GetBarrageCountMultiplier(mOwner) + (float)iMin);
        if (iTotal < 2)
        {
            iTotal = 2;
        }
        mBarrageTotal = iTotal;
    }

    if (mBarrageState == STATE_WAITING)
    {
        float fWait = 1.6f - AIDifficulty::GetBarrageWaitMultiplier(mOwner);
        if (GameLoop::GetMissionTime() > pWpn->mClass->GetBarrageDelay() * fWait + mBarrageBeginDelay)
        {
            float fSalvos = (float)mBarrageTotal / pWpn->mClass->GetShotsPerSalvo();
            float fHeatPerSalvo = pWpn->mClass->GetHeatPerSalvo();
            float fHeatNeeded = (float)ceil(fSalvos) * fHeatPerSalvo;
            if (fHeatNeeded > 1.0f)
            {
                fHeatNeeded = 0.99f;
            }
            if (1.0f - fHeat > fHeatNeeded)
            {
                if (fHeatPerSalvo >= 1.0f)
                {
                    fHeatPerSalvo = 0.99f;
                }
                mBarrageBeginAmmo = iAmmo;
                mBarrageState = STATE_FIRING;
                mBarrageHeat = 1.0f - fHeatPerSalvo - 0.0001f;
            }
        }
    }

    if (mBarrageState == STATE_FIRING)
    {
        if (fHeat >= mBarrageHeat || mBarrageBeginAmmo - iAmmo >= mBarrageTotal)
        {
            mBarrageBeginDelay = GameLoop::GetMissionTime();
            mBarrageState = STATE_SETUP;
        }
    }

    if (mBarrageState != STATE_FIRING)
    {
        *pFire = false;
    }
}

// FUNCTION: SWBF2 0x0043b460
void AILowLevel::ProcessTriggerSingle(float dt, ControllableHeading* pHeading,
                                      Weapon* pWpn, bool* pFire)
{
    if (CombatUtil::IsSniperWpn(pWpn))
    {
        ProcessSniperWeapon(dt, pHeading, pFire);
        return;
    }

    if (!*pFire)
    {
        mTriggerSingleState = STATE_TRIGGER_UP;
        mTriggerSingleWaitUntil = 0.0f;
        return;
    }

    if (mTriggerSingleState == STATE_TRIGGER_DOWN)
    {
        *pFire = true;
        mTriggerSingleState = STATE_TRIGGER_UP;
        float fScale = 1.0f - AIDifficulty::GetTriggerSingleWaitDelayMultiplier(mOwner);
        if (fScale < 0.0f)
        {
            fScale = 0.0f;
        }
        float fDelay = AIUtil::GetRandomFloat(fScale * 0.2f, fScale * 2.0f);
        mTriggerSingleWaitUntil = fDelay + GameLoop::GetMissionTime();
    }
    else
    {
        *pFire = false;
        if (GameLoop::GetMissionTime() > mTriggerSingleWaitUntil)
        {
            mTriggerSingleState = STATE_TRIGGER_DOWN;
        }
    }

    if (pWpn->IsRtti(WeaponMelee::rttiHashWeaponMelee))
    {
        AI_ForceDeflectState(static_cast<WeaponMelee*>(pWpn), false);
    }
}

// FUNCTION: SWBF2 0x0043b560
static bool sIsChargeWpn(const Weapon* pWpn)
{
    bool bCannon = pWpn
        && pWpn->mClass->IsRtti(WeaponCannonClass::rttiHashWeaponCannonClass)
        && ((WeaponCannonClass*)pWpn->mClass)->mMaxPressedTime > 0.0f;
    bool bGrenade = pWpn
        && pWpn->mClass->IsRtti(WeaponGrenadeClass::rttiHashWeaponGrenadeClass)
        && ((WeaponGrenadeClass*)pWpn->mClass)->mMaxPressedTime > 0.0f;
    bool bCharge = bCannon || bGrenade;
    if (pWpn->mClass->IsRtti(WeaponLauncherClass::rttiHashWeaponLauncherClass))
    {
        return false;
    }
    return bCharge;
}

// FUNCTION: SWBF2 0x0043b5f0
void AILowLevel::ProcessFire(float dt, ControllableHeading* pHeading)
{
    Weapon* pWpn = mOwner->mOwner->GetCurWpn();
    bool bFire = mImmediateFire;
    if (!bFire && mFireUponAim && pWpn)
    {
        const GameObject* pTarget = AIUtil::GetAlivePtr(mTarget, false);
        if (pTarget)
        {
            bFire = CombatUtil::TestWpnConstraintsObj(mOwner->mOwner, pTarget);
        }
    }

    if (pWpn)
    {
        if (sIsChargeWpn(pWpn))
        {
            ProcessChargeWpn(pWpn, &bFire);
        }
        else if (pWpn->mClass->mTriggerSingle)
        {
            ProcessTriggerSingle(dt, pHeading, pWpn, &bFire);
        }
        else if (pWpn->mClass->IsRtti(WeaponCannonClass::rttiHashWeaponCannonClass))
        {
            ProcessBarrageWpn(pWpn, &bFire);
        }
    }

    if (bFire)
    {
        bFire = !AvoidPlayer(dt, pWpn);
        if (bFire && mOwner->IsTargetLOSPossiblyBlocked())
        {
            bFire = false;
        }
    }

    Controllable* pMe = mOwner->mOwner;
    Trigger* pFireTrigger = &pMe->mControlFire[pMe->mWpnChannel];
    Trigger* pOtherTrigger = &pMe->mControlFire[1 - pMe->mWpnChannel];
    Trigger* pReleaseTrigger = 0;
    if (pMe->mWpnChannel == 1
        && pMe->GetWeaponPtr(0)->IsRtti(WeaponMelee::rttiHashWeaponMelee))
    {
        pReleaseTrigger = pFireTrigger;
        pFireTrigger = &mOwner->mOwner->mControlReload;
    }

    pFireTrigger->Update(dt, bFire);
    pOtherTrigger->Update(dt, false);
    if (pReleaseTrigger)
    {
        pReleaseTrigger->Update(dt, false);
    }

    if (pFireTrigger->IsDown())
    {
        mOwner->mForceAudible = true;
    }
}

// FUNCTION: SWBF2 0x0043b760
bool AILowLevel::AvoidPlayer(float dt, const Weapon* pWpn)
{
    if (!pWpn || pWpn->IsRtti(WeaponRepair::rttiHashWeaponRepair)
        || pWpn->IsRtti(WeaponDispenser::rttiHashWeaponDispenser))
    {
        return false;
    }

    if (UnitControllerPtr(mOwner->mOwner->mCtrl))
    {
        UnitController* pController = UnitControllerPtr(mOwner->mOwner->mCtrl);
        for (int i = 0; i < pController->GetFriendlyFireCount(); ++i)
        {
            const GameObject* pFriend = pController->GetFriendlyFire(i);
            if (!pFriend || !GetCollisionObject(pFriend))
            {
                continue;
            }

            const Controllable* pMe = mOwner->mOwner;
            const PblVector3& center = GetCollisionObject(pFriend)->GetCollisionCenter();
            PblVector3 toFriend(center.x - pMe->mEyePoint.x,
                                center.y - pMe->mEyePoint.y,
                                center.z - pMe->mEyePoint.z);
            float fRadius = GetCollisionObject(pFriend)->GetCollisionRadius();

            const PblVector3& dir = mOwner->mOwner->mEyeDir;
            float fCrossX = dir.y * toFriend.z - dir.z * toFriend.y;
            float fCrossY = dir.z * toFriend.x - dir.x * toFriend.z;
            float fCrossZ = dir.x * toFriend.y - dir.y * toFriend.x;
            if (fRadius * fRadius > fCrossY * fCrossY + fCrossX * fCrossX + fCrossZ * fCrossZ
                && dir.x * toFriend.x + dir.y * toFriend.y + dir.z * toFriend.z > 0.0f)
            {
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0043b970
void AILowLevel::SetAimPoint(const PblVector3& aimPoint)
{
    mOwner->mOwner->mTargetInfo.SetAimPoint(aimPoint);
}

// FUNCTION: SWBF2 0x0043b990
void AILowLevel::SetFireRequest(bool bImmediate, bool bUponAim)
{
    mImmediateFire = bImmediate;
    mFireUponAim = bUponAim;
}

// FUNCTION: SWBF2 0x0043b9c0
void AILowLevel::SetTarget(const GameObject* pTarget)
{
    Weapon* pWpn = mOwner->mOwner->GetCurWpn();
    mTarget.mObject = pTarget;
    mTarget.mSavedHandleId = pTarget ? GameObjectHandleId(pTarget) : 0;

    Controllable* pMe = mOwner->mOwner;
    PblHandle<const GameObject>& reticule = pMe->mReticuleTarget[pMe->mWpnChannel];
    reticule.mObject = pTarget;
    reticule.mSavedHandleId = pTarget ? GameObjectHandleId(pTarget) : 0;

    if (pWpn)
    {
        if (pTarget)
        {
            int iTargetTeam = GetTeam(pTarget);
            int iMyTeam = GetTeam(mOwner->mOwner->GetGameObject());
            int iAffiliation = Team::sTeams[iMyTeam]->GetAffiliation(iTargetTeam);
            const WeaponClass* pClass = pWpn->mClass;
            if (!(pClass->mTargetSides & (1 << (iAffiliation + 1)))
                || !(pClass->mAITargetTypes & (1 << GetHealthType(pTarget))))
            {
                pTarget = 0;
            }
        }
        pWpn->mTarget.mObject = pTarget;
        pWpn->mTarget.mSavedHandleId = pTarget ? GameObjectHandleId(pTarget) : 0;
        pWpn->mTargetBodyID = -1;
    }
}

// FUNCTION: SWBF2 0x0043bac0
const GameObject* AILowLevel::GetTarget() const
{
    if (mTarget.mObject && GameObjectHandleId(mTarget.mObject) != mTarget.mSavedHandleId)
    {
        AILowLevel* pThis = const_cast<AILowLevel*>(this);
        pThis->mTarget.mObject = 0;
        pThis->mTarget.mSavedHandleId = 0;
    }
    return mTarget.mObject;
}

// FUNCTION: SWBF2 0x0043baf0
bool AILowLevel::GetAlert()
{
    return mIsAlert;
}

// FUNCTION: SWBF2 0x0043bb00
void AILowLevel::SetAlert()
{
    mIsAlert = true;
}

// FUNCTION: SWBF2 0x0043bb10
void AILowLevel::SetFastTurn()
{
    if (mOwner->mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier)
        || mOwner->mOwner->GetEntity()->IsRtti(EntityDroideka::rttiHashEntityDroideka))
    {
        mFastTurn = true;
    }
}

// FUNCTION: SWBF2 0x0043bb60
bool AILowLevel::UseFastTurn() const
{
    return mFastTurn;
}

// FUNCTION: SWBF2 0x0043bb70
bool AILowLevel::IsSafeToRoll(const PblVector3& dir)
{
    Controllable* pControllable = mOwner->mOwner;
    if (!pControllable || !pControllable->GetGameObject())
    {
        return false;
    }

    PblVector3 dirXZ = AIUtil::GetSafeNormalXZ(dir);
    const PblVector3& pos = *(const PblVector3*)&GetMatrix().m[3][0];
    PblVector3 dest(pos.x + dirXZ.x * 4.0f,
                    pos.y + dirXZ.y * 4.0f,
                    pos.z + dirXZ.z * 4.0f);
    return !PathFinder::TestTrivialStraightLineHit(pos, dest,
                                                   mOwner->mOwner->GetGameObject());
}

// FUNCTION: SWBF2 0x0043bc40
void AILowLevel::SetNextWeapon(int iIndex)
{
    mIsCharging = false;
    mNextWeaponIndex = iIndex;
    mNextSniperFireTime = 0.0f;
    mGrenadeOffTimer = -1.0f;
    mBarrageState = STATE_UNDEFINED;
}

// FUNCTION: SWBF2 0x0043bc80
bool AILowLevel::UpdateIndirect(float fDeltaTime)
{

    Controllable* pMe = mOwner->mOwner;
    if (pMe)
    {
        pMe->GetEntity();
    }

    if (mBoardVehicle)
    {
        mBoardVehicle = false;
        GameObject* pVehicle = AIUtil::GetAlivePtr(mVehicle, false);
        if (pVehicle && mOwner->mOwner->EnterVehicle(pVehicle))
        {
            return false;
        }
    }

    if (mExitVehicle)
    {
        mExitVehicle = false;
        Controllable* pOwner = mOwner->mOwner;
        if (pOwner->mCharacter && pOwner->mCharacter->mVehicle)
        {
            bool bForce = true;
            if (mOwner && mOwner->GetLastSquadCommand().mWhatOrder == SquadCommands::SC_EXITVEHICLE)
            {
                bForce = false;
            }
            if (pOwner->GetPilot()->ExitVehicle(bForce, true))
            {
                return false;
            }
        }
    }

    if (mNextWeaponIndex >= 0)
    {
        Weapon* pNextWpn = mOwner->mOwner->GetWeaponPtr(mNextWeaponIndex);
        Controllable* pOwner = mOwner->mOwner;
        pOwner->mWpnChannel = pNextWpn->GetTrigger() != &pOwner->mControlFire[0];
        int iCurIndex = mOwner->mOwner->GetWeaponIndex(mOwner->mOwner->mWpnChannel);
        if (iCurIndex != mNextWeaponIndex)
        {
            Weapon* pCurWpn = mOwner->mOwner->GetWeaponPtr(iCurIndex);
            if (pCurWpn->mSelectedFlag)
            {
                pCurWpn->Deselect(0, pNextWpn->mClass->mIsOffhand);
            }
            mOwner->mOwner->SetWeaponIndex(mNextWeaponIndex, mOwner->mOwner->mWpnChannel);
            if (!pNextWpn->mSelectedFlag)
            {
                pNextWpn->Select(0, pCurWpn->mClass->mIsOffhand);
            }
            mOwner->mOwner->mTargetInfo.mIsAiming = false;
        }
        mNextWeaponIndex = -1;
    }
    return true;
}

// FUNCTION: SWBF2 0x0043be40
void AILowLevel::Update(float dt)
{
    if (mNavigator)
    {
        mNavigator->Update(dt);
    }
    else
    {
        SetDestCurrent();
    }

    if (mHeadLookState)
    {
        UpdateHeadLook(dt);
    }
}

// FUNCTION: SWBF2 0x0043be80
void AILowLevel::UpdateSkip()
{
    if (mNavigator)
    {
        mNavigator->UpdateSkip();
    }
    else
    {
        SetDestCurrent();
    }
}

// FUNCTION: SWBF2 0x0043bea0
void AILowLevel::DeleteNavigator()
{
    PathManager::RemovePathRequest(mOwner);
    if (mNavigator)
    {
        mNavigator->Kill();
        delete mNavigator;
        mNavigator = 0;
    }
}

// FUNCTION: SWBF2 0x0043bed0
bool AILowLevel::IsNavigatorInProgress()
{
    return mNavigator && !mNavigator->IsDone();
}

// FUNCTION: SWBF2 0x0043bef0
bool AILowLevel::IsNavigatorFailedPath()
{
    return mNavigator && mNavigator->IsFailedPathSearch();
}

// FUNCTION: SWBF2 0x0043bf10
void AILowLevel::Navigator_Wait(float fTime)
{
    if (mNavigator && mNavigator->GetPathFollower())
    {
        mNavigator->GetPathFollower()->Wait(fTime);
    }
}

// FUNCTION: SWBF2 0x0043c100
void AILowLevel::SetNavigator_GotoDirect(const PblVector3& dest,
                                         bool bFlyerIgnoreBarriers, bool bUnused)
{
    AIPath* pPath = new AIPath();
    if (pPath)
    {
        pPath->Append(*(const PblVector3*)&GetMatrix().m[3][0], 0);
        pPath->Append(dest, 0);
        SetNavigator_FollowPath(pPath, false, bFlyerIgnoreBarriers, false);
        pPath->Release();
    }
}

// FUNCTION: SWBF2 0x0043bf30
void AILowLevel::SetNavigator_Goto(const PblVector3& dest, const Obstacle* pIgnore,
                                   bool bFlyerIgnoreBarriers, bool bRollIntoDest)
{
    DeleteNavigator();
    const GameObject* pObj = mOwner->mOwner->GetGameObject();
    const PblVector3& pos = *(const PblVector3*)&GetMatrix().m[3][0];
    PblVector3 vDest = dest;

    const Obstacle* ignore[2];
    ignore[0] = AIUtil::GetGameObjectBarrier(pObj);
    ignore[1] = pIgnore;
    PathFinder::GetClosestValidPoint(vDest, vDest, AIUtil::GetCollisionRadius(pObj),
                                     AIUtil::GetAIType(pObj), ignore, 2);

    bool bFar = true;
    if (!bFlyerIgnoreBarriers)
    {
        bFar = PathManager::GetPathPriority(&pos, &vDest);
    }

    if (!mOwner->mOwner->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer) && bFar)
    {
        if (ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1)
            && (ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1)->mAllowTypes
                & AIUtil::GetAIType(pObj)))
        {
            mNavigator = new NavigatorGraphFollower(this, vDest, ignore, 2, bRollIntoDest);
            return;
        }
        if (ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1))
        {
            AIUtil::GetAIType(pObj);
        }
    }

    mNavigator = new NavigatorPathFollower(this, false, ignore, 2, bRollIntoDest);
    PblVector3 vStart = pos;
    PathFinder::GetClosestValidPoint(vStart, vStart, AIUtil::GetCollisionRadius(pObj),
                                     AIUtil::GetAIType(pObj), ignore, 2);
    PathManager::RequestPath(mOwner, vStart, vDest, AIUtil::GetCollisionRadius(pObj),
                             AIUtil::GetAIType(pObj), 0, 0, ignore, 2);
}

// FUNCTION: SWBF2 0x0043c190
void AILowLevel::SetNavigator_FollowPath(AIPath* pPath, bool bDoCallbacksOnPts,
                                         bool bFlyerIgnoreBarriers,
                                         bool bFlyerIgnoreSafeHeight)
{
    DeleteNavigator();
    mFlyerIgnoreBarriers = bFlyerIgnoreBarriers;
    mFlyerIgnoreSafeHeight = bFlyerIgnoreSafeHeight;
    const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(mOwner->mOwner->GetGameObject());
    mNavigator = new NavigatorPathFollower(this, bDoCallbacksOnPts, &pIgnore, 1,
                                           false, pPath);
}

// FUNCTION: SWBF2 0x0043c240
void AILowLevel::SetNavigator_FollowTarget(const GameObject* pTarget,
                                           const PblVector3& followOffset)
{
    if (NavigatorFollowTargetPtr(mNavigator)
        && NavigatorFollowTargetPtr(mNavigator)->IsEqual(this, pTarget, followOffset))
    {
        return;
    }

    DeleteNavigator();
    const Obstacle* ignore[2];
    ignore[0] = AIUtil::GetGameObjectBarrier(mOwner->mOwner->GetGameObject());
    ignore[1] = AIUtil::GetGameObjectBarrier(pTarget);
    mNavigator = new NavigatorFollowTarget(this, pTarget, ignore, 2, followOffset);
}

// FUNCTION: SWBF2 0x0043c300
void AILowLevel::SetNavigator_Slide(const PblVector3& dest, const Obstacle* pIgnore)
{
    if (NavigatorSlidePtr(mNavigator) && NavigatorSlidePtr(mNavigator)->IsEqual(this, dest))
    {
        return;
    }

    DeleteNavigator();
    const Obstacle* ignore[2];
    ignore[0] = AIUtil::GetGameObjectBarrier(mOwner->mOwner->GetGameObject());
    ignore[1] = pIgnore;
    mNavigator = new NavigatorSlide(this, dest, ignore, 2);
}

// FUNCTION: SWBF2 0x0043c3c0
void AILowLevel::SetNavigator_Slide(const GameObject* pSlideObj, float fOffset,
                                    bool bUseAttackHelper, const Obstacle* pIgnore)
{
    if (NavigatorSlidePtr(mNavigator)
        && NavigatorSlidePtr(mNavigator)->IsEqual(this, pSlideObj, fOffset, bUseAttackHelper))
    {
        return;
    }

    DeleteNavigator();
    const Obstacle* ignore[2];
    ignore[0] = AIUtil::GetGameObjectBarrier(mOwner->mOwner->GetGameObject());
    ignore[1] = pIgnore;
    mNavigator = new NavigatorSlide(this, pSlideObj, fOffset, bUseAttackHelper,
                                    ignore, 2);
}

// FUNCTION: SWBF2 0x0043c470
void AILowLevel::SetNavigator_Stationary()
{
    DeleteNavigator();
}

// FUNCTION: SWBF2 0x0043c480
void AILowLevel::ReceivePathResult(AIPath* pPath)
{
    if (mNavigator)
    {
        mNavigator->ReceivePathResult(pPath);
    }
    else if (pPath)
    {
        SetNavigator_FollowPath(pPath, false, true, false);
    }

    if (!pPath && mNavigator)
    {
        mNavigator->SetFailedPathSearch();
    }
}

// FUNCTION: SWBF2 0x0043c4d0
PblVector3 AILowLevel::GetGlobalDest()
{
    if (mNavigator)
    {
        return mNavigator->GetGlobalDest();
    }
    return GetLocalDest();
}

// FUNCTION: SWBF2 0x0043c510
PblVector3 AILowLevel::GetLocalDest()
{
    if (mDest.mHasDest)
    {
        return mDest.mPos;
    }
    return *(const PblVector3*)&GetMatrix().m[3][0];
}

// FUNCTION: SWBF2 0x0043c550
AI::HeadLookState AILowLevel::GetHeadLookState() const
{
    return mHeadLookState;
}

// FUNCTION: SWBF2 0x0043c560
void AILowLevel::UpdateHeadLook(float dt)
{
    if (mHeadLookState != AI::HEAD_LOOK_RIGHT && mHeadLookState != AI::HEAD_LOOK_LEFT &&
        mHeadLookState != AI::HEAD_LOOK_OBJECT)
    {
        return;
    }

    mHeadLook_Time -= dt;
    if (mHeadLook_Time < 0.0f)
    {
        if (mHeadLook_TurnToTime > 0.0f)
        {
            mHeadLook_TurnToTime = 0.0f;
            mHeadLook_Time = mHeadLook_StayTime;
        }
        else if (mHeadLook_StayTime > 0.0f)
        {
            mHeadLook_StayTime = 0.0f;
            mHeadLook_Time = mHeadLook_TurnBackTime;
        }
        else if (mHeadLook_TurnBackTime > 0.0f)
        {
            mHeadLook_TurnBackTime = 0.0f;
            mHeadLook_Time = 0.0f;
            mHeadLookState = AI::HEAD_LOOK_OFF;
        }
    }
}

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x0043c610
bool AILowLevel::CalcHeadLookMatrix(const PblMatrix* pHead, PblMatrix* pOut)
{
    float fBlend = 0.0f;
    if (mHeadLook_TurnToTime > 0.0f)
    {
        fBlend = PblMath::Cos(mHeadLook_Time / mHeadLook_TurnToTime * PblMath::fHalfPi);
    }
    else if (mHeadLook_StayTime > 0.0f)
    {
        fBlend = 1.0f;
    }
    else if (mHeadLook_TurnBackTime > 0.0f)
    {
        fBlend = PblMath::Cos(PblMath::fHalfPi - mHeadLook_Time / mHeadLook_TurnBackTime * PblMath::fHalfPi);
    }

    float fSin, fCos;
    PblQuaternion q;
    if (mHeadLookState == AI::HEAD_LOOK_OBJECT)
    {
        const GameObject* pObj = mHeadLook_Object;
        if (!pObj || !pObj->GetControllable())
        {
            return false;
        }

        PblMatrix mHead;
        D3DXMatrixMultiply(&mHead, pHead, &GetMatrix());
        PblVector3 vFwd;
        D3DXVec3Normalize(&vFwd, (const PblVector3*)&mHead.m[1][0]);
        const PblVector3& vEye = pObj->GetControllable()->mEyePoint;
        PblVector3 vTo(vEye.x - mHead.m[3][0], vEye.y - mHead.m[3][1], vEye.z - mHead.m[3][2]);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vTo);

        const float fDot = vFwd.y * vDir.y + vDir.x * vFwd.x + vFwd.z * vDir.z;
        if (fDot < -0.7f)
        {
            return false;
        }
        float fTurn = fBlend * 1.3694384f;
        if (fDot >= 0.2f)
        {
            fTurn = (float)acos((double)fDot) * fBlend;
        }

        PblVector3 vCross(vFwd.y * vDir.z - vDir.y * vFwd.z,
                          vFwd.z * vDir.x - vDir.z * vFwd.x,
                          vDir.y * vFwd.x - vFwd.y * vDir.x);
        PblVector3 vAxis;
        D3DXVec3Normalize(&vAxis, &vCross);
        PblMath::SinCos(&fSin, &fCos, fTurn * 0.5f);
        q._fS = fCos;
        q._fX = vAxis.x * fSin;
        q._fY = vAxis.y * fSin;
        q._fZ = vAxis.z * fSin;

        PblVector3 vRotated = q.RotateVector(vFwd);
        PblVector3 vY;
        D3DXVec3Normalize(&vY, &vRotated);
        const PblVector3& vUp = gVectorUnitY;
        PblVector3 vSide(vY.z * vUp.y - vY.y * vUp.z,
                         vY.x * vUp.z - vY.z * vUp.x,
                         vY.y * vUp.x - vY.x * vUp.y);
        PblVector3 vZ;
        D3DXVec3Normalize(&vZ, &vSide);
        const PblVector3 vX(vZ.z * vY.y - vZ.y * vY.z,
                            vZ.x * vY.z - vZ.z * vY.x,
                            vZ.y * vY.x - vY.y * vZ.x);

        const PblVector3* pRows[4] = { &vX, &vY, &vZ, (const PblVector3*)&mHead.m[3][0] };
        for (int r = 0; r < 4; ++r)
        {
            pOut->m[r][0] = pRows[r]->x;
            pOut->m[r][1] = pRows[r]->y;
            pOut->m[r][2] = pRows[r]->z;
            pOut->m[r][3] = r == 3 ? 1.0f : 0.0f;
        }
        const PblMatrix mInv = GetMatrix().GetInverseFast();
        D3DXMatrixMultiply(pOut, pOut, &mInv);
        return true;
    }

    if (mHeadLookState != AI::HEAD_LOOK_RIGHT && mHeadLookState != AI::HEAD_LOOK_LEFT)
    {
        return false;
    }
    if (mHeadLookState == AI::HEAD_LOOK_RIGHT)
    {
        fBlend = -fBlend;
    }
    PblMath::SinCos(&fSin, &fCos, fBlend * 0.5f);
    q._fS = fCos;
    q._fX = fSin;
    q._fY = fSin * 0.0f;
    q._fZ = fSin * 0.0f;
    PblQuaternion qUnit;
    D3DXMatrixRotationQuaternion(pOut, D3DXQuaternionNormalize(&qUnit, &q));
    pOut->m[3][0] = 0.0f;
    pOut->m[3][1] = 0.0f;
    pOut->m[3][2] = 0.0f;
    pOut->m[3][3] = 1.0f;
    const PblMatrix mHeadCopy = *pHead;
    D3DXMatrixMultiply(pOut, pOut, &mHeadCopy);
    return true;
}

// FUNCTION: SWBF2 0x0043cb80
void AILowLevel::HeadLook_Glance(bool bRight, float fTurnToTime, float fStayTime, float fTurnBackTime)
{
    if (mHeadLookState == AI::HEAD_LOOK_RIGHT || mHeadLookState == AI::HEAD_LOOK_LEFT)
    {
        return;
    }
    mHeadLook_TurnToTime = fTurnToTime;
    mHeadLook_StayTime = fStayTime;
    mHeadLook_TurnBackTime = fTurnBackTime;
    mHeadLookState = bRight ? AI::HEAD_LOOK_RIGHT : AI::HEAD_LOOK_LEFT;
    mHeadLook_Time = fTurnToTime;
}

// FUNCTION: SWBF2 0x0043cbd0
void AILowLevel::HeadLook_Object(const GameObject* pObj, float fTurnToTime, float fStayTime, float fTurnBackTime)
{
    if (mOwner->mLodState < 3)
    {
        return;
    }

    if (mHeadLookState == AI::HEAD_LOOK_OBJECT && mHeadLook_Object == pObj)
    {
        if (mHeadLook_TurnToTime > 0.0f)
        {
        }
        else if (mHeadLook_StayTime > 0.0f)
        {
            mHeadLook_Time = fStayTime;
        }
        else if (mHeadLook_TurnBackTime > 0.0f)
        {
            mHeadLook_Time = (mHeadLook_TurnToTime - mHeadLook_Time) / mHeadLook_TurnToTime * fTurnToTime;
            mHeadLook_TurnToTime = fTurnToTime;
        }
        else
        {
            return;
        }
    }
    else
    {
        mHeadLook_Object = pObj;
        mHeadLookState = AI::HEAD_LOOK_OBJECT;
        mHeadLook_Time = fTurnToTime;
        mHeadLook_TurnToTime = fTurnToTime;
    }
    mHeadLook_TurnBackTime = fTurnBackTime;
    mHeadLook_StayTime = fStayTime;
}
