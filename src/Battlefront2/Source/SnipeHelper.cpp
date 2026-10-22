#include "SnipeHelper.h"

#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "CollisionObject.h"
#include "ConnectivityGraph.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "HintManager.h"
#include "UnitAgent.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"

// FUNCTION: SWBF2 0x0063c0c0
bool SnipeHelper::HELPERGrenadeEventInterrupt() const
{
    return true;
}

namespace AI
{

enum AIAffil
{
    AFFIL_ENEMY = 1,
    AFFIL_NEUTRAL = 2,
    AFFIL_ALLY = 4,
    AFFIL_ALLYNEUTRAL = 6,
    AFFIL_ALL = 7
};
}

namespace TeamManager
{

    int GetObjectsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                          int iMax, const GameObject* pIgnore, AI::AIAffil eAffil);

    int GetApproxObjectsAlongLine(const PblVector3& vStart, const PblVector3& vEnd, float fRadius,
                                  GameObject** const ppObjects, int iMax, const GameObject* pIgnore,
                                  AI::AIAffil eAffil);
}

static const float FOV_ANGLE = 2.70526028f;

namespace
{
    const PblVector3& GetMatrixPos(const PblMatrix& m)
    {
        return *(const PblVector3*)&m.m[3][0];
    }

    const CollisionObject* GetCollisionObject(const GameObject* pObj)
    {
        return static_cast<const CollisionObject*>(pObj);
    }

    float DistSq(const PblVector3& a, const PblVector3& b)
    {
        float fX = a.x - b.x;
        float fY = a.y - b.y;
        float fZ = a.z - b.z;
        return fX * fX + fY * fY + fZ * fZ;
    }
}

// FUNCTION: SWBF2 0x0063b6b0
SnipeHelper::SnipeHelper(UnitAgent* pOwner)
    : HelperFsm(STATE_START, STATE_START)
    , mOwner(pOwner)
{
    mNextHintCheckTime = 0.0f;
}

// SYNTHETIC: SWBF2 0x0063b680
// SnipeHelper::`scalar deleting destructor'

const Controllable* SnipeHelper::GetMe() const
{
    return mOwner->GetMe();
}

Controllable* SnipeHelper::GetMe()
{
    return mOwner->GetMe();
}

UnitController* SnipeHelper::GetCtrl()
{
    return mOwner->mCtrl;
}

const AttackHelper* SnipeHelper::GetAttackHelper() const
{
    return reinterpret_cast<const AttackHelper*>(mOwner->mAttackHelper);
}

// FUNCTION: SWBF2 0x0063b6e0
bool SnipeHelper::LookForSnipeHintAndInit()
{
    if (mCurState != 0 && mCurState != STATE_START && mCurState != 100)
    {
        return false;
    }

    if (!AIUtil::IsSniperGuy(GetMe()->GetGameObject()))
    {
        return false;
    }

    if (!AIUtil::IsSnipeHintNearby(GetMe()->GetGameObject(), *this, (const SnipeHint**)&mHint,
                                   &mNextHintCheckTime))
    {
        return false;
    }

    HELPERInit(mHint);
    return true;
}

// FUNCTION: SWBF2 0x0063b760
void SnipeHelper::HELPERInit(const SnipeHint* pHint)
{
    mHint = const_cast<SnipeHint*>(pHint);
    mHint->Reserve(GetMe()->GetGameObject());
    mOverrideDefaultFacePos = false;
    mHeight = HEIGHT_STAND;
    SetNextState(STATE_GOTO);
    FSMChangeState();
}

// FUNCTION: SWBF2 0x0063b7b0
void SnipeHelper::EnterState()
{
    switch (mCurState)
    {
    case STATE_GOTO:
        mOwner->GetLowLevel().SetNavigator_Goto(mHint->GetCollisionCenter(), NULL, true, false);
        break;
    case STATE_SNIPE:
        mHeight = mHint->GetRandomPrimaryStance();
        mCheckTime = 0.0f;
        mSnipeTimer = AIUtil::GetRandomFloat(60.0f, 120.0f) + GameLoop::GetMissionTime();
        break;
    }
}

// FUNCTION: SWBF2 0x0063b840
void SnipeHelper::ExitState()
{
    mOverrideDefaultFacePos = false;
    mHeight = HEIGHT_STAND;
    mOverrideFaceTarget = false;
}

