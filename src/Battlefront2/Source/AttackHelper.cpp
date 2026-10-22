#include <float.h>

#include "AttackHelper.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "Aimer.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "UnitAgent.h"
#include "UnitAttackPatterns.h"
#include "UnitController.h"

#include "GameObject.h"

#include "UnitThreatManager.h"

extern PblVector3 gVectorZero;

namespace AI
{

enum VOType
{
    VOT_AcquiredTarget = 1
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget, const GameObject* pOther, float fDelay);
}

}

namespace AIUtil
{

    PblVector3 GetHisVisionTargetPos(const Controllable* pMe, const GameObject* pObj);

    bool CanSeePosition(Controllable* pMe, const PblVector3& vPos, const GameObject* pObj);
}

namespace CombatUtil
{

    bool SelectCombatResponse(Controllable* pMe, const GameObject* pTarget,
                              AttackPatterns::State* pState, float* pTime,
                              bool* pbFlag, bool bA, bool bB);
}

namespace VisionManager
{

    int GetVisualPriority(const Controllable* pMe, const GameObject* pObj);
}

namespace CombatUtil { bool TestWpnConstraintsPos(Controllable* pMe, const PblVector3& vAim, const PblVector3& vPos); }

namespace
{
    unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }

    AILowLevel& LowLevelOf(Controller* pCtrl)
    {
        return *reinterpret_cast<AILowLevel*>(UnitControllerPtr(pCtrl)->mLowLevel);
    }
}

// FUNCTION: SWBF2 0x00447a80
AttackHelper::AttackHelper(UnitAgent* pAgent)
    : mAgent(pAgent)
{
    Reset();
}

// FUNCTION: SWBF2 0x00447ab0
void AttackHelper::Reset()
{
    mTargetHandle.mObject = 0;
    mTargetHandle.mSavedHandleId = 0;
    mWaitingForTarget = false;
    mTargetVisible = false;
    mLastTimeSeen = -1000.0f;

    mCurPriority = (float)60000;
    ResetWaitDelay();
    mAllowEnemyUnseen = false;
}

// FUNCTION: SWBF2 0x00447af0
void AttackHelper::SetTarget(GameObject const* pTarget)
{
    mTargetHandle.mObject = pTarget;
    mTargetHandle.mSavedHandleId = pTarget ? GameObjectHandleId(pTarget) : 0;
    if (pTarget)
    {
        AI::VOHelper::Play(AI::VOT_AcquiredTarget, GetMe()->GetGameObject(), 0, pTarget, 0.0f);
    }
}

// FUNCTION: SWBF2 0x00447b40
void AttackHelper::SetTargetInfo(GameObject const* pTarget, PblVector3 const& vPos)
{
    mTargetVisible = true;
    mLastKnownTargetPos = vPos;
    mLastTimeSeen = GameLoop::GetMissionTime();
    mCurPriority = (float)VisionManager::GetVisualPriority(GetMe(), pTarget);
    mWaitingForTarget = false;
}

// FUNCTION: SWBF2 0x00447b90
bool AttackHelper::UpdateTarget()
{
    const AI::UnitThreatManager& threats =
        *reinterpret_cast<const AI::UnitThreatManager*>(mAgent->mCtrl->mThreatManager);
    const GameObject* pVisible = threats.GetVisibleObject();
    const GameObject* pOldTarget = GetTarget();
    const GameObject* pTarget = pOldTarget;
    if (pTarget && !GetMe()->GetGameObject()->IsMyEnemy(pTarget))
    {
        SetTarget(pVisible);
        pTarget = GetTarget();
    }
    if (!pTarget || pTarget != pVisible)
    {
        SetTarget(pVisible);
        pTarget = GetTarget();
    }
    if (pTarget && pTarget == pVisible)
    {
        SetTargetInfo(pTarget, threats.GetVisibleObjectPos());
    }
    else
    {
        mTargetVisible = false;
    }
    return pOldTarget != GetTarget();
}

