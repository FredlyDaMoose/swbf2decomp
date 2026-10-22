#include "CoverHelper.h"

#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "CombatUtil.h"
#include "Controllable.h"
#include "Entity.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "HintManager.h"
#include "PathFinder.h"
#include "UnitAgent.h"
#include "UnitAttackPatterns.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

#include <float.h>
#include <d3dx9math.h>

namespace
{
    const PblVector3& GetPos(const PblMatrix& m)
    {
        return *(const PblVector3*)&m.m[3][0];
    }

    AttackHelper* GetAttackHelper(UnitAgent* pOwner)
    {
        return reinterpret_cast<AttackHelper*>(pOwner->mAttackHelper);
    }
}

class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

// FUNCTION: SWBF2 0x00486ac0
CoverHelper::CoverHelper(UnitAgent* pOwner)
    : HelperFsm(STATE_START, STATE_START)
    , mOwner(pOwner)
{
    mCoverTarget.mObject = NULL;
    mCoverTarget.mSavedHandleId = 0;
    mHint = NULL;
}

// SYNTHETIC: SWBF2 0x00486a90
// CoverHelper::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00486bf0
void CoverHelper::HELPERClean()
{
    ExitState();
}

// FUNCTION: SWBF2 0x00486fb0
void CoverHelper::ExitState()
{
    AttackHelper* pAttack = reinterpret_cast<AttackHelper*>(mOwner->mAttackHelper);
    mStance = HEIGHT_STAND;
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

// FUNCTION: SWBF2 0x00486b00
bool CoverHelper::CheckForHintAndInit(CoverType eType, const GameObject* pTarget)
{
    if (!GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return false;
    }

    mCoverType = eType;
    mCoverTarget.mObject = pTarget;
    mCoverTarget.mSavedHandleId = pTarget ? pTarget->mHandleId : 0;
    mHint = const_cast<CoverHint*>(FindHint());
    if (!mHint)
    {
        return false;
    }
    return InitAtHint(eType, mHint);
}

// FUNCTION: SWBF2 0x00486b70
bool CoverHelper::InitAtHint(CoverType eType, const BaseHint* pHint)
{
    mHint = const_cast<BaseHint*>(pHint);
    mCoverType = eType;
    mHint->Reserve(GetMe()->GetGameObject());
    mStance = HEIGHT_STAND;
    mNextHintCheck = GameLoop::GetMissionTime() + 5.0f;
    mNewHintCheck = (float)AIUtil::GetRandomInt(5, 12) + GameLoop::GetMissionTime();
    SetNextState(STATE_GOTO);
    FSMChangeState();
    return true;
}

// FUNCTION: SWBF2 0x00487570
const Controllable* CoverHelper::GetMe() const
{
    return mOwner->GetMe();
}

// FUNCTION: SWBF2 0x00487580
Controllable* CoverHelper::GetMe()
{
    return mOwner->GetMe();
}

// FUNCTION: SWBF2 0x00487590
UnitController* CoverHelper::GetCtrl()
{
    return mOwner->mCtrl;
}

// FUNCTION: SWBF2 0x004875a0
const UnitController* CoverHelper::GetCtrl() const
{
    return mOwner->mCtrl;
}

// FUNCTION: SWBF2 0x00486c00
void CoverHelper::EnterState()
{
    switch (mCurState)
    {
    case STATE_GOTO:
        mHint->Reserve(GetMe()->GetGameObject());
        GetCtrl()->GetLowLevel().SetNavigator_Goto(mHint->GetCollisionCenter(), NULL, false,
                                                   (PblRandom::GetGlobalInt() & 1) != 0);
        mStance = HEIGHT_STAND;
        GetAttackHelper(mOwner)->AllowEnemyUnseen(true);
        break;
    case STATE_COVERED:
    {

        if (mCoverType == COVERTYPE_Idle && AIUtil::GetRandomInt(0, 100) > 75)
        {
            SetNextState(100);
            break;
        }
        mTimer = (float)AIUtil::GetRandomInt(2, 4) + GameLoop::GetMissionTime();
        mStance = mHint->GetRandomPrimaryStance();
        GetAttackHelper(mOwner)->AllowEnemyUnseen(true);
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
        mStance = mHint->GetRandomSecondaryStance();
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
        float fDist = (float)iSide * 2.0f;
        const PblVector3& vHint = mHint->GetCollisionCenter();
        PblVector3 vDest(vHint.x + vSide.x * fDist, vHint.y + vSide.y * fDist, vHint.z + vSide.z * fDist);
        GetCtrl()->GetLowLevel().SetNavigator_Goto(vDest, NULL, false, false);
        mStance = mHint->GetRandomSecondaryStance();
        mOwner->GetLowLevel().SetAlert();
        GetAttackHelper(mOwner)->SetWaitDelay(4.0f);
        break;
    }
    case 100:
        if (mHint)
        {
            mHint->Unreserve(GetMe()->GetGameObject());
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00486ff0
void CoverHelper::UpdateState(float dt)
{
    if (mCurState == 100)
    {
        return;
    }

    if (mCoverType != COVERTYPE_Idle && GameLoop::GetMissionTime() > mNextHintCheck)
    {
        mNextHintCheck = GameLoop::GetMissionTime() + 5.0f;
        if (IsHintSuitable(mHint) == FLT_MAX)
        {
            mHint->Unreserve(GetMe()->GetGameObject());
            mHint = const_cast<CoverHint*>(FindHint());
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

    if (mCoverType != COVERTYPE_Idle && GameLoop::GetMissionTime() > mNewHintCheck)
    {
        mNewHintCheck = (float)AIUtil::GetRandomInt(5, 12) + GameLoop::GetMissionTime();
        const CoverHint* pHint = FindHint();
        if (pHint)
        {
            mHint = const_cast<CoverHint*>(pHint);
            mHint->Reserve(GetMe()->GetGameObject());
            SetNextState(STATE_GOTO);
            return;
        }
    }

    switch (mCurState)
    {
    case STATE_GOTO:
        if (GetCtrl()->GetLowLevel().IsAtDest())
        {
            SetNextState(STATE_COVERED);
        }
        else
        {
            GetMe()->GetGameObject();
            mHint->Reserve(GetMe()->GetGameObject());
        }

        if (mCoverType == COVERTYPE_ForwardCover)
        {
            const PblVector3& vMe = GetPos(GetMe()->GetMatrix());
            const PblVector3& vHint = mHint->GetCollisionCenter();
            float dx = vMe.x - vHint.x;
            float dy = vMe.y - vHint.y;
            float dz = vMe.z - vHint.z;
            if (dy * dy + dx * dx + dz * dz > 25.0f)
            {
                mStance = HEIGHT_SPRINT;
                break;
            }
        }
        mStance = HEIGHT_STAND;
        break;
    case STATE_COVERED:
        GetMe()->GetGameObject();
        mHint->Reserve(GetMe()->GetGameObject());
        if (GameLoop::GetMissionTime() > mTimer)
        {
            if (mHint->HasSidestep() && (PblRandom::GetGlobalInt() & 1))
            {
                SetNextState(STATE_STEP_OUT);
            }
            else
            {
                SetNextState(STATE_POP_UP);
            }
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
        if (GameLoop::GetMissionTime() > mTimer - 2.0f && !GetAttackHelper(mOwner)->GetVisibleTarget() &&
            (PblRandom::GetGlobalInt() & 3) == 1)
        {

            mOwner->GetLowLevel().HeadLook_Glance(mHint->GetRandomSidestep() == BaseHint::STEP_LEFT, 0.2f,
                                                  PblRandom::GetGlobalFloat() * 0.5f, 0.3f);
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
            PblVector3 vLook(vPos.x + vAhead.x, vPos.y + vAhead.y, vPos.z + vAhead.z);
            mOwner->GetLowLevel().SetHeading(vLook, 0.0f);
        }
    }
    else
    {
        mOwner->GetLowLevel().SetHeadingDefault();
    }

    if (mStance == HEIGHT_PRONE)
    {
        mStance = HEIGHT_STAND;
    }
    mOwner->GetLowLevel().SetHeight(mStance);
}

// FUNCTION: SWBF2 0x004874d0
ControllableHeight CoverHelper::HELPERGetHeight() const
{
    return mStance;
}

// FUNCTION: SWBF2 0x004874e0
bool CoverHelper::HELPERFaceTarget() const
{
    if (mCurState != STATE_GOTO)
    {
        return true;
    }

    PblVector3 vMe = GetPos(GetMe()->GetMatrix());
    PblVector3 vHint = mHint->GetCollisionCenter();
    float dx = vHint.x - vMe.x;
    float dz = vHint.z - vMe.z;
    return dx * dx + dz * dz <= 9.0f;
}

// FUNCTION: SWBF2 0x004875b0
float CoverHelper::GetMaxHintDist() const
{
    if (!GetMe()->GetCurWpn())
    {
        return 40.0f;
    }
    float fRange = CombatUtil::GetWeaponMaxRange(GetMe());
    return fRange > 40.0f ? 40.0f : fRange;
}

// FUNCTION: SWBF2 0x004875f0
const CoverHint* CoverHelper::FindHint() const
{
    float fMaxDist = GetMaxHintDist();
    const BaseHint* pHints[30];
    int iCount = HintManager::RequestCoverHints(GetCtrl(), GetPos(GetMe()->GetMatrix()), (int)fMaxDist, 30, pHints);
    return SelectHint(iCount, pHints);
}

// FUNCTION: SWBF2 0x00487650
const CoverHint* CoverHelper::SelectHint(int iCount, const BaseHint** ppHints) const
{
    const GameObject* pMe = GetMe()->GetGameObject();
    float fBest = FLT_MAX;
    const CoverHint* pBest = NULL;
    while (iCount > 0)
    {
        --iCount;
        int i = AIUtil::GetRandomInt(0, iCount);
        const CoverHint* pHint = static_cast<const CoverHint*>(ppHints[i]);
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

// FUNCTION: SWBF2 0x004876f0
float CoverHelper::IsHintSuitable_Combat(const BaseHint* pHint) const
{
    PblVector3 vMe = GetPos(GetMe()->GetMatrix());
    PblVector3 vTarget = reinterpret_cast<const AttackHelper*>(mOwner->mAttackHelper)->GetLastSeenTargetPos();
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
    if (fMaxDist * fMaxDist <= fDistSq)
    {
        return FLT_MAX;
    }
    float fNear = AttackPatterns::GetNear(GetMe());
    if (fDistSq <= fNear * fNear)
    {
        return FLT_MAX;
    }

    float dx = vHint.x - vMe.x;
    float dz = vHint.z - vMe.z;
    return dz * dz * 2.25f + dx * dx * 2.25f + fDistSq;
}

// FUNCTION: SWBF2 0x00487bd0
float CoverHelper::IsHintSuitable(const BaseHint* pHint) const
{
    switch (mCoverType)
    {
    case COVERTYPE_Combat:
        return IsHintSuitable_Combat(pHint);
    case COVERTYPE_ForwardCover:
        return IsHintSuitable_ForwardCover(pHint);
    case COVERTYPE_Idle:
        return IsHintSuitable_Idle(pHint);
    }
    return FLT_MAX;
}

// FUNCTION: SWBF2 0x004878e0
float CoverHelper::IsHintSuitable_ForwardCover(const BaseHint* pHint) const
{
    const PblVector3& vHint = pHint->GetCollisionCenter();
    const GameObject* pTarget = mCoverTarget.mObject;
    if (pTarget && pTarget->mHandleId != mCoverTarget.mSavedHandleId)
    {
        CoverHelper* pMutable = const_cast<CoverHelper*>(this);
        pMutable->mCoverTarget.mObject = NULL;
        pMutable->mCoverTarget.mSavedHandleId = 0;
        pTarget = NULL;
    }
    const PblVector3& vTarget = GetPos(pTarget->GetMatrix());
    float dx = vHint.x - vTarget.x;
    float dy = vHint.y - vTarget.y;
    float dz = vHint.z - vTarget.z;
    float fDistSq = dx * dx + dy * dy + dz * dz;
    if (fDistSq < 25.0f || fDistSq > 900.0f)
    {
        return FLT_MAX;
    }

    const PblMatrix& mTarget = pTarget->GetMatrix();
    const PblVector3& vMe = GetPos(GetMe()->GetMatrix());
    PblVector3 vTargetToMe(vTarget.x - vMe.x, vTarget.y - vMe.y, vTarget.z - vMe.z);
    PblVector3 vTargetToHint(vTarget.x - vHint.x, vTarget.y - vHint.y, vTarget.z - vHint.z);
    const PblVector3& vForward = *(const PblVector3*)&mTarget.m[0][0];
    float fMeSide = vForward.x * vTargetToMe.x + vForward.y * vTargetToMe.y + vForward.z * vTargetToMe.z;
    float fHintSide = vForward.x * vTargetToHint.x + vForward.y * vTargetToHint.y + vForward.z * vTargetToHint.z;
    if ((fMeSide < 0.0f) != (fHintSide < 0.0f))
    {
        return FLT_MAX;
    }

    PblVector3 vNormal;
    PblVector3 vHintFromTarget(vHint.x - vTarget.x, vHint.y - vTarget.y, vHint.z - vTarget.z);
    D3DXVec3Normalize((D3DXVECTOR3*)&vNormal, (const D3DXVECTOR3*)&vHintFromTarget);
    const PblVector3& vSide = *(const PblVector3*)&mTarget.m[2][0];
    float fDot = vSide.x * vNormal.x + vSide.y * vNormal.y + vSide.z * vNormal.z;
    if (fDot > 0.99619401f || fDot < 0.93969202f)
    {
        return FLT_MAX;
    }
    if (PathFinder::TestTrivialStraightLineHit(vMe, vHint, GetMe()->GetGameObject()))
    {
        return FLT_MAX;
    }
    float fScore = 1.0f - fDot;
    return fScore * fDistSq * fScore;
}

// FUNCTION: SWBF2 0x00487b70
float CoverHelper::IsHintSuitable_Idle(const BaseHint* pHint) const
{
    const PblVector3& vHint = pHint->GetCollisionCenter();
    const PblVector3& vMe = GetPos(GetMe()->GetMatrix());
    float dx = vHint.x - vMe.x;
    float dy = vHint.y - vMe.y;
    float dz = vHint.z - vMe.z;
    return dx * dx + dy * dy + dz * dz;
}
