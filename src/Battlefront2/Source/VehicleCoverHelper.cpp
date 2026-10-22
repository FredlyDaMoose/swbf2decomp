#include "VehicleCoverHelper.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "HintManager.h"
#include "UnitAgent.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

#include <float.h>
#include <math.h>
#include <stddef.h>

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

namespace CombatUtil
{
    float GetWeaponMaxRange(const Controllable* pMe);
}

namespace
{

    const PblHash kRttiHashEntityHover("EntityHover");

    const PblVector3& GetPos(const PblMatrix& m)
    {
        return *(const PblVector3*)&m.m[3][0];
    }

    AILowLevel& GetLowLevel(UnitController* pCtrl)
    {
        return *reinterpret_cast<AILowLevel*>(pCtrl->mLowLevel);
    }

    AttackHelper* GetAttackHelper(UnitAgent* pOwner)
    {
        return reinterpret_cast<AttackHelper*>(pOwner->mAttackHelper);
    }

    CollisionObject* GetCollisionObject(GameObject* pObj)
    {
        return pObj->GetCollisionObject();
    }

    const CollisionObject* GetCollisionObject(const GameObject* pObj)
    {
        return pObj->GetCollisionObject();
    }
}

// FUNCTION: SWBF2 0x0066bd20
VehicleCoverHelper::VehicleCoverHelper(UnitAgent* pOwner)
    : HelperFsm(STATE_START, STATE_START)
{
    mOwner = pOwner;
    mVehicleCoverTarget.mObject = NULL;
    mVehicleCoverTarget.mSavedHandleId = 0;
    mHint = NULL;
}

// FUNCTION: SWBF2 0x0066bd60
bool VehicleCoverHelper::CheckForHintAndInit(const GameObject* pTarget)
{
    if (!GetMe()->GetEntity()->IsRtti(kRttiHashEntityHover))
    {
        return false;
    }

    mVehicleCoverTarget.mObject = pTarget;
    mVehicleCoverTarget.mSavedHandleId = pTarget ? pTarget->mHandleId : 0;
    mHint = const_cast<VehicleCoverHint*>(FindHint());
    if (!mHint)
    {
        return false;
    }

    mHint->Reserve(GetMe()->GetGameObject());
    mNextHintCheck = GameLoop::GetMissionTime() + 5.0f;
    mNewHintCheck = (float)AIUtil::GetRandomInt(10, 20) + GameLoop::GetMissionTime();
    SetNextState(STATE_GOTO);
    FSMChangeState();
    return true;
}