// FUNCTION: SWBF2 0x00447c50
void AttackHelper::Aim()
{
    const GameObject* pTarget = GetTarget();
    if (GetVisibleTarget())
    {
        LowLevelOf(GetMe()->mCtrl).SetTarget(pTarget);
        return;
    }
    LowLevelOf(GetMe()->mCtrl).SetTarget(0);
    if (pTarget)
    {
        LowLevelOf(GetMe()->mCtrl).SetAimPoint(mLastKnownTargetPos);
    }
}

// FUNCTION: SWBF2 0x00447cd0
void AttackHelper::Fire()
{
    bool bImmediate = false;
    bool bUponAim = false;
    const GameObject* pTarget = GetTarget();
    if (pTarget)
    {
        if (mTargetVisible)
        {
            bImmediate = false;
            bUponAim = true;
        }
        else
        {
            const Aimer* pAimer = GetMe()->GetCurAimer();
            PblVector3 vLead = mLastKnownTargetPos;
            if (pAimer)
            {
                pAimer->GetLeadPos(mLastKnownTargetPos, gVectorZero, &vLead);
            }
            if (CombatUtil::TestWpnConstraintsPos(GetMe(), vLead, mLastKnownTargetPos) && TargetIsHiding() &&
                AIUtil::CanSeePosition(GetMe(), mLastKnownTargetPos, pTarget))
            {
                bImmediate = true;
            }
        }
    }
    LowLevelOf(GetMe()->mCtrl).SetFireRequest(bImmediate, bUponAim);
}

// FUNCTION: SWBF2 0x00447da0
bool AttackHelper::TargetIsHiding() const
{
    PblVector3 vPos = AIUtil::GetHisVisionTargetPos(GetMe(), GetTarget());
    float fX = mLastKnownTargetPos.x - vPos.x;
    float fY = mLastKnownTargetPos.y - vPos.y;
    float fZ = mLastKnownTargetPos.z - vPos.z;
    return 9.0f > fX * fX + fY * fY + fZ * fZ;
}

// FUNCTION: SWBF2 0x00447e10
void AttackHelper::SelectBestWeapon()
{
    if (!GetMe()->GetWeaponPtr(0))
    {
        return;
    }
    const GameObject* pTarget = GetTarget();
    if (!pTarget)
    {
        return;
    }
    AttackPatterns::State eState;
    float fTime;
    bool bFlag;
    CombatUtil::SelectCombatResponse(GetMe(), pTarget, &eState, &fTime, &bFlag, true, false);
}

// FUNCTION: SWBF2 0x00447e60
void AttackHelper::Update()
{
    UpdateTarget();
    SelectBestWeapon();
    Aim();
    Fire();
}

