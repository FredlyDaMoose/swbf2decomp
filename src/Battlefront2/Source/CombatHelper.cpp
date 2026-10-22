#include "CombatHelper.h"

#include "AIUtil.h"
#include "AILowLevel.h"
#include "AttackHelper.h"
#include "Character.h"
#include "CollisionManager.h"
#include "CombatUtil.h"
#include "EventInfo.h"
#include "PathFinder.h"
#include "Team.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "UnitAgent.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "WeaponMeleeInstance.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

#include "UnitThreatManager.h"

class Weapon;

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationY(PblMatrix* pOut, float fAngle);

namespace CombatUtil
{

    bool SelectCombatResponse(Controllable* pMe, const GameObject* pTarget,
                              AttackPatterns::State* pState, float* pTime,
                              bool* pbFlag, bool bA, bool bB);

    bool EngageInCombat(const UnitController* pCtrl, const GameObject* pTarget, bool bFlag);
}

namespace AI
{

enum VOType
{
    VOT_RebelsShootDeadBody = 11
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget, const GameObject* pOther, float fDelay);
}
}

namespace CombatUtil { float GetCombatRange(const Controllable* pMe); }

namespace
{

    const PblHash kRttiHashEntityFlyer("EntityFlyer");
    const PblHash kRttiHashEntitySoldier("EntitySoldier");
    const PblHash kRttiHashEntityDroideka("EntityDroideka");
    const PblHash kRttiHashEntityHover("EntityHover");
    const PblHash kRttiHashEntityWalker("EntityWalker");
    const PblHash kRttiHashWeaponMelee("WeaponMelee");
    const PblHash kRttiHashMountedTurret("MountedTurret");

    const GameObject* HandleObject(PblHandle<const GameObject>& handle)
    {
        if (handle.mObject &&
            handle.mObject->mHandleId != handle.mSavedHandleId)
        {
            handle.mObject = 0;
            handle.mSavedHandleId = 0;
        }
        return handle.mObject;
    }

    void HandleSet(PblHandle<const GameObject>& handle, const GameObject* pObj)
    {
        handle.mObject = pObj;
        handle.mSavedHandleId = pObj ? pObj->mHandleId : 0;
    }

    const PblVector3& GetPosition(const GameObject* pObj)
    {
        return static_cast<const CollisionObject*>(pObj)->GetPosition();
    }

    bool WeaponIsRtti(Weapon* pWpn, unsigned int uRtti)
    {
        return pWpn->IsRtti(uRtti);
    }

    void AI_ForceDeflectState(Weapon* pMelee, bool bDeflect)
    {
        static_cast<WeaponMelee*>(pMelee)->m_bAI_Deflect = bDeflect;
    }

    const Damageable* GetDamageable(const GameObject* pObj) { return static_cast<const Damageable*>(pObj); }
    bool IsShielded(const Damageable* pDamageable) { return pDamageable->mIsShielded; }
    float GetCurShield(const Damageable* pDamageable) { return pDamageable->mCurShield; }
    float GetMaxShield(const Damageable* pDamageable) { return pDamageable->mMaxShield; }