// FUNCTION: SWBF2 0x0066be20
void VehicleCoverHelper::EnterState()
{
    switch (mCurState)
    {
    case STATE_GOTO:
    {

        mHint->Reserve(GetMe()->GetGameObject());
        PblVector3 vMe = GetPos(GetMe()->GetMatrix());
        const PblVector3& vHint = mHint->GetCollisionCenter();
        PblVector3 vDir(vHint.x - vMe.x, vHint.y - vMe.y, vHint.z - vMe.z);
        float fDist = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
        PblVector3 vNorm;
        D3DXVec3Normalize(&vNorm, &vDir);
        fDist += 4.0f;
        PblVector3 vDest(vNorm.x * fDist + vMe.x, vNorm.y * fDist + vMe.y, vNorm.z * fDist + vMe.z);
        GetLowLevel(GetCtrl()).SetNavigator_Goto(vDest, NULL, false, false);
        GetLowLevel(GetCtrl()).SetHeight(HEIGHT_WALK);
        GetLowLevel(GetCtrl()).mAllowHoverStrafe = true;
        GetAttackHelper(mOwner)->AllowEnemyUnseen(true);
        break;
    }
    case STATE_COVERED:
    {
        mTimer = (float)AIUtil::GetRandomInt(2, 4) + GameLoop::GetMissionTime();
        GetAttackHelper(mOwner)->AllowEnemyUnseen(true);
        mOwner->GetLowLevel().SetNavigator_Stationary();
        if (!GetAttackHelper(mOwner)->GetVisibleTarget())
        {

            const Character* pPlayer = mOwner->mCtrl->mLodHumanPlayer;
            if (pPlayer && pPlayer->GetControllable() && pPlayer->GetControllable()->GetGameObject() &&
                (PblRandom::GetGlobalInt() & 3) == 1)
            {
                mOwner->GetLowLevel().HeadLook_Object(pPlayer->GetControllable()->GetGameObject(),
                                                      0.2f, 1.0f, 0.3f);
            }
        }
        break;
    }
    case STATE_POP_UP:
        mTimer = (float)AIUtil::GetRandomInt(2, 6) + GameLoop::GetMissionTime();
        mOwner->GetLowLevel().SetAlert();
        break;
    case STATE_STEP_OUT:
    {

        mTimer = (float)AIUtil::GetRandomInt(4, 7) + GameLoop::GetMissionTime();
        const PblVector3& vZ = mHint->GetZAxis();
        PblVector3 vSide(vZ.y * gVectorUnitY.z - vZ.z * gVectorUnitY.y,
                         vZ.z * gVectorUnitY.x - vZ.x * gVectorUnitY.z,
                         vZ.x * gVectorUnitY.y - vZ.y * gVectorUnitY.x);
        int iSide = mHint->GetRandomSidestep() == BaseHint::STEP_LEFT ? -1 : 1;
        float fRadius = GetCollisionObject(GetMe()->GetGameObject())->GetCollisionRadius();
        float fDist = AIUtil::GetRandomFloat(fRadius * 2.0f, fRadius * 3.0f) * (float)iSide;
        const PblVector3& vHint = mHint->GetCollisionCenter();
        PblVector3 vDest(vHint.x + vSide.x * fDist, vHint.y + vSide.y * fDist, vHint.z + vSide.z * fDist);
        GetLowLevel(GetCtrl()).SetNavigator_Goto(vDest, NULL, false, false);
        GetLowLevel(GetCtrl()).mAllowHoverStrafe = true;
        GetLowLevel(GetCtrl()).SetHeight(HEIGHT_WALK);
        mOwner->GetLowLevel().SetAlert();
        GetAttackHelper(mOwner)->SetWaitDelay(10.0f);
        break;
    }
    }
}

// FUNCTION: SWBF2 0x0066c260
void VehicleCoverHelper::ExitState()
{
    AttackHelper* pAttack = reinterpret_cast<AttackHelper*>(mOwner->mAttackHelper);
    switch (mCurState)
    {
    case STATE_GOTO:
    case STATE_COVERED:
        pAttack->AllowEnemyUnseen(false);
        break;
    case STATE_STEP_OUT:
        pAttack->ResetWaitDelay();
        break;
    }
}