// FUNCTION: SWBF2 0x00447e80
Controllable const* AttackHelper::GetMe() const
{
    return mAgent->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x00447e90
Controllable* AttackHelper::GetMe()
{
    return mAgent->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x00447ea0
bool AttackHelper::HasTarget() const
{
    return GetTarget() != 0;
}

// FUNCTION: SWBF2 0x00447eb0
GameObject const* AttackHelper::GetVisibleTarget() const
{
    return mTargetVisible ? GetTarget() : 0;
}

// FUNCTION: SWBF2 0x00447ec0
PblVector3 const& AttackHelper::GetLastSeenTargetPos() const
{
    return mLastKnownTargetPos;
}

// FUNCTION: SWBF2 0x00447ed0
float AttackHelper::GetLastTimeSeen() const
{
    return mLastTimeSeen;
}

// FUNCTION: SWBF2 0x00447ee0
GameObject const* AttackHelper::GetTarget() const
{
    return AIUtil::GetAlivePtr(mTargetHandle, false);
}

// FUNCTION: SWBF2 0x00447ef0
void AttackHelper::SetVisibleTarget(GameObject const* pTarget)
{
    Reset();
    mTargetHandle.mObject = pTarget;
    mTargetHandle.mSavedHandleId = pTarget ? GameObjectHandleId(pTarget) : 0;
    SetTargetInfo(pTarget, AIUtil::GetHisVisionTargetPos(GetMe(), pTarget));
}

// FUNCTION: SWBF2 0x00447f40
void AttackHelper::SetWaitDelay(float fDelay)
{
    mWaitDelay = fDelay;
    if (mWaitingForTarget)
    {
        ResetWaitOverTime();
    }
}

// FUNCTION: SWBF2 0x00447f50
void AttackHelper::ResetWaitDelay()
{
    mWaitDelay = 3.0f;
    if (mWaitingForTarget)
    {
        ResetWaitOverTime();
    }
}

// FUNCTION: SWBF2 0x00447f70
void AttackHelper::AllowEnemyUnseen(bool bAllow)
{
    mAllowEnemyUnseen = bAllow;
    if (mWaitingForTarget)
    {
        ResetWaitOverTime();
    }
}

// FUNCTION: SWBF2 0x00447f90
void AttackHelper::ResetWaitOverTime()
{
    if (mAllowEnemyUnseen)
    {
        mWaitOverTime = FLT_MAX;
    }
    else
    {
        mWaitOverTime = GameLoop::GetMissionTime() + mWaitDelay;
    }
}

// GLOBAL: SWBF2 0x007888d0

extern const unsigned char kBase64DecodeValue[128] = {
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127, 62,127,127,127, 63,
     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,127,127,127, 64,127,127,
    127,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,127,127,127,127,127,
    127, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,127,127,127,127,127
};

// FUNCTION: SWBF2 0x00448490
int __cdecl DecodeBase64(unsigned char* pOutput, unsigned int uiOutputCapacity, unsigned int* pOutputSize,
                       const unsigned char* pSource, unsigned int uiSourceSize)
{
    unsigned int uiPadding = 0;
    unsigned int uiValid = 0;
    unsigned int i = 0;
    while (i < uiSourceSize)
    {
        unsigned int uiSpaces = 0;
        while (i < uiSourceSize && pSource[i] == ' ')
        {
            ++i;
            ++uiSpaces;
        }
        if (i == uiSourceSize)
        {
            break;
        }

        const unsigned char c = pSource[i];
        if (c != '\n' && !(c == '\r' && i + 1 < uiSourceSize && pSource[i + 1] == '\n'))
        {
            if (uiSpaces)
            {
                return -44;
            }
            if (c == '=' && ++uiPadding > 2)
            {
                return -44;
            }
            if (c > 0x7f)
            {
                return -44;
            }
            const unsigned char digit = kBase64DecodeValue[c];
            if (digit == 127 || (digit < 64 && uiPadding))
            {
                return -44;
            }
            ++uiValid;
        }
        ++i;
    }

    if (!uiValid)
    {
        *pOutputSize = 0;
        return 0;
    }

    const unsigned int uiNeeded = ((6 * uiValid + 7) >> 3) - uiPadding;
    if (!pOutput || uiNeeded > uiOutputCapacity)
    {
        *pOutputSize = uiNeeded;
        return -42;
    }

    unsigned char* pWrite = pOutput;
    unsigned int quartet = 0;
    unsigned int nQuartet = 0;
    unsigned int nBytes = 3;
    for (unsigned int j = 0; j < i; ++j)
    {
        const unsigned char c = pSource[j];
        if (c == '\r' || c == '\n' || c == ' ')
        {
            continue;
        }
        const unsigned char digit = kBase64DecodeValue[c];
        if (digit == 64)
        {
            --nBytes;
        }
        quartet = (quartet << 6) | (digit & 63);
        if (++nQuartet == 4)
        {
            if (nBytes)
            {
                *pWrite++ = (unsigned char)(quartet >> 16);
            }
            if (nBytes > 1)
            {
                *pWrite++ = (unsigned char)(quartet >> 8);
            }
            if (nBytes > 2)
            {
                *pWrite++ = (unsigned char)quartet;
            }
            nQuartet = 0;
        }
    }
    *pOutputSize = (unsigned int)(pWrite - pOutput);
    return 0;
}