    float DotXZ(const PblVector3& vTarget, const PblVector3& vFacing)
    {
        PblVector3 a, b, vFacingXZ = vFacing, vTargetXZ = vTarget;
        vTargetXZ.y = 0.0f;
        vFacingXZ.y = 0.0f;
        D3DXVec3Normalize(&a, &vFacingXZ);
        D3DXVec3Normalize(&b, &vTargetXZ);
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
}

// SYNTHETIC: SWBF2 0x0046c2e0
// CombatHelper::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0046c330
CombatHelper::CombatHelper(UnitAgent* pOwner)
    : HelperFsm(STATE_START, STATE_START)
    , mOwner(pOwner)
    , mAttackPatternHelper(pOwner)
{
    mTargetHandle.mObject = 0;
    mTargetHandle.mSavedHandleId = 0;
    mTargetPos.x = 0.0f;
    mTargetPos.y = 0.0f;
    mTargetPos.z = 0.0f;
    mWaiting = false;
    mWaitOverTime = 0.0f;
    mLookAngle = 0.0f;
    mLookCount = 0;
    mPauseTime = 0.0f;
    mFleeSrc.x = 0.0f;
    mFleeSrc.y = 0.0f;
    mFleeSrc.z = 0.0f;
    mFleeTime = 0.0f;
    mFleeAllowRoll = true;
    mDeadBodyWeaponIdx = 0;
    mDeflect = false;
    mDeflectTimer = 0.0f;
}

CombatHelper::~CombatHelper()
{
}

// FUNCTION: SWBF2 0x0046c400
void CombatHelper::Kill()
{
    mAttackPatternHelper.Kill();
    Fsm::Kill();
}

UnitController* CombatHelper::GetCtrl()
{
    return mOwner->mCtrl;
}

Controllable* CombatHelper::GetMe()
{
    return mOwner->mCtrl->mOwner;
}

AILowLevel& CombatHelper::GetLowLevel()
{
    return *reinterpret_cast<AILowLevel*>(mOwner->mCtrl->mLowLevel);
}

AttackHelper& CombatHelper::GetAttackHelper()
{
    return *reinterpret_cast<AttackHelper*>(mOwner->mAttackHelper);
}

// FUNCTION: SWBF2 0x0046c420
void CombatHelper::Init(const UnitCombatInfo& info)
{
    mDeflect = true;
    mDeflectTimer = AIUtil::GetRandomFloat(3.0f, 6.0f) + GameLoop::GetMissionTime();

    switch (info.GetState())
    {
    case UnitCombatInfo::TURN:
        SetNextState(STATE_Turn);
        mTargetPos = info.GetTargetPos();
        break;
    case UnitCombatInfo::ENGAGE:
        SetNextState(STATE_Engage);
        if (info.GetWho())
        {
            GetAttackHelper().SetVisibleTarget(info.GetWho());
        }
        break;
    case UnitCombatInfo::FLEE_EVENT:
        SetNextState(STATE_FleeEvent);
        mFleeSrc = info.GetTargetPos();
        mFleeAllowRoll = info.GetAllowRoll();
        break;
    case UnitCombatInfo::INVESTIGATE:
    {
        SetNextState(STATE_Investigate);
        HandleSet(mTargetHandle, info.GetWho());
        const GameObject* pTarget = HandleObject(mTargetHandle);
        if (pTarget)
        {
            mTargetPos = GetPosition(pTarget);
        }
        else
        {
            mTargetPos = info.GetTargetPos();
        }
        mPauseTime = AIUtil::GetRandomFloat(1.0f, 4.0f) + GameLoop::GetMissionTime();
        break;
    }
    }

    FSMChangeState();
}

// FUNCTION: SWBF2 0x0046c5f0
void CombatHelper::Clean()
{
    ExitState();
}

// FUNCTION: SWBF2 0x0046c600
float CombatHelper::GetUpdateRate() const
{
    return 0.4f;
}

// FUNCTION: SWBF2 0x0046c610
void CombatHelper::EnterState()
{
    mEnterStateTime = GameLoop::GetMissionTime();
    GetLowLevel().mFoldStatus = FOLD_UNFOLD;

    if ((GetMe()->GetEntity()->IsRtti(kRttiHashEntityFlyer) &&
         mCurState > STATE_START && mCurState < STATE_FINAL && mCurState != STATE_Engage) ||
        (mCurState == STATE_LookAround &&
         !GetMe()->GetEntity()->IsRtti(kRttiHashEntitySoldier) &&
         !GetMe()->GetEntity()->IsRtti(kRttiHashEntityDroideka)))
    {
        SetNextState(100);
        return;
    }

    switch (mCurState)
    {
    case STATE_Turn:
    {
        GetLowLevel().SetNavigator_Stationary();
        const PblMatrix& mMe = GetMe()->GetMatrix();
        PblVector3 vDelta(mTargetPos.x - mMe.m[3][0], mTargetPos.y - mMe.m[3][1],
                          mTargetPos.z - mMe.m[3][2]);
        if (vDelta.x * vDelta.x + vDelta.z * vDelta.z < 100.0f)
        {
            const PblVector3 vDir = AIUtil::GetSafeNormalXZ(vDelta);
            mTargetPos.x = vDir.x * 15.0f + mMe.m[3][0];
            mTargetPos.y = mMe.m[3][1] + vDir.y * 15.0f;
            mTargetPos.z = mMe.m[3][2] + vDir.z * 15.0f;
        }
        GetLowLevel().SetHeading(mTargetPos, 0.0f);
        GetLowLevel().SetHeight(HEIGHT_STAND);
        mWaiting = false;
        break;
    }

    case STATE_LookAround:
        GetLowLevel().SetNavigator_Stationary();
        mLookCount = 0;
        SetRandomHeading();
        mWaiting = false;
        break;

    case STATE_Investigate:
        GetLowLevel().SetNavigator_Goto(mTargetPos, 0, false, false);
        break;

    case STATE_Engage:
    {
        GetLowLevel().SetNavigator_Stationary();
        if (!GetAttackHelper().HasTarget())
        {
            SetNextState(100);
            return;
        }
        GetLowLevel().HeadLook_Object(GetAttackHelper().GetTarget(), 0.05f, 1.0f, 0.3f);
        float fTime = 0.0f;
        bool bFlag;
        if (!CombatUtil::SelectCombatResponse(GetMe(), GetAttackHelper().GetTarget(),
                                              &mAttackPattern, &fTime, &bFlag, true, false))
        {
            SetNextState(100);
            return;
        }
        mAttackPatternHelper.HELPERSetNextState(mAttackPattern, fTime);
        break;
    }

    case STATE_FleeEvent:
    {
        const PblMatrix& mMe = GetMe()->GetMatrix();
        PblVector3 vAway(mMe.m[3][0] - mFleeSrc.x, mMe.m[3][1] - mFleeSrc.y,
                         mMe.m[3][2] - mFleeSrc.z);
        const PblVector3 vDir = AIUtil::GetSafeNormalXZ(vAway);
        PblVector3 vDest(vDir.x * 20.0f + mMe.m[3][0], mMe.m[3][1] + vDir.y * 20.0f,
                         vDir.z * 20.0f + mMe.m[3][2]);

        if (GetMe()->GetEntity()->IsRtti(kRttiHashEntityHover) ||
            GetMe()->GetEntity()->IsRtti(kRttiHashEntityWalker))
        {

            const PblMatrix& mNow = GetMe()->GetMatrix();
            vDest.x = mNow.m[2][0] * 20.0f + mMe.m[3][0];
            vDest.y = mMe.m[3][1] + mNow.m[2][1] * 20.0f;
            vDest.z = mNow.m[2][2] * 20.0f + mMe.m[3][2];
            GetLowLevel().SetNavigator_Goto(vDest, 0, false, false);
            mFleeTime = GameLoop::GetMissionTime() + 5.0f;
            break;
        }

        bool bRolled = false;
        if (mFleeAllowRoll && AIUtil::GetRandomInt(0, 1) == 0)
        {
            PblVector3 vToDest(vDest.x - mMe.m[3][0], vDest.y - mMe.m[3][1], vDest.z - mMe.m[3][2]);
            const PblVector3 vRollDir = AIUtil::GetSafeNormalXZ(vToDest);
            if (GetLowLevel().IsSafeToRoll(vRollDir))
            {
                GetLowLevel().SetRoll(vRollDir);
                GetLowLevel().SetNavigator_Stationary();
                bRolled = true;
            }
        }
        if (!bRolled)
        {
            GetLowLevel().SetNavigator_Slide(vDest, 0);
        }

        GetLowLevel().SetHeight(HEIGHT_STAND);
        GetLowLevel().SetHeadingDefault();
        mFleeTime = GameLoop::GetMissionTime() + 2.0f;
        break;
    }
    }
}

// FUNCTION: SWBF2 0x0046cb70
void CombatHelper::ExitState()
{
    GetLowLevel().mFoldStatus = FOLD_FOLD;

    Weapon* pWpn = GetMe()->GetCurWpn();
    if (pWpn && WeaponIsRtti(pWpn, kRttiHashWeaponMelee))
    {
        AI_ForceDeflectState(pWpn, false);
    }

    switch (mCurState)
    {
    case STATE_Engage:
        mAttackPatternHelper.HELPERClean();
        break;
    case STATE_FleeEvent:
        GetMe()->EndSprint();
        break;
    }
}

// FUNCTION: SWBF2 0x0046cbe0
void CombatHelper::SetRandomHeading()
{
    if (mLookCount == 0)
    {
        mLookAngle = AIUtil::GetRandomFloat(30.0f, 75.0f) * 0.017453292f;
        if (AIUtil::GetRandomInt(0, 1) == 0)
        {
            mLookAngle = mLookAngle * -1.0f;
        }
    }
    else
    {
        mLookAngle = mLookAngle * -2.0f;
    }

    PblMatrix mLook = GetMe()->GetMatrix();
    PblMatrix mRot;
    D3DXMatrixMultiply(&mLook, &mLook, D3DXMatrixRotationY(&mRot, mLookAngle));
    const float fX = mLook.m[2][0] * 100.0f;
    const float fZ = mLook.m[2][2] * 100.0f;
    const PblMatrix& mMe = GetMe()->GetMatrix();
    mTargetPos.x = mMe.m[3][0] + fX;
    mTargetPos.y = mMe.m[3][1] + 0.01f;
    mTargetPos.z = mMe.m[3][2] + fZ;
    GetLowLevel().SetHeading(mTargetPos, 0.0f);
    ++mLookCount;
}

// FUNCTION: SWBF2 0x0046cf70
void CombatHelper::UpdateState(float dt)
{
    GetLowLevel().mFoldStatus = FOLD_UNFOLD;
    const bool bNewTarget = GetAttackHelper().UpdateTarget();
    GetAttackHelper().Aim();
    GetLowLevel().SetAlert();

    Weapon* pWpn = GetMe()->GetCurWpn();
    if (pWpn && WeaponIsRtti(pWpn, kRttiHashWeaponMelee))
    {
        if (GameLoop::GetMissionTime() > mDeflectTimer)
        {
            mDeflect = !mDeflect;
            if (mDeflect)
            {
                mDeflectTimer = AIUtil::GetRandomFloat(3.0f, 7.0f) + GameLoop::GetMissionTime();
            }
            else
            {
                mDeflectTimer = AIUtil::GetRandomFloat(1.5f, 4.0f) + GameLoop::GetMissionTime();
            }
        }
        AI_ForceDeflectState(pWpn, mDeflect);
    }

    switch (mCurState)
    {
    case STATE_Turn:
    {
        Controllable* pMe = GetMe();
        PblVector3 vToTarget(mTargetPos.x - pMe->mEyePoint.x, mTargetPos.y - pMe->mEyePoint.y,
                             mTargetPos.z - pMe->mEyePoint.z);
        if (DotXZ(vToTarget, GetMe()->mEyeDir) > 0.939f)
        {
            if (GetAttackHelper().HasTarget())
            {
                SetNextState(STATE_Engage);
                break;
            }
            if (!mWaiting)
            {
                mWaiting = true;
                mWaitOverTime = GameLoop::GetMissionTime() + 0.75f;
            }
            else if (GameLoop::GetMissionTime() > mWaitOverTime)
            {
                SetNextState(STATE_LookAround);
                break;
            }
        }
        GetLowLevel().SetFastTurn();
        GetLowLevel().SetHeading(mTargetPos, 0.0f);
        break;
    }

    case STATE_LookAround:
    {
        if (GetAttackHelper().HasTarget())
        {
            SetNextState(STATE_Engage);
            break;
        }
        Controllable* pMe = GetMe();
        PblVector3 vToTarget(mTargetPos.x - pMe->mEyePoint.x, mTargetPos.y - pMe->mEyePoint.y,
                             mTargetPos.z - pMe->mEyePoint.z);
        if (DotXZ(vToTarget, GetMe()->mEyeDir) > 0.939f)
        {
            if (!mWaiting)
            {
                mWaiting = true;
                mWaitOverTime = GameLoop::GetMissionTime() + 0.4f;
            }
            else if (GameLoop::GetMissionTime() > mWaitOverTime)
            {
                if (mLookCount >= 2)
                {
                    SetNextState(100);
                    break;
                }
                SetRandomHeading();
                mWaiting = false;
                break;
            }
        }
        GetLowLevel().SetFastTurn();
        break;
    }

    case STATE_Investigate:
    {
        if (!GetLowLevel().IsNavigatorInProgress())
        {
            SetNextState(STATE_LookAround);
            break;
        }
        const float fTime = GameLoop::GetMissionTime();
        if (fTime > mPauseTime)
        {
            mPauseTime = AIUtil::GetRandomFloat(3.0f, 6.0f) + fTime;
            const bool bRight = (PblRandom::GetGlobalInt() & 1) != 0;
            const float fTurnTo = AIUtil::GetRandomFloat(0.1f, 0.3f);
            const float fStay = AIUtil::GetRandomFloat(0.4f, 0.7f);
            const float fTurnBack = AIUtil::GetRandomFloat(0.1f, 0.3f);
            if ((PblRandom::GetGlobalInt() & 1) && HandleObject(mTargetHandle))
            {
                GetLowLevel().HeadLook_Object(HandleObject(mTargetHandle), fTurnTo, fStay, fTurnBack);
            }
            else
            {
                GetLowLevel().HeadLook_Glance(bRight, fTurnTo, fStay, fTurnBack);
            }
            GetLowLevel().Navigator_Wait(fStay + fTurnTo + fTurnBack);
        }
        if (GameLoop::GetMissionTime() > mEnterStateTime + 10.0f)
        {
            SetNextState(STATE_LookAround);
            break;
        }
        GetLowLevel().SetHeadingDefault();
        break;
    }

    case STATE_Engage:
    {
        if (DeadBodyCheck())
        {
            break;
        }

        if (mAttackPatternHelper.IsJetJumpHinting() && GetLowLevel().mJetJumping)
        {
            mAttackPatternHelper.FSMUpdate(dt, false);
            break;
        }

        if (!GetAttackHelper().HasTarget())
        {
            SetNextState(STATE_LookAround);
            break;
        }

        const PblMatrix& mMe = GetMe()->GetMatrix();
        const PblVector3& vLastSeen = GetAttackHelper().GetLastSeenTargetPos();
        const float fDX = vLastSeen.x - mMe.m[3][0];
        const float fDZ = vLastSeen.z - mMe.m[3][2];
        const float fRange = CombatUtil::GetCombatRange(GetMe()) + 10.0f;
        if (fDZ * fDZ + fDX * fDX > fRange * fRange)
        {
            SetNextState(100);
        }

        AttackPatterns::State eState;
        float fTime;
        bool bFlag;
        if (!CombatUtil::SelectCombatResponse(GetMe(), GetAttackHelper().GetTarget(),
                                              &eState, &fTime, &bFlag, true, false))
        {
            SetNextState(STATE_LookAround);
            break;
        }
        if (bNewTarget || eState != mAttackPattern)
        {
            SetNextState(STATE_Engage);
            break;
        }

        mAttackPatternHelper.FSMUpdate(dt, false);
        const bool bOverrideFire = mAttackPatternHelper.HELPERGetOverrideFire();

        if (GetMe()->GetEntity()->IsRtti(kRttiHashEntityDroideka))
        {
            const Damageable* pDamageable = GetDamageable(GetMe()->GetGameObject());
            if (!IsShielded(pDamageable) &&
                GetCurShield(pDamageable) > GetMaxShield(pDamageable) * 0.5f)
            {
                GetLowLevel().SetNextWeapon(1);
                GetLowLevel().SetFireRequest(true, false);
                return;
            }
        }
        if (bOverrideFire)
        {
            return;
        }
        break;
    }

    case STATE_FleeEvent:
        if (GetMe()->GetEntity()->IsRtti(kRttiHashEntityHover) ||
            GetMe()->GetEntity()->IsRtti(kRttiHashEntityWalker))
        {
            GetLowLevel().SetHeight(HEIGHT_SPRINT);
        }
        if (GameLoop::GetMissionTime() > mFleeTime)
        {
            SetNextState(STATE_LookAround);
        }
        break;

    case STATE_ShootDeadBody:
        if (HandleObject(mTargetHandle) == 0)
        {
            SetNextState(STATE_Engage);
            break;
        }
        GetLowLevel().SetTarget(0);
        GetLowLevel().SetHeading(GetPosition(HandleObject(mTargetHandle)), 0.0f);
        GetLowLevel().SetNextWeapon(mDeadBodyWeaponIdx);
        GetLowLevel().SetFireRequest(true, false);
        return;
    }

    GetAttackHelper().Fire();
}

// FUNCTION: SWBF2 0x0046cd50
bool CombatHelper::DeadBodyCheck()
{
    if (GetCtrl()->mLodState < 4)
    {
        return false;
    }

    Character* pCharacter = GetMe()->mCharacter;
    Team* pTeam = pCharacter ? pCharacter->mTeamPtr : 0;
    if (!pTeam || pTeam->GetSide() != Team::side_Alliance)
    {
        return false;
    }
    if (GetMe()->GetGameObject()->GetGameObjectClass()->GetSoldierType() != 4 ||
        (PblRandom::GetGlobalInt() & 0x7f) == 2)
    {
        return false;
    }

    GameObject* pMe = GetMe()->GetGameObject();
    if (!pMe->IsRtti(kRttiHashEntitySoldier))
    {
        return false;
    }

    const PblVector3& vPos = GetPosition(pMe);
    GameObject* pObjects[5];
    int iCount = CollisionManager::GetObjectsInRange(vPos, 5.0f, pObjects, 5, pMe, 2);
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObject = pObjects[i];
        if (pObject->mCurHealth > 0.0f)
        {
            continue;
        }
        if (pTeam->GetAffiliation(pObject->GetTeam()) >= 0)
        {
            continue;
        }
        if (!pObject->IsRtti(kRttiHashEntitySoldier))
        {
            continue;
        }
        if (PathFinder::TestTrivialStraightLineHit(vPos, GetPosition(pObject), pMe) == true)
        {
            continue;
        }

        mTargetHandle = pObject;
        GetLowLevel().SetNavigator_Goto(GetPosition(pObject), 0, false, false);
        SetNextState(STATE_ShootDeadBody);
        mDeadBodyWeaponIdx = 0;
        for (unsigned char uWpn = 0; uWpn < GetMe()->GetWeaponCount(); ++uWpn)
        {
            if (CombatUtil::GetWeaponLabel(GetMe()->GetWeaponPtr(uWpn)) == WeaponClass::BLASTER)
            {
                mDeadBodyWeaponIdx = uWpn;
                break;
            }
        }
        AI::VOHelper::Play(AI::VOT_RebelsShootDeadBody, pMe, 0, pObject, 0.0f);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0046d790
int CombatHelper::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
        return mOwner->RunForVehicle(pInfo->mGameObject) ? AI::EVT_NotHandled : AI::EVT_Ignored;

    case AI::EVT_Grenade:
        if (GetMe()->GetEntity()->IsRtti(kRttiHashMountedTurret) || mCurState == STATE_FleeEvent)
        {
            return AI::EVT_Ignored;
        }
        mFleeSrc = pInfo->mPos;
        SetNextState(STATE_FleeEvent);
        return AI::EVT_Handled;

    case AI::EVT_Damaged:
    {
        AI::EventInfo_Damaged* pDamaged = static_cast<AI::EventInfo_Damaged*>(pInfo);
        mOwner->DamagedEventPlayerCentricness(pInfo);
        mOwner->PlayDamageEventVO(pInfo);

        if ((GetMe()->GetEntity()->IsRtti(kRttiHashEntityHover) ||
             GetMe()->GetEntity()->IsRtti(kRttiHashEntityWalker)) &&
            pDamaged->mIsCriticalHit && mCurState != STATE_FleeEvent)
        {
            PblVector3 vFrom = GetMe()->GetMatrix().GetTranslation();
            const PblMatrix& mMe = GetMe()->GetMatrix();
            PblVector3 vTo(vFrom.x + mMe.m[2][0] * 10.0f, vFrom.y + mMe.m[2][1] * 10.0f,
                           vFrom.z + mMe.m[2][2] * 10.0f);
            if (!PathFinder::TestTrivialStraightLineHit(vFrom, vTo, GetMe()->GetGameObject()))
            {
                SetNextState(STATE_FleeEvent);
            }
        }

        if (mCurState != STATE_Turn && mCurState != STATE_LookAround && mCurState != STATE_Investigate)
        {
            return AI::EVT_Ignored;
        }
        if (pDamaged->mDamageOwner && pDamaged->mDamageOwner->GetControllable())
        {
            mTargetPos = pDamaged->mDamageOwner->GetControllable()->GetMatrix().GetTranslation();
            SetNextState(STATE_Turn);
        }
        return AI::EVT_Handled;
    }

    case AI::EVT_Vision_CanSeeEnemy:
        if (mCurState == STATE_Engage)
        {
            return AI::EVT_Ignored;
        }
        if (!CombatUtil::EngageInCombat(GetCtrl(), pInfo->mGameObject, false))
        {
            return AI::EVT_Ignored;
        }
        SetNextState(STATE_Engage);
        return AI::EVT_Handled;

    case AI::EVT_Audible_CanHearEnemy:
        if (AIUtil::IsVehicle(GetMe()->GetGameObject()))
        {
            return AI::EVT_Ignored;
        }
        if (mCurState != STATE_LookAround)
        {
            return AI::EVT_Ignored;
        }
        if (!CombatUtil::EngageInCombat(GetCtrl(), pInfo->mGameObject, false))
        {
            return AI::EVT_Ignored;
        }
        reinterpret_cast<AI::UnitThreatManager*>(GetCtrl()->mThreatManager)->AddThreat(pInfo->mGameObject);
        mTargetHandle = pInfo->mGameObject;
        mTargetPos = GetPosition(pInfo->mGameObject);
        SetNextState(STATE_Investigate);
        return AI::EVT_Handled;

    case AI::EVT_EndInvisibility:
        if (mCurState == STATE_Engage)
        {
            return AI::EVT_Ignored;
        }
        GetCtrl()->ForceSetAttackHelperTarget(pInfo->mGameObject);
        SetNextState(STATE_Engage);
        return AI::EVT_Handled;
    }
    return AI::EVT_NotHandled;
}