// FUNCTION: SWBF2 0x0066c290
void VehicleCoverHelper::UpdateState(float dt)
{
    if (mCurState == 100)
    {
        return;
    }

    if (GameLoop::GetMissionTime() > mNextHintCheck)
    {
        mNextHintCheck = GameLoop::GetMissionTime() + 5.0f;
        if (IsHintSuitable(mHint) == FLT_MAX)
        {
            mHint->Unreserve(GetMe()->GetGameObject());
            mHint = const_cast<VehicleCoverHint*>(FindHint());
            if (mHint)
            {
                mHint->Reserve(GetMe()->GetGameObject());
                SetNextState(STATE_GOTO);
            }
            else
            {
                SetNextState(100);
            }
            return;
        }
    }

    if (GameLoop::GetMissionTime() > mNewHintCheck)
    {
        mNewHintCheck = (float)AIUtil::GetRandomInt(10, 20) + GameLoop::GetMissionTime();
        const VehicleCoverHint* pHint = FindHint();
        if (pHint)
        {
            mHint = const_cast<VehicleCoverHint*>(pHint);
            mHint->Reserve(GetMe()->GetGameObject());
            SetNextState(STATE_GOTO);
            return;
        }
    }

    switch (mCurState)
    {
    case STATE_GOTO:
        if (GetLowLevel(GetCtrl()).IsAtDest())
        {
            SetNextState(STATE_COVERED);
            break;
        }
        GetMe()->GetGameObject();
        mHint->Reserve(GetMe()->GetGameObject());
        GetLowLevel(GetCtrl()).mAllowHoverStrafe = true;
        break;
    case STATE_COVERED:
        GetMe()->GetGameObject();
        mHint->Reserve(GetMe()->GetGameObject());
        if (GameLoop::GetMissionTime() > mTimer)
        {
            SetNextState(mHint->HasSidestep() ? STATE_STEP_OUT : STATE_POP_UP);
        }
        break;
    case STATE_POP_UP:
        GetMe()->GetGameObject();
        mHint->Reserve(GetMe()->GetGameObject());
        if (GameLoop::GetMissionTime() > mTimer - 2.0f && !GetAttackHelper(mOwner)->GetVisibleTarget() &&
            (PblRandom::GetGlobalInt() & 3) == 1)
        {
            bool bRight;
            if (mHint->HasSidestep())
            {
                bRight = mHint->GetRandomSidestep() == BaseHint::STEP_RIGHT;
            }
            else
            {
                bRight = (PblRandom::GetGlobalInt() & 1) != 0;
            }
            mOwner->GetLowLevel().HeadLook_Glance(bRight, 0.2f, PblRandom::GetGlobalFloat(), 0.3f);
        }
        if (GameLoop::GetMissionTime() > mTimer)
        {
            SetNextState(STATE_COVERED);
        }
        break;
    case STATE_STEP_OUT:
        GetMe()->GetGameObject();
        mHint->Reserve(GetMe()->GetGameObject());
        GetLowLevel(GetCtrl()).mAllowHoverStrafe = true;
        if (GetLowLevel(GetCtrl()).IsAtDest())
        {
            GetLowLevel(GetCtrl()).SetNavigator_Stationary();
        }
        if (GameLoop::GetMissionTime() > mTimer)
        {
            SetNextState(STATE_GOTO);
        }
        break;
    }

    if (HELPERFaceTarget())
    {
        if (GetAttackHelper(mOwner)->GetVisibleTarget())
        {
            mOwner->GetLowLevel().SetHeading(GetAttackHelper(mOwner)->GetVisibleTarget());
        }
        else
        {

            Controllable* pMe = GetMe();
            const PblVector3& vZ = mHint->GetZAxis();
            PblVector3 vAhead(vZ.x * 30.0f, vZ.y * 30.0f, vZ.z * 30.0f);
            const PblVector3& vPos = GetPos(pMe->GetMatrix());
            PblVector3 vLook(vAhead.x + vPos.x, vPos.y + vAhead.y, vPos.z + vAhead.z);
            mOwner->GetLowLevel().SetHeading(vLook, 0.0f);
        }
    }
    else
    {
        mOwner->GetLowLevel().SetHeadingDefault();
    }
}

// FUNCTION: SWBF2 0x0066be10
void VehicleCoverHelper::HELPERClean()
{
    ExitState();
}

// FUNCTION: SWBF2 0x0066c690
bool VehicleCoverHelper::HELPERFaceTarget() const
{
    if (mCurState != STATE_GOTO)
    {
        return true;
    }

    PblVector3 vMe = GetPos(GetMe()->GetMatrix());
    PblVector3 vHint = mHint->GetCollisionCenter();
    float fRadius = GetCollisionObject(GetMe()->GetGameObject())->GetCollisionRadius() + 1.0f;
    float dx = vHint.x - vMe.x;
    float dz = vHint.z - vMe.z;
    return dx * dx + dz * dz <= fRadius * fRadius;
}

// FUNCTION: SWBF2 0x0066c760
const Controllable* VehicleCoverHelper::GetMe() const
{
    return mOwner->GetMe();
}

// FUNCTION: SWBF2 0x0066c770
Controllable* VehicleCoverHelper::GetMe()
{
    return mOwner->GetMe();
}