// FUNCTION: SWBF2 0x0063b850
void SnipeHelper::UpdateState(float dt)
{
    switch (mCurState)
    {
    case STATE_GOTO:
        mHint->Reserve(GetMe()->GetGameObject());
        if (mOwner->GetLowLevel().IsNavigatorInProgress())
        {
            break;
        }
        if (DistSq(GetMatrixPos(GetMe()->GetMatrix()), mHint->GetCollisionCenter()) < 4.0f)
        {
            SetNextState(STATE_SNIPE);
        }
        else
        {
            SetNextState(100);
        }
        break;

    case STATE_SNIPE:
    {
        if (GameLoop::GetMissionTime() > mSnipeTimer)
        {
            SetNextState(100);
            return;
        }

        if (GameLoop::GetMissionTime() > mCheckTime)
        {
            mCheckTime = GameLoop::GetMissionTime() + 15.0f;
            if (!IsHintSuitable(mHint, NULL))
            {
                mHint = const_cast<SnipeHint*>(FindSnipeHint());
                if (mHint)
                {
                    mHint->Reserve(GetMe()->GetGameObject());
                    SetNextState(STATE_GOTO);
                }
                else
                {
                    SetNextState(100);
                }
                break;
            }
        }

        mHint->Reserve(GetMe()->GetGameObject());
        PblVector3 vZ = AIUtil::GetSafeNormal(mHint->GetZAxis());
        PblVector3 vMyPos = GetMatrixPos(GetMe()->GetMatrix());

        PblVector3 vFace(vZ.x * 10.0f + vMyPos.x, vZ.y * 10.0f + vMyPos.y, vZ.z * 10.0f + vMyPos.z);

        PblVector3 vTarget;
        const AttackHelper* pAttack = GetAttackHelper();
        if (pAttack->GetVisibleTarget())
        {
            vTarget = GetMatrixPos(GetCollisionObject(pAttack->GetVisibleTarget())->GetMatrix());
        }
        else if (pAttack->HasTarget())
        {
            vTarget = pAttack->GetLastSeenTargetPos();
        }
        else
        {
            vTarget = vFace;
        }

        if (IsWithinSnipeFOV(vTarget))
        {
            mDefaultFacePos = vFace;
            mOverrideFaceTarget = false;
            mOverrideDefaultFacePos = true;
        }
        else
        {
            mOverrideFaceTarget = true;
            mOwner->GetLowLevel().SetHeight(mHeight);
            mOwner->GetLowLevel().SetHeading(vFace, 0.0f);
        }
        GetMe()->SetWpnDistFactor(2.0f);
        break;
    }
    }

    if (!HELPERGetOverrideFaceTarget())
    {
        ControllableHeight eHeight = HELPERGetHeight();
        if (HELPERGetOverrideDefaultFacePos())
        {
            mOwner->FaceTarget(eHeight, HELPERGetDefaultFacePos());
        }
        else
        {
            mOwner->FaceTarget(eHeight);
        }
    }
}

// FUNCTION: SWBF2 0x0063bb80
bool SnipeHelper::IsWithinSnipeFOV(const PblVector3& vPos) const
{
    PblVector3 vZ = AIUtil::GetSafeNormal(mHint->GetZAxis());
    PblVector3 vMyPos = GetMatrixPos(GetMe()->GetMatrix());
    PblVector3 vDir = AIUtil::GetSafeNormal(PblVector3(vPos.x - vMyPos.x, vPos.y - vMyPos.y, vPos.z - vMyPos.z));
    return vZ.x * vDir.x + vZ.y * vDir.y + vZ.z * vDir.z > PblMath::Cos(FOV_ANGLE * 0.5f);
}

