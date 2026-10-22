#include <float.h>
#include <string.h>

#include "NavigatorSlide.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "PathFinder.h"
#include "PathFollower.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"

extern PblVector3 gVectorZero;

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

class EntityDroideka
{
public:
    static PblHash rttiHashEntityDroideka;
};

class EntityDroid
{
public:
    static PblHash rttiHashEntityDroid;
};

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

namespace
{
    const PblVector3& GetPos(const PblMatrix& m)
    {
        return *(const PblVector3*)&m.m[3][0];
    }

    const PblMatrix& GameObjectGetMatrix(const GameObject* pObj)
    {
        return static_cast<const CollisionObject&>(*pObj).GetMatrix();
    }

    const PblVector3& GameObjectGetVelocity(const GameObject* pObj)
    {
        return pObj->GetVelocity();
    }

    float GameObjectGetMaxSpeed(const GameObject* pObj)
    {
        return pObj->GetMaxSpeed();
    }
}

// GLOBAL: SWBF2 0x007ec4c4

int gUnchangedVehicleSlideCount = 1;
// GLOBAL: SWBF2 0x007ec4c8

int gClearVehicleSlideCount = 1;

// GLOBAL: SWBF2 0x01ecb258
// SYNTHETIC: SWBF2 0x004043f0
// `dynamic initializer for 'NavigatorSlide::rttiHashNavigatorSlide''
PblHash NavigatorSlide::rttiHashNavigatorSlide("NavigatorSlide");
char NavigatorSlide::rttiNameNavigatorSlide[] = "NavigatorSlide";

// FUNCTION: SWBF2 0x005addf0
bool NavigatorSlide::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashNavigatorSlide || Navigator::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005ade20
unsigned int NavigatorSlide::GetDerivedRtti() const
{
    return rttiHashNavigatorSlide;
}

// FUNCTION: SWBF2 0x005ade30
const char* NavigatorSlide::GetDerivedRttiName() const
{
    return rttiNameNavigatorSlide;
}

// SYNTHETIC: SWBF2 0x005ade40
// NavigatorSlide::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005ade70
NavigatorSlide* NavigatorSlidePtr(Navigator* pNavigator)
{
    if (pNavigator && pNavigator->IsRtti(NavigatorSlide::rttiHashNavigatorSlide))
    {
        return (NavigatorSlide*)pNavigator;
    }
    return 0;
}

// FUNCTION: SWBF2 0x005ade90
NavigatorSlide::NavigatorSlide(AILowLevel* pOwner, const PblVector3& globalDest,
                               const Obstacle** ppIgnore, int iIgnoreCnt)
    : Navigator(pOwner)
{
    mGlobalDest = globalDest;
    mSlideObj.mObject = 0;
    mSlideObj.mSavedHandleId = 0;
    mUseAttackHelper = false;
    mOffset = 0.0f;
    mCurState = STATE_START;
    mIgnoreCnt = iIgnoreCnt;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));

    Entity* pEntity = pOwner->mOwner->mOwner->GetEntity();
    if (pEntity->IsRtti(EntitySoldier::rttiHashEntitySoldier) ||
        pEntity->IsRtti(EntityDroideka::rttiHashEntityDroideka) ||
        pEntity->IsRtti(EntityDroid::rttiHashEntityDroid))
    {
        mNextState = STATE_InfantryToPosition;
    }
    else
    {
        mNextState = pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer) ? 100 : STATE_VehicleToPosition;
    }
}

// FUNCTION: SWBF2 0x005adfa0
NavigatorSlide::NavigatorSlide(AILowLevel* pOwner, const GameObject* pSlideObj, float fOffset,
                               bool bUseAttackHelper, const Obstacle** ppIgnore, int iIgnoreCnt)
    : Navigator(pOwner)
{
    mGlobalDest = PblVector3(0, 0, 0);
    mSlideObj.mObject = pSlideObj;
    mSlideObj.mSavedHandleId =
        pSlideObj ? pSlideObj->mHandleId : 0;
    mUseAttackHelper = bUseAttackHelper;
    mOffset = fOffset;
    mCurState = STATE_START;
    mIgnoreCnt = iIgnoreCnt;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    mNextState = STATE_InfantryToObject;
}

// FUNCTION: SWBF2 0x005ae010
NavigatorSlide::~NavigatorSlide()
{
}

// FUNCTION: SWBF2 0x005ae020
bool NavigatorSlide::IsEqual(AILowLevel* pOwner, const PblVector3& globalDest)
{
    return mOwner == pOwner && !(mGlobalDest != globalDest) && mCurState != 100;
}

// FUNCTION: SWBF2 0x005ae070
bool NavigatorSlide::IsEqual(AILowLevel* pOwner, const GameObject* pSlideObj, float fOffset,
                             bool bUseAttackHelper)
{
    if (mOwner != pOwner)
    {
        return false;
    }

    if (mSlideObj.mObject &&
        mSlideObj.mObject->mHandleId != mSlideObj.mSavedHandleId)
    {
        mSlideObj.mObject = 0;
        mSlideObj.mSavedHandleId = 0;
    }

    return mSlideObj.mObject == pSlideObj && mOffset == fOffset &&
           mUseAttackHelper == bUseAttackHelper && mCurState != 100;
}