// FUNCTION: SWBF2 0x0066c780
UnitController* VehicleCoverHelper::GetCtrl()
{
    return mOwner->mCtrl;
}

// FUNCTION: SWBF2 0x0066c790
const UnitController* VehicleCoverHelper::GetCtrl() const
{
    return mOwner->mCtrl;
}

// FUNCTION: SWBF2 0x0066c7a0
float VehicleCoverHelper::GetMaxHintDist() const
{
    if (!GetMe()->GetCurWpn())
    {
        return 100.0f;
    }
    float fRange = CombatUtil::GetWeaponMaxRange(GetMe());
    return fRange > 100.0f ? 100.0f : fRange;
}

// FUNCTION: SWBF2 0x0066c7e0
const VehicleCoverHint* VehicleCoverHelper::FindHint() const
{
    float fMaxDist = GetMaxHintDist();
    const BaseHint* pHints[30];
    int iCount = HintManager::RequestVehicleCoverHints(GetCtrl(), GetPos(GetMe()->GetMatrix()),
                                                       (int)fMaxDist, 30, pHints);
    return SelectHint(iCount, pHints);
}

// FUNCTION: SWBF2 0x0066c840
const VehicleCoverHint* VehicleCoverHelper::SelectHint(int iCount, const BaseHint** ppHints) const
{
    const GameObject* pMe = GetMe()->GetGameObject();
    float fBest = FLT_MAX;
    const VehicleCoverHint* pBest = NULL;
    while (iCount > 0)
    {
        --iCount;
        int i = AIUtil::GetRandomInt(0, iCount);
        const VehicleCoverHint* pHint = static_cast<const VehicleCoverHint*>(ppHints[i]);
        if (AIUtil::IsHintAvailable(pMe, pHint))
        {
            float fScore = IsHintSuitable(pHint);
            if (fScore < fBest)
            {
                fBest = fScore;
                pBest = pHint;
            }
        }
        ppHints[i] = ppHints[iCount];
    }
    return pBest;
}

// FUNCTION: SWBF2 0x0066c8e0
float VehicleCoverHelper::IsHintSuitable_Combat(const VehicleCoverHint* pHint) const
{
    PblVector3 vMe = GetPos(GetMe()->GetMatrix());
    PblVector3 vTarget = GetAttackHelper(mOwner)->GetLastSeenTargetPos();
    PblVector3 vMeToTarget(vTarget.x - vMe.x, 0.0f, vTarget.z - vMe.z);
    PblVector3 vHint = pHint->GetCollisionCenter();
    float fCosMax = PblMath::Cos(1.0471976f);
    PblVector3 vTargetToHint(vHint.x - vTarget.x, vHint.y - vTarget.y, vHint.z - vTarget.z);

    if (vTargetToHint.x * vMeToTarget.x + vTargetToHint.y * vMeToTarget.y + vTargetToHint.z * vMeToTarget.z >= 0.0f)
    {
        return FLT_MAX;
    }

    PblVector3 vHintToTarget(vTarget.x - vHint.x, vTarget.y - vHint.y, vTarget.z - vHint.z);
    PblVector3 vDir = AIUtil::GetSafeNormalXZ(vHintToTarget);
    const PblVector3& vZ = pHint->GetZAxis();
    if (vZ.y * vDir.y + vZ.x * vDir.x + vZ.z * vDir.z <= fCosMax)
    {
        return FLT_MAX;
    }

    float fMaxDist = GetMaxHintDist();
    float fDistSq = vTargetToHint.z * vTargetToHint.z + vTargetToHint.x * vTargetToHint.x;
    if (fMaxDist * fMaxDist <= fDistSq || fDistSq <= 0.0f)
    {
        return FLT_MAX;
    }

    float dx = vHint.x - vMe.x;
    float dz = vHint.z - vMe.z;
    return dz * dz * 2.25f + dx * dx * 2.25f + fDistSq;
}

// FUNCTION: SWBF2 0x0066cab0
float VehicleCoverHelper::IsHintSuitable(const VehicleCoverHint* pHint) const
{
    return IsHintSuitable_Combat(pHint);
}