// FUNCTION: SWBF2 0x0063bc60
bool SnipeHelper::IsHintSuitable(const SnipeHint* pHint, const Team* pTeam) const
{

    if (AIUtil::IsEndGame(0))
    {
        return false;
    }

    float fDist = AIUtil::gSnipeSuitabilityDist;
    const GameObject* pMe = GetMe()->GetGameObject();
    PblVector3 vHintPos = pHint->GetCollisionCenter();
    const PblVector3& vZ = pHint->GetZAxis();
    PblVector3 vPos(vZ.x * fDist + vHintPos.x, vZ.y * fDist + vHintPos.y, vZ.z * fDist + vHintPos.z);

    GameObject* apObjects[1200];
    int iCount = TeamManager::GetObjectsInRange(vPos, fDist, apObjects, 1200, pMe, AI::AFFIL_ENEMY);
    for (int i = 0; i < iCount; ++i)
    {
        if (AIUtil::IsValidEnemy(pMe, apObjects[i]))
        {
            break;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0063bd70
const SnipeHint* SnipeHelper::FindSnipeHint() const
{
    const GameObject* pMe = GetMe()->GetGameObject();
    PblVector3 vMyPos = GetMatrixPos(GetMe()->GetMatrix());

    PblHeap<const BaseHint*, float>::RECORD aPool[21];
    PblHeap<const BaseHint*, float> heap(20, 0.0f, aPool);

    const BaseHint* apHints[20];
    int iCount = HintManager::RequestSnipeHints(mOwner->mCtrl, vMyPos, 100, 20, apHints);
    for (int i = 0; i < iCount; ++i)
    {
        heap.Push(apHints[i], -DistSq(vMyPos, apHints[i]->GetCollisionCenter()));
    }

    int iTried = 0;
    while (!heap.IsEmpty())
    {
        const SnipeHint* pHint = (const SnipeHint*)heap.Get(0);
        heap.Pop();

        if (!AIUtil::IsHintAvailable(pMe, pHint))
        {
            continue;
        }
        if (iTried++ > 10)
        {
            break;
        }
        if (!IsHintSuitable(pHint, NULL))
        {
            continue;
        }

        PblVector3 vHintPos = pHint->GetCollisionCenter();
        GameObject* apObjects[1200];
        int iObjects = TeamManager::GetApproxObjectsAlongLine(vMyPos, vHintPos, 12.0f, apObjects, 1200,
                                                              pMe, AI::AFFIL_ENEMY);
        int j;
        for (j = 0; j < iObjects; ++j)
        {
            if (AIUtil::IsValidEnemy(pMe, apObjects[j]))
            {
                break;
            }
        }
        if (j < iObjects)
        {
            continue;
        }

        ConnectivityGraph* pGraph =
            ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)AIUtil::GetAIType(pMe));
        if (!pGraph ||
            pGraph->CanGetThereFromHere_QuickButNotSoPerfectlyAccurateTest(mOwner->mCtrl, pHint->GetCollisionCenter()))
        {
            return pHint;
        }
    }
    return NULL;
}

// FUNCTION: SWBF2 0x0063c050
const SnipeHint* SnipeHelper::HELPERFindSnipeHint() const
{
    return FindSnipeHint();
}

// FUNCTION: SWBF2 0x0063c060
bool SnipeHelper::HELPERGetOverrideDefaultFacePos() const
{
    return mOverrideDefaultFacePos;
}

// FUNCTION: SWBF2 0x0063c070
bool SnipeHelper::HELPERGetOverrideFaceTarget() const
{
    return mOverrideFaceTarget;
}

// FUNCTION: SWBF2 0x0063c080
const PblVector3& SnipeHelper::HELPERGetDefaultFacePos() const
{
    return mDefaultFacePos;
}

// FUNCTION: SWBF2 0x0063c090
ControllableHeight SnipeHelper::HELPERGetHeight() const
{
    return mHeight;
}

// FUNCTION: SWBF2 0x0063c0d0
bool SnipeHelper::HELPERDamagedInterrupt(bool b, const PblVector3& vPos) const
{
    if (mCurState == STATE_SNIPE && b)
    {
        return !IsWithinSnipeFOV(vPos);
    }
    return true;
}

// FUNCTION: SWBF2 0x0063c100
bool SnipeHelper::HELPERSeeEnemyInterrupt(const GameObject* pEnemy) const
{
    if (!pEnemy || mCurState != STATE_SNIPE)
    {
        return true;
    }
    const PblMatrix& rEnemy = GetCollisionObject(pEnemy)->GetMatrix();
    if (IsWithinSnipeFOV(GetMatrixPos(rEnemy)))
    {
        return false;
    }
    return DistSq(GetMatrixPos(GetCollisionObject(pEnemy)->GetMatrix()), GetMatrixPos(GetMe()->GetMatrix())) < 1600.0f;
}

// FUNCTION: SWBF2 0x0063c1a0
bool SnipeHelper::HELPERHearEnemyInterrupt(const GameObject* pEnemy) const
{
    if (mCurState == STATE_SNIPE)
    {
        return !IsWithinSnipeFOV(GetMatrixPos(GetCollisionObject(pEnemy)->GetMatrix()));
    }
    return true;
}

// FUNCTION: SWBF2 0x0063c1e0
bool SnipeHelper::HELPERVehicleNearbyInterrupt(const GameObject* pVehicle) const
{
    return false;
}

// FUNCTION: SWBF2 0x0063c200
template void PblHeap<const BaseHint*, float>::Pop();

// FUNCTION: SWBF2 0x0063c0a0
void SnipeHelper::HELPERClean()
{
    ExitState();
    mCurState = 100;
    mNextState = 100;
}