// FUNCTION: SWBF2 0x005ae0d0
void NavigatorSlide::EnterState()
{
    switch (mCurState)
    {
    case STATE_InfantryToPosition:
        mBounceCount = 0;
        mBounceAtLocalDest = false;
        NextInfantrySlidePos(mGlobalDest);
        break;

    case STATE_VehicleToPosition:
        mSlideDirection = SLIDE_NONE;
        mSlidePos = mGlobalDest;
        mSlidePathDest = gVectorZero;
        NextVehicleSlidePos();
        break;

    case STATE_InfantryToObject:
        mGlobalDest = PblVector3(-FLT_MAX, 0, -FLT_MAX);
        mPrevMeleeTime = -100.0f;
        UpdateInfantrySlideObject();
        break;

    case 100:
        if (mPathFollower)
        {
            delete mPathFollower;
            mPathFollower = 0;
        }
        {
            AI::EventInfo info(AI::EVT_Nav_NavigatorFinished);
            mOwner->mOwner->SendEvent(&info);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x005ae240
void NavigatorSlide::ExitState()
{
}

// FUNCTION: SWBF2 0x005ae250
void NavigatorSlide::UpdateState(float dt)
{
    if (mPathFollower)
    {
        mPathFollower->Update(dt);
    }

    switch (mCurState)
    {
    case STATE_InfantryToPosition:
        UpdateInfantrySlidePos();
        break;
    case STATE_VehicleToPosition:
        UpdateVehicleSlidePos();
        break;
    case STATE_InfantryToObject:
        UpdateInfantrySlideObject();
        break;
    }
}

// FUNCTION: SWBF2 0x005ae2a0
void NavigatorSlide::UpdateInfantrySlidePos()
{
    if (mPathFollower->mDone)
    {
        if (mPathFollower->mFinalPoint != mGlobalDest && ++mBounceCount < 6)
        {
            NextInfantrySlidePos(mBounceAtLocalDest ? GetBouncePosition() : mGlobalDest);
        }
        else
        {
            SetNextState(100);
        }
    }
}

// FUNCTION: SWBF2 0x005ae330
PblVector3 NavigatorSlide::GetBouncePosition() const
{
    float fDist = (float)AIUtil::GetRandomInt(10, 20);
    float fAngle = AIUtil::GetRandomFloat(PblMath::fPi / 4.0f, PblMath::fPi / 2.0f);
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, (AIUtil::GetRandomInt(0, 1) ? -1 : 1) * fAngle);

    PblVector3 dir((mBarrierNormal.x * fCos - mBarrierNormal.z * fSin) * fDist,
                   mBarrierNormal.y * fDist,
                   (mBarrierNormal.z * fCos + mBarrierNormal.x * fSin) * fDist);

    const PblMatrix& mat = GetMatrix();
    return PblVector3(mat.m[3][0] + dir.x, mat.m[3][1] + dir.y, mat.m[3][2] + dir.z);
}

// FUNCTION: SWBF2 0x005ae430
void NavigatorSlide::NextInfantrySlidePos(const PblVector3& dest)
{
    PblVector3 pos = GetPos(GetMatrix());
    PblVector3 slideDest = dest;

    const GameObject* pObj = mOwner->mOwner->mOwner->GetGameObject();
    const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(pObj);
    PathFinder::GetClosestValidPoint(pos, pos, AIUtil::GetCollisionRadius(pObj),
                                     AIUtil::GetAIType(pObj), &pIgnore, 1);
    mBounceAtLocalDest = PathFinder::PushValidPoint(pos, slideDest, mBarrierNormal,
                                                    AIUtil::GetCollisionRadius(pObj),
                                                    AIUtil::GetAIType(pObj), &pIgnore, 1);
    if (mBounceAtLocalDest)
    {
        PblVector3 delta(slideDest.x - pos.x, slideDest.y - pos.y, slideDest.z - pos.z);
        if (delta.x * delta.x + delta.y * delta.y + delta.z * delta.z > 0.1f)
        {
            PblVector3 dir;
            D3DXVec3Normalize(&dir, &delta);
            if (delta.x * delta.x + delta.y * delta.y + delta.z * delta.z > 1.0f)
            {
                slideDest = PblVector3(slideDest.x - dir.x, slideDest.y - dir.y, slideDest.z - dir.z);
            }
            else
            {
                slideDest = pos;
            }
        }
        else
        {
            slideDest = pos;
        }
    }

    delete mPathFollower;
    mPathFollower = new PathFollower(mOwner->mOwner, pos, slideDest, 0, 0, false);
}

// FUNCTION: SWBF2 0x005ae660
void NavigatorSlide::UpdateInfantrySlideObject()
{
    PblVector3 slidePos;
    if (!GetObjectSlideToPosition(&slidePos))
    {
        SetNextState(100);
        return;
    }

    const GameObject* pObj = AIUtil::GetAlivePtr(mSlideObj, true);
    if (mOwner->mOwner->mOwner->GetCurrentCollision() == pObj)
    {
        mOwner->SetDestCurrent();
        return;
    }

    PblVector3 pos = GetPos(GetMatrix());
    float dx = slidePos.x - mGlobalDest.x;
    float dz = slidePos.z - mGlobalDest.z;
    if (dz * dz + dx * dx > 1.0f)
    {
        float fDelay = 1.5f;
        float fx = pos.x - slidePos.x;
        float fz = pos.z - slidePos.z;
        if (fz * fz + fx * fx > 25.0f)
        {
            fDelay = 3.0f;
        }
        if (GameLoop::GetMissionTime() > mPrevMeleeTime + fDelay)
        {
            mPrevMeleeTime = GameLoop::GetMissionTime();
            PblVector3 dest = slidePos;
            const GameObject* pTarget = mOwner->mOwner->GetAttackHelperVisibleTarget();
            if (pTarget)
            {
                const PblVector3& vel = GameObjectGetVelocity(pTarget);
                dest.x += vel.x;
                dest.y += vel.y;
                dest.z += vel.z;
            }
            NextInfantrySlidePos(dest);
            mGlobalDest = dest;
        }
    }
}

// FUNCTION: SWBF2 0x005ae7f0
bool NavigatorSlide::GetObjectSlideToPosition(PblVector3* pPos) const
{
    const GameObject* pObj = AIUtil::GetAlivePtr(mSlideObj, true);
    if (!pObj)
    {
        return false;
    }

    if (mUseAttackHelper)
    {
        if (pObj != mOwner->mOwner->GetAttackHelperTarget())
        {
            return false;
        }
        *pPos = mOwner->mOwner->GetAttackHelperTargetLastSeenPosition();
        return true;
    }

    *pPos = GetPos(GameObjectGetMatrix(pObj));
    const PblVector3& myPos = GetPos(GetMatrix());
    PblVector3 dir(myPos.x - pPos->x, myPos.y - pPos->y, myPos.z - pPos->z);
    PblVector3 normal = AIUtil::GetSafeNormalXZ(dir);
    pPos->x += normal.x * mOffset;
    pPos->y += normal.y * mOffset;
    pPos->z += normal.z * mOffset;
    return true;
}

// FUNCTION: SWBF2 0x005ae900
void NavigatorSlide::NextVehicleSlidePos()
{
    PblVector3 pos = GetPos(GetMatrix());
    const float fRadius = GetMyRadius() * 2.0f;
    const int eProhibit = GetMyProhibit();
    PathFinder::GetClosestValidPoint(pos, pos, fRadius, eProhibit, (const Obstacle**)mIgnore, mIgnoreCnt);

    for (int i = 0; i < 5; ++i)
    {
        const Obstacle* pHit = 0;
        float fStep = GameObjectGetMaxSpeed(GetGameObject()) * 1.5f;
        if (fStep > 30.0f)
        {
            fStep = 30.0f;
        }
        else if (fStep < 1.0f)
        {
            fStep = 1.0f;
        }

        const PblVector3 delta(mSlidePos.x - pos.x, mSlidePos.y - pos.y, mSlidePos.z - pos.z);
        PblVector3 dest = mSlidePos;
        if (delta.z * delta.z + delta.x * delta.x > fStep * fStep)
        {
            const PblVector3 normal = AIUtil::GetSafeNormalXZ(delta);
            dest = PblVector3(pos.x + normal.x * fStep, pos.y + normal.y * fStep, pos.z + normal.z * fStep);
        }

        float fDistSq;
        if (!PathFinder::FindClosestObstacleIntersection(pos, dest, fRadius, eProhibit, &pHit, &fDistSq, false,
                                                         (const Obstacle**)mIgnore, mIgnoreCnt))
        {
            ++gClearVehicleSlideCount;
            break;
        }

        PblVector3 left;
        PblVector3 right;
        pHit->FindCorners(pos, dest, fRadius, &left, &right);
        if (mSlideDirection == SLIDE_NONE)
        {
            const float lx = left.x - pos.x;
            const float lz = left.z - pos.z;
            const float rx = right.x - pos.x;
            const float rz = right.z - pos.z;
            mSlideDirection = (rz * rz + rx * rx <= lz * lz + lx * lx) ? SLIDE_RIGHT : SLIDE_LEFT;
        }
        mSlidePos = (mSlideDirection == SLIDE_LEFT) ? left : right;
    }

    if (!(mSlidePathDest != mSlidePos))
    {
        ++gUnchangedVehicleSlideCount;
        return;
    }

    delete mPathFollower;
    mPathFollower = new PathFollower(mOwner->mOwner, pos, mSlidePos, (const Obstacle**)mIgnore, mIgnoreCnt, false);
    mSlidePathDest = mSlidePos;
}

// FUNCTION: SWBF2 0x005aec40
void NavigatorSlide::UpdateVehicleSlidePos()
{
    if (mPathFollower->mDone)
    {
        if (!(mPathFollower->mFinalPoint != mGlobalDest))
        {
            SetNextState(100);
            return;
        }
        mSlidePos = mGlobalDest;
    }
    NextVehicleSlidePos();
}
