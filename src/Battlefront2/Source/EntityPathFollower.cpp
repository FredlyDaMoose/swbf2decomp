#include <float.h>
#include <stdio.h>

#include "EntityPathFollower.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include <math.h>
#include "CollisionManager.h"
#include "EntityPath.h"
#include "FlagItem.h"
#include "GameObject.h"
#include "NetGame.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblListDoubleSorted.h"
#include "BoundaryManager.h"
#include "Character.h"
#include "EntityCarrier.h"
#include "GameLoop.h"
#include "LockOnManager.h"
#include "Trackable.h"
#include "RedEngineFL/Util/RedStack.h"
#include "CollisionObject.h"
#include "AIUtil.h"
#include "CollisionModel.h"
#include "PebbleFL/Common/PblQuaternion.h"

// FUNCTION: SWBF2 0x004d7900
EntityPathFollowerClass::EntityPathFollowerClass()
{
    mEntityClassID = 0;
    mEntityClassID_CTF = 0;
    mSpeed = 1.0f;
    mTurnSpeed = 0.5f;
    mAcceleration = 0.5f;
    mBranchPaths = 0;
    mBranchRange = FLT_MAX;
    mCosMaxAngle = -1.0f;
    mMinBranchDist = 300.0f;
    mbFlyerVsFlyer = true;

    mPathRollValues.mRollAcceleration = 1.5707964f;
    mPathRollValues.mRollLookAhead = 1.0f;
    mPathRollValues.mRollTolerance = 0.39269909f;
    mPathRollValues.mRollSpeed = 1.5707964f;
    mPathRollValues.mRollAccelResp = 1000.0f;
    mPathRollValues.mRollTurnFactor = 5.0f;
    mPathRollValues.mRollSpeedFactor = 1.0f;
    mPathRollValues.mRollDampening = 0.5f;
}

// GLOBAL: SWBF2 0x00789380
const ODFPropertySet::ODFProperty EntityPathFollowerClass::sPropertySet[15] = {
    { 0x91032dcb, ODFPropertySet::type_Float, offsetof(EntityPathFollowerClass, mSpeed) },
    { 0x1a2fabca, ODFPropertySet::type_Float, offsetof(EntityPathFollowerClass, mTurnSpeed) },
    { 0xab9db3f4, ODFPropertySet::type_Float, offsetof(EntityPathFollowerClass, mAcceleration) },
    { 0x2d976c37, ODFPropertySet::type_Float,
      offsetof(EntityPathFollowerClass, mPathRollValues) + 0x00 },
    { 0x732ff7d1, ODFPropertySet::type_Float,
      offsetof(EntityPathFollowerClass, mPathRollValues) + 0x04 },
    { 0x93425352, ODFPropertySet::type_Float,
      offsetof(EntityPathFollowerClass, mPathRollValues) + 0x08 },
    { 0xb1354466, ODFPropertySet::type_Float,
      offsetof(EntityPathFollowerClass, mPathRollValues) + 0x0c },
    { 0x60900c05, ODFPropertySet::type_Float,
      offsetof(EntityPathFollowerClass, mPathRollValues) + 0x10 },
    { 0xacf747f3, ODFPropertySet::type_Float,
      offsetof(EntityPathFollowerClass, mPathRollValues) + 0x14 },
    { 0xe217bc71, ODFPropertySet::type_Float,
      offsetof(EntityPathFollowerClass, mPathRollValues) + 0x18 },
    { 0x4453dc0a, ODFPropertySet::type_Float,
      offsetof(EntityPathFollowerClass, mPathRollValues) + 0x1c },
    { 0x9f528aec, ODFPropertySet::type_UInt32, offsetof(EntityPathFollowerClass, mBranchPaths) },
    { 0x9eececc5, ODFPropertySet::type_Float, offsetof(EntityPathFollowerClass, mBranchRange) },
    { 0x886f9fe5, ODFPropertySet::type_Float, offsetof(EntityPathFollowerClass, mCosMaxAngle) },
    { 0xe103ce94, ODFPropertySet::type_Float, offsetof(EntityPathFollowerClass, mMinBranchDist) },
};

// FUNCTION: SWBF2 0x004d7990
bool EntityPathFollowerClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x995f7d3c:
        mEntityClassID_CTF = PblHash(pszValue);
        return true;
    case 0xc5cf2a8d:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        mbFlyerVsFlyer = iValue != 0;
        return true;
    }
    case 0xff158b18:
        mEntityClassID = PblHash(pszValue);
        return true;
    }

    if (ODFPropertySet::SetProperty(sPropertySet, sizeof(sPropertySet) / sizeof(sPropertySet[0]), this,
                                    uProperty, pszValue))
    {
        switch (uProperty)
        {
        case 0x2d976c37:
            mPathRollValues.mRollAcceleration *= 0.017453292f;
            return true;
        case 0x93425352:
            mPathRollValues.mRollTolerance *= 0.017453292f;
            return true;
        case 0x60900c05:
            mPathRollValues.mRollAccelResp = 1.0f / mPathRollValues.mRollAccelResp;
            return true;
        case 0x886f9fe5:
            mCosMaxAngle = PblMath::Cos(mCosMaxAngle * 0.017453292f);
            return true;
        case 0xab9db3f4:
            if (mAcceleration != 0.0f)
            {
                mAcceleration = 1.0f / mAcceleration;
            }
            else
            {
                mAcceleration = 0.0001f;
            }
            return true;
        }
    }
    return EntitySquadronClass::SetProperty(uProperty, pszValue);
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

namespace
{

    Controllable* GetControllable(GameObject* pObject)
    {
        typedef Controllable*(__thiscall * GetControllableFn)(GameObject*);
        return pObject->GetControllable();
    }
}

// GLOBAL: SWBF2 0x007eb828
PblList<EntityPathFollower> EntityPathFollower::sList;

// FUNCTION: SWBF2 0x004d78f0
void EntityPathFollower::ClearList()
{
    sList.RemoveAll();
}

// FUNCTION: SWBF2 0x004d7b40
template void PblList<EntityPathFollower>::RemoveAll();

// FUNCTION: SWBF2 0x004d2710
EntityPathFollower::BranchParams::BranchParams(const EntityPathFollowerClass* pPathClass, PblMatrix& matrix,
                                               float fDiameter, GameObject* const pGameObject,
                                               float fWorldSpeed, float fRange, unsigned int uBranchPaths,
                                               float fCosMaxAngle, bool bOverrideBranching,
                                               bool bFallbackToClosest)
    : mPathClass(pPathClass), mMatrix(matrix), mDiameter(fDiameter), mGameObject(pGameObject),
      mWorldSpeed(fWorldSpeed)
{
    if (fRange < FLT_MAX)
    {
        fRange *= fRange;
    }
    mSquareRange = fRange;
    mBranchPaths = uBranchPaths;
    mCosMaxAngle = fCosMaxAngle;
    mOverrideBranching = bOverrideBranching;
    mFallbackToClosest = bFallbackToClosest;
}

// FUNCTION: SWBF2 0x004d2770
EntityPathFollower::EntityPathFollower(const EntityPathFollowerClass* const pClass)
{
    mSpeed = 1.0f;
    mOffPathSpeed = 1.0f;
    mCurSpeed = 0.0f;
    mAcceleration = 1.0f;
    mYaw = FLT_MAX;
    mSplineRoll = 0.0f;
    mCurRoll = 0.0f;
    mCurRollAcceleration = 0.0f;
    mRollVelocity = 0.0f;
    mStrafeTimer = 0.0f;
    mT = 0.0f;
    mPath = 0;
    mIndex = 0;
    mDirection = 0;
    mSetPoints = 0;
    mStaredownTimer = 0.0f;
    mFireTimer = 10.0f;
    mBoostTimer = 0.0f;
    mbTeleportNow = false;
    mbQueuePoint = false;
    mbGoMinSpeed = false;
    mbDelayIndexIncrement = false;
    mbCirclingPoint = false;
    mbLandNow = false;
    mbStraightLine = false;
    if (pClass)
    {
        mSpeed = pClass->mSpeed;
        mAcceleration = pClass->mAcceleration;
    }
    sList.AddLast(this, this);
    mAttacker = 0;
}

// SYNTHETIC: SWBF2 0x004d28e0
// EntityPathFollower::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004d2910
EntityPathFollower::~EntityPathFollower()
{
    if (PblList<EntityPathFollower>::Node::IsInList())
    {
        GetList()->Remove(this);
    }
    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x004d2700
unsigned int EntityPathFollower::GetAllegiance()
{
    return 0;
}

// FUNCTION: SWBF2 0x00477670
bool EntityPathFollower::CanJoinSquadron() const
{
    return !InSquadron();
}

// FUNCTION: SWBF2 0x004d2970
void EntityPathFollower::Reset(const EntityPathFollowerClass* const pClass)
{
    RemovePathRef(pClass);
    if (InSquadron())
    {
        RemoveFromSquadron();
    }
    mSetPoints = 0;
    ResetStateVars();
}

// FUNCTION: SWBF2 0x004d29a0
void EntityPathFollower::ResetStateVars()
{
    mStaredownTimer = 0.0f;
    mbTeleportNow = false;
    mbQueuePoint = false;
    mbGoMinSpeed = false;
    mbDelayIndexIncrement = false;
    mbStraightLine = false;
}

// FUNCTION: SWBF2 0x004d29c0
void EntityPathFollower::RemovePathRef(const EntityPathFollowerClass* const pClass)
{
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        if (mPath)
        {
            mPath->Remove(mLastUsedID, &mNode);
        }
        mPath = 0;
        mDirection = 0;
        mIndex = 0;
        mT = 0.0f;
    }
}

// FUNCTION: SWBF2 0x004d2a30
bool EntityPathFollower::MoveTo(const PathStart& start, const PblVector3* const pPosition,
                                const PblVector3* const pDirection, BranchParams& params,
                                bool bUseCTFClass, bool b2, bool b3)
{
    RemovePathRef(params.mPathClass);
    mPath = start.mAttrib.mPath;
    mIndex = start.mAttrib.mPointIndex;
    mDirection = start.mDirection;
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        const EntityPathFollowerClass* pClass = params.mPathClass;
        mLastUsedID = (bUseCTFClass && pClass->mEntityClassID_CTF) ? pClass->mEntityClassID_CTF
                                                                   : pClass->mEntityClassID;
        if (!mPath->Add(mLastUsedID, mDirection, &mNode))
        {
            mPath = 0;
            Reset(params.mPathClass);
            return false;
        }
    }
    SetupNextSegment(pPosition, params, false, b3);
    return true;
}

// FUNCTION: SWBF2 0x004d2b00
bool EntityPathFollower::HeadTowards(PblVector3& vTarget, float fDistance, BranchParams& params)
{
    const GameObject* pIgnore = params.mGameObject;
    PblVector3 vPosition = params.mMatrix.GetTranslation();
    PblVector3 vDelta(vTarget.x - vPosition.x, vTarget.y - vPosition.y, vTarget.z - vPosition.z);
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vDelta);
    PblVector3 vEnd(vDir.x * fDistance + vPosition.x, vDir.y * fDistance + vPosition.y,
                    vDir.z * fDistance + vPosition.z);
    if (CollisionManager::RayTest(vPosition, vEnd, &pIgnore, 1, 0))
    {
        return false;
    }

    Reset(params.mPathClass);
    const PblVector3& vForward = params.mMatrix.GetZAxis();
    PblVector3 vBehind(vPosition.x - vForward.x * 30.0f, vPosition.y - vForward.y * 30.0f,
                       vPosition.z - vForward.z * 30.0f);
    PblVector3 vDelta2(vTarget.x - vPosition.x, vTarget.y - vPosition.y, vTarget.z - vPosition.z);
    PblVector3 vBeyond;
    D3DXVec3Normalize(&vBeyond, &vDelta2);
    float fBeyond = fDistance + 200.0f;
    vBeyond.x = vBeyond.x * fBeyond + vPosition.x;
    vBeyond.y = vBeyond.y * fBeyond + vPosition.y;
    vBeyond.z = vBeyond.z * fBeyond + vPosition.z;
    SetPoint(vBehind, 0);
    SetPoint(vPosition, 1);
    SetPoint(vEnd, 2);
    SetPoint(vBeyond, 3);
    SmoothPath(params, &vPosition, 20.0f, 125.0f, 300.0f, 30.0f, 0.707f);
    return true;
}

// FUNCTION: SWBF2 0x004d2d30
void EntityPathFollower::SmoothPath(BranchParams& params, const PblVector3* pTarget, float fMinOffset,
                                    float fMaxOffset, float fDistRange, float fForwardScale,
                                    float fCosAngle)
{
    if (!pTarget || !GetPointIsSet(2))
    {
        return;
    }

    bool bForce = fCosAngle >= 1.0f;
    PblVector3 vRel(pTarget->x - mOffset.x, pTarget->y - mOffset.y, pTarget->z - mOffset.z);
    const PblVector3& vPoint2 = GetPoint(2);
    PblVector3 vDelta(vPoint2.x - vRel.x, vPoint2.y - vRel.y, vPoint2.z - vRel.z);
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vDelta);

    PblVector3 vForward = params.mMatrix.GetZAxis();
    PblVector3 vPoint = vForward;
    if (mPath)
    {
        mPath->CalculateDirection(mIndex, mDirection, vPoint);
    }

    const PblMatrix& m = params.mMatrix;
    float fDot = m.m[2][0] * vDir.x + m.m[2][1] * vDir.y + m.m[2][2] * vDir.z;
    if (!(fCosAngle > fDot) && !(fCosAngle > vDir.x * vPoint.x + vDir.y * vPoint.y + vDir.z * vPoint.z) &&
        !bForce)
    {
        return;
    }

    vPoint.x = m.m[1][2] * vDir.y - m.m[1][1] * vDir.z;
    vPoint.y = m.m[1][0] * vDir.z - m.m[1][2] * vDir.x;
    vPoint.z = m.m[1][1] * vDir.x - m.m[1][0] * vDir.y;
    if (m.m[2][0] * vPoint.x + m.m[2][1] * vPoint.y + m.m[2][2] * vPoint.z < 0.0f)
    {
        vPoint.x = -vPoint.x;
        vPoint.y = -vPoint.y;
        vPoint.z = -vPoint.z;
    }

    float fDist = sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
    float fDistT = fDist / fDistRange;
    float fAngleT = (1.0f - fDot) * 3.0f;
    if (0.0f > fAngleT)
    {
        fAngleT = 0.0f;
    }
    else if (fAngleT > 1.0f)
    {
        fAngleT = 1.0f;
    }
    if (0.0f > fDistT)
    {
        fDistT = 0.0f;
    }
    else if (fDistT > 1.0f)
    {
        fDistT = 1.0f;
    }
    float fT = fDistT * fAngleT;
    float fOffset = fT * fMaxOffset;
    if (fMinOffset > fOffset)
    {
        fOffset = fMinOffset;
    }
    else if (fOffset > fMaxOffset)
    {
        fOffset = fMaxOffset;
    }
    float fLead = fDot;
    if (0.0f > fDot)
    {
        fLead = 0.0f;
    }
    else if (fDot > 1.0f)
    {
        fLead = 1.0f;
    }
    fLead *= 0.5f;

    PblVector3 vLead;
    if (fDot > 0.0f)
    {
        vLead.x = vDir.x * fDist * fLead;
        vLead.y = vDir.y * fDist * fLead;
        vLead.z = vDir.z * fDist * fLead;
    }
    else
    {
        vLead.x = vForward.x * fForwardScale * fT;
        vLead.y = vForward.y * fForwardScale * fT;
        vLead.z = vForward.z * fForwardScale * fT;
    }
    PblVector3 vSide(vPoint.x * fOffset, vPoint.y * fOffset, vPoint.z * fOffset);
    vPoint.x = pTarget->x - mOffset.x + vSide.x + vLead.x;
    vPoint.y = pTarget->y - mOffset.y + vSide.y + vLead.y;
    vPoint.z = pTarget->z - mOffset.z + vSide.z + vLead.z;

    if (150.0f > fDist && !bForce)
    {
        return;
    }

    if (!RayTestSpline(GetPoint(0), GetPoint(1), vPoint, GetPoint(2), params.mGameObject, 3))
    {

        if (fDot > 0.0f)
        {
            vLead.x = vDir.x * fDist * fLead * 0.25f;
            vLead.y = vDir.y * fDist * fLead * 0.25f;
            vLead.z = vDir.z * fDist * fLead * 0.25f;
        }
        else
        {
            vLead.x = vForward.x * fForwardScale * fT * 0.25f;
            vLead.y = vForward.y * fForwardScale * fT * 0.25f;
            vLead.z = vForward.z * fForwardScale * fT * 0.25f;
        }
        vPoint.x = pTarget->x - mOffset.x + vSide.x * 0.25f + vLead.x;
        vPoint.y = pTarget->y - mOffset.y + vSide.y * 0.25f + vLead.y;
        vPoint.z = pTarget->z - mOffset.z + vSide.z * 0.25f + vLead.z;
        if (!bForce && !RayTestSpline(GetPoint(0), GetPoint(1), vPoint, GetPoint(2), params.mGameObject, 2))
        {
            return;
        }
    }
    SetPoint(GetPoint(2), 3);
    SetPoint(vPoint, 2);
    mbDelayIndexIncrement = true;
}

// FUNCTION: SWBF2 0x004d32f0
void EntityPathFollower::DelayPath(BranchParams& params, float fDistance)
{
    if (GetPointIsSet(2))
    {
        SetPoint(GetPoint(2), 3);
        const PblMatrix& m = params.mMatrix;
        PblVector3 vAhead(m.m[2][0] * fDistance + m.m[3][0], m.m[2][1] * fDistance + m.m[3][1],
                          m.m[2][2] * fDistance + m.m[3][2]);
        SetPoint(vAhead, 2);
        mbDelayIndexIncrement = true;
    }
    mbStraightLine = true;
}

// FUNCTION: SWBF2 0x004d3390
bool EntityPathFollower::IsFlyerPlayerCentric(BranchParams& params)
{
    if (FlagItem::sFlagGameplayType == 2 || FlagItem::sFlagGameplayType == 3)
    {
        return false;
    }
    Controllable* pControllable = params.mGameObject ? GetControllable(params.mGameObject) : 0;
    UnitController* pController = UnitControllerPtr(pControllable);
    return !pController || (char)(pController->GetRandomSeed() & 0xf) > 7;
}

// FUNCTION: SWBF2 0x004d66a0
const EntityPath* EntityPathFollower::GetPath() const
{
    return mPath;
}

extern bool gAttractMode;

namespace
{

    bool IsCarryingFlag(GameObject* pObject)
    {
        return pObject && pObject->GetControllable() && pObject->GetControllable()->mCharacter &&
               pObject->GetControllable()->mCharacter->mCarriedFlagPtr;
    }
}

// FUNCTION: SWBF2 0x004d33e0
unsigned int EntityPathFollower::CreateSquadronForFlyby(BranchParams& params)
{
    if (NetGame::IsEnabled() || 30.0f > GameLoop::GetMissionTime() || !params.mPathClass->mbFlyerVsFlyer ||
        gAttractMode)
    {
        return 0;
    }

    bool bCreateSquadron = !GetEntitySquadronClass()->mNumFormations && !InSquadron();
    if (!IsFlyerPlayerCentric(params))
    {
        return 0;
    }
    Character* pPlayer = NetGame::GetLocalPlayer(0);
    if (!pPlayer)
    {
        return 0;
    }
    Controllable* pPlayerControllable = pPlayer->GetControllable();
    if (!pPlayerControllable)
    {
        return 0;
    }
    if (!static_cast<Trackable*>(pPlayerControllable)->GetGameObject()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        return 0;
    }

    PblVector3 vForward = pPlayerControllable->GetMatrix().GetZAxis();
    PblVector3 vPlayerPos = pPlayerControllable->GetMatrix().GetTranslation();
    PblVector3 vPosition = params.mMatrix.GetTranslation();
    PblVector3 vDelta(vPosition.x - vPlayerPos.x, vPosition.y - vPlayerPos.y, vPosition.z - vPlayerPos.z);
    float fDistSq = (vPlayerPos.y - vPosition.y) * (vPlayerPos.y - vPosition.y) +
                    (vPlayerPos.x - vPosition.x) * (vPlayerPos.x - vPosition.x) +
                    (vPlayerPos.z - vPosition.z) * (vPlayerPos.z - vPosition.z);
    PblVector3 vAhead(vForward.x * 300.0f + vPlayerPos.x, vForward.y * 300.0f + vPlayerPos.y,
                      vForward.z * 300.0f + vPlayerPos.z);
    if (BoundaryManager::sInstance->IsOutOfBounds(vAhead) || 2500.0f > fDistSq ||
        (vForward.x * vDelta.x + vForward.y * vDelta.y + vForward.z * vDelta.z > 0.25f && 250000.0f > fDistSq))
    {
        return 0;
    }
    if (mbQueuePoint || mbStraightLine)
    {
        return 0;
    }
    if (LockOnManager::IsGameObjectTargetedAll(params.mGameObject))
    {
        return 0;
    }
    int iPlayerTeam = static_cast<Trackable*>(pPlayerControllable)->GetGameObject()->GetTeam();
    if (GetAllegiance() == (unsigned int)iPlayerTeam && (PblRandom::GetGlobalInt() & 3))
    {
        return 0;
    }

    unsigned int uCount = 0;
    unsigned int uSquadronSize = (int)PblRandom::GetGlobalFloatRange(4.5f, 3.0f);
    bool bSameDirection = ((PblRandom::GetGlobalInt() >> 1) & 1) != 0;
    float fRadius = static_cast<CollisionObject*>(params.mGameObject)->GetCollisionRadius() * 6.0f;
    float fSide = bSameDirection ? 4.0f : -3.0f;

    const PblMatrix& mUp = pPlayerControllable->GetMatrix();
    float fUp = PblRandom::GetGlobalFloatRange(0.0f, 2.0f);
    PblVector3 vUp(mUp.m[1][0] * fUp, mUp.m[1][1] * fUp, mUp.m[1][2] * fUp);
    const PblMatrix& mRight = pPlayerControllable->GetMatrix();
    float fRight = PblRandom::GetGlobalFloatRange(0.0f, 2.0f);
    PblVector3 vRandom(mRight.m[0][0] * fRight + vUp.x, mRight.m[0][1] * fRight + vUp.y,
                       mRight.m[0][2] * fRight + vUp.z);
    PblVector3 vOffset;
    D3DXVec3Normalize(&vOffset, &vRandom);
    PblVector3 vSum(vForward.x * 85.0f + vOffset.x * 85.0f, vOffset.y * 85.0f + vForward.y * 85.0f,
                    vOffset.z * 85.0f + vForward.z * 85.0f);
    PblVector3 vEntry;
    D3DXVec3Normalize(&vEntry, &vSum);
    PblVector3 vBack(vForward.x * fSide - vOffset.x, vForward.y * fSide - vOffset.y, vForward.z * fSide - vOffset.z);
    PblVector3 vExit;
    D3DXVec3Normalize(&vExit, &vBack);
    float fLead = bSameDirection ? 85.0f : 700.0f;
    PblVector3 vStart(vOffset.x * 85.0f + vPlayerPos.x + vForward.x * fLead,
                      vOffset.y * 85.0f + vPlayerPos.y + vForward.y * fLead,
                      vOffset.z * 85.0f + vPlayerPos.z + vForward.z * fLead);

    int iTries = 0;
    GameObject* pFound;
    while (CollisionManager::GetObjectsInRange(vStart, fRadius, &pFound, 1,
                                               static_cast<Trackable*>(pPlayerControllable)->GetGameObject(), 0))
    {
        ++iTries;
        const PblMatrix& mUp2 = pPlayerControllable->GetMatrix();
        fUp = PblRandom::GetGlobalFloatRange(0.0f, 2.0f);
        vUp = PblVector3(mUp2.m[1][0] * fUp, mUp2.m[1][1] * fUp, mUp2.m[1][2] * fUp);
        const PblMatrix& mRight2 = pPlayerControllable->GetMatrix();
        fRight = PblRandom::GetGlobalFloatRange(0.0f, 2.0f);
        vRandom = PblVector3(fRight * mRight2.m[0][0] + vUp.x, mRight2.m[0][1] * fRight + vUp.y,
                             mRight2.m[0][2] * fRight + vUp.z);
        D3DXVec3Normalize(&vOffset, &vRandom);
        vSum = PblVector3(vForward.x * 85.0f + vOffset.x * 85.0f, vOffset.y * 85.0f + vForward.y * 85.0f,
                          vOffset.z * 85.0f + vForward.z * 85.0f);
        D3DXVec3Normalize(&vEntry, &vSum);
        vBack = PblVector3(vForward.x * fSide - vOffset.x, vForward.y * fSide - vOffset.y,
                           vForward.z * fSide - vOffset.z);
        D3DXVec3Normalize(&vExit, &vBack);
        fLead = bSameDirection ? 85.0f : 700.0f;
        vStart = PblVector3(vPlayerPos.x + vOffset.x * 85.0f + vForward.x * fLead,
                            vOffset.y * 85.0f + vPlayerPos.y + vForward.y * fLead,
                            vOffset.z * 85.0f + vPlayerPos.z + vForward.z * fLead);
        if (iTries > 3)
        {
            return 0;
        }
    }

    if (!bCreateSquadron || CreateSquadron(uSquadronSize))
    {
        uCount = 1;
        RemovePathRef(params.mPathClass);
        mSetPoints = 0;
        ResetStateVars();

        float fSlotOffset = bCreateSquadron ? GetFormationSlot() * 15.0f : 0.0f;
        PblVector3 vPos(vStart.x + fSlotOffset * vEntry.x, vEntry.y * fSlotOffset + vStart.y,
                        vEntry.z * fSlotOffset + vStart.z);
        PblMatrix& matrix = params.mMatrix;
        matrix.m[3][0] = vPos.x;
        matrix.m[3][1] = vPos.y;
        matrix.m[3][2] = vPos.z;
        matrix.m[3][3] = 1.0f;
        static_cast<CollisionObject*>(params.mGameObject)->mLastPosition = vPos;
        if (params.mGameObject->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            static_cast<EntityFlyer*>(params.mGameObject)->TurnOffContrailEffects();
        }

        PblVector3 vDir = bSameDirection ? vForward : PblVector3(-vForward.x, -vForward.y, -vForward.z);
        matrix.m[2][0] = vDir.x;
        matrix.m[2][1] = vDir.y;
        matrix.m[2][2] = vDir.z;
        matrix.m[2][3] = 0.0f;
        EndOfSegment(params, &vPos, true, false);

        if (!(bCreateSquadron && mPath && InSquadron() && IsSquadronLeader()))
        {
            PblVector3 vTarget(vStart.x + vExit.x * 200.0f, vExit.y * 200.0f + vStart.y, vExit.z * 200.0f + vStart.z);
            HeadTowards(vTarget, 750.0f, params);
            mBoostTimer = bSameDirection ? 6.0f : 0.0f;
            return uCount;
        }

        for (PblList<EntityPathFollower>::Itor it(sList); (EntityPathFollower*)it; ++it)
        {
            if ((EntityPathFollower*)it == this)
            {
                continue;
            }
            BranchParams otherParams = it->GetBranchParams();
            if (!IsFlyerPlayerCentric(otherParams))
            {
                continue;
            }
            unsigned int uAllegiance = GetAllegiance();
            unsigned int uOther = it->GetAllegiance();
            if (uAllegiance && uOther && uAllegiance != uOther)
            {
                continue;
            }
            if (!it->CanJoinSquadron() || it->mbQueuePoint)
            {
                continue;
            }
            const PblMatrix& m = otherParams.mMatrix;
            if (!(0.5f > vForward.x * (m.m[3][0] - vPlayerPos.x) + (m.m[3][1] - vPlayerPos.y) * vForward.y +
                             (m.m[3][2] - vPlayerPos.z) * vForward.z))
            {
                continue;
            }
            if (LockOnManager::IsGameObjectTargetedAll(otherParams.mGameObject) || IsCarryingFlag(params.mGameObject))
            {
                continue;
            }
            if (AddToSquadron(it, -1))
            {
                ++uCount;
                it->mbTeleportNow = true;
                it->mbStraightLine = true;
            }
            if (uCount >= uSquadronSize)
            {
                break;
            }
        }
        if (uCount)
        {
            return uCount;
        }
    }

    if (InSquadron())
    {
        Reset(params.mPathClass);
        EndOfSegment(params, &vPosition, true, false);
    }
    return uCount;
}

// FUNCTION: SWBF2 0x004d3fa0
unsigned int EntityPathFollower::CreateSquadronFromClosestEntities(const PblVector3& vPosition,
                                                                   const PblVector3& vDirection, float fSquareRange,
                                                                   float fCosMaxAngle)
{
    if (NetGame::IsEnabled() || !GetBranchParams().mPathClass->mbFlyerVsFlyer ||
        (!GetEntitySquadronClass()->mNumFormations && !InSquadron()) || !mPath)
    {
        return 0;
    }

    class Candidate
    {
    public:

        // FUNCTION: SWBF2 0x004d43c0
        void* operator new(size_t, void* pMemory) { return pMemory; }
        void operator delete(void*, void*) {}

        EntityPathFollower* mFollower;
        PblListDoubleSorted::Node mNode;
    };

    RedStack stack;
    PblListDoubleSorted candidates;
    unsigned int uCount = 0;
    char* pMemory = (char*)stack.Alloc(sList.GetCount() * sizeof(Candidate));
    unsigned int uAllegiance = GetAllegiance();
    for (PblList<EntityPathFollower>::Itor it(sList); (EntityPathFollower*)it; ++it)
    {
        if ((EntityPathFollower*)it == this)
        {
            continue;
        }
        unsigned int uOther = it->GetAllegiance();
        if (uAllegiance && uOther && uAllegiance != uOther)
        {
            continue;
        }
        if (!it->CanJoinSquadron())
        {
            continue;
        }
        const PblMatrix& m = it->GetMatrix();
        float fDistSq = (vPosition.x - m.m[3][0]) * (vPosition.x - m.m[3][0]) +
                        (vPosition.y - m.m[3][1]) * (vPosition.y - m.m[3][1]) +
                        (vPosition.z - m.m[3][2]) * (vPosition.z - m.m[3][2]);
        if (fSquareRange >= fDistSq)
        {
            float fDot = fCosMaxAngle > -1.0f
                             ? m.m[2][1] * vDirection.y + vDirection.x * m.m[2][0] + m.m[2][2] * vDirection.z
                             : fCosMaxAngle;
            if (fDot >= fCosMaxAngle)
            {
                ++uCount;
                Candidate* pCandidate = new (pMemory) Candidate;
                pMemory += sizeof(Candidate);
                pCandidate->mFollower = it;
                pCandidate->mNode.mSortValue = (1.0f - fDistSq / fSquareRange) + (fDot + 1.0f);
                candidates.Add(&pCandidate->mNode);
            }
        }
    }

    if (uCount)
    {
        if (uCount > mPath->mMaxSquadronSize)
        {
            uCount = mPath->mMaxSquadronSize;
        }
        do
        {
            if (CreateSquadron(uCount + 1))
            {
                unsigned int uAdded = 0;
                for (PblListDouble::Node* pLink = candidates.mTerminator.mNext; pLink != &candidates.mTerminator;
                     pLink = pLink->mNext)
                {
                    PblListDoubleSorted::Node* pNode =
                        (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
                    if (pNode == candidates.GetTerminator())
                    {
                        break;
                    }
                    Candidate* pCandidate = (Candidate*)pNode->GetObjectA(&candidates, offsetof(Candidate, mNode));
                    if (AddToSquadron(pCandidate->mFollower, -1) && ++uAdded == uCount)
                    {
                        break;
                    }
                }
                if (uAdded == uCount)
                {
                    PathStart start;
                    start.mNode.mSortValue = 1.0f;
                    start.mAttrib.mPath = mPath;
                    start.mAttrib.mPointIndex = mIndex;
                    start.mAttrib.mSquareDist = 0.0f;
                    start.mAttrib.mVectorDist = PblVector3(0.0f, 0.0f, 0.0f);
                    start.mDirection = mDirection;
                    for (PblListDouble::Node* pLink = candidates.mTerminator.mNext;
                         pLink != &candidates.mTerminator; pLink = pLink->mNext)
                    {
                        PblListDoubleSorted::Node* pNode =
                            (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
                        if (pNode == candidates.GetTerminator())
                        {
                            break;
                        }
                        EntityPathFollower* pFollower =
                            ((Candidate*)pNode->GetObjectA(&candidates, offsetof(Candidate, mNode)))->mFollower;
                        if (!pFollower->InSquadron())
                        {
                            continue;
                        }
                        BranchParams params = pFollower->GetBranchParams();
                        if (pFollower->mPath != start.mAttrib.mPath ||
                            pFollower->mIndex != start.mAttrib.mPointIndex)
                        {
                            if (pFollower->MoveTo(start, &pFollower->GetMatrix().GetTranslation(),
                                                  &pFollower->GetMatrix().GetZAxis(), params, false, false, true))
                            {
                                pFollower->mT = 0.0f;
                            }
                        }
                    }
                    break;
                }
            }
        } while (--uCount > 1);
    }
    candidates.Clear();
    return uCount;
}

// FUNCTION: SWBF2 0x004d4400
void EntityPathFollower::SetSquadronLeader(const EntityPathFollowerClass* const pClass, EntityPathFollower* pLeader)
{
    mSpeed = pClass->mSpeed;
    pLeader->EntitySquadron::SetSquadronLeader(this);
}

// FUNCTION: SWBF2 0x004d4420
bool EntityPathFollower::Branch(BranchParams& params, const PblVector3* const pPosition, bool bHavePosition)
{
    ResetStateVars();
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return false;
    }

    PathStart start;
    if (InSquadron())
    {
        EntityPathFollower* pLeader = static_cast<EntityPathFollower*>(GetSquadronLeader());
        if (pLeader && !IsSquadronLeader() && pLeader->mPath == mPath &&
            mIndex * mDirection >= pLeader->mIndex * pLeader->mDirection)
        {
            SetSquadronLeader(params.mPathClass, pLeader);
        }
    }

    bool bHasFlag = IsCarryingFlag(params.mGameObject);
    if (InSquadron() && !IsSquadronLeader())
    {
        const EntityPathFollower* pLeader = static_cast<const EntityPathFollower*>(GetSquadronLeader());
        start.mAttrib.mVectorDist = PblVector3(0.0f, 0.0f, 0.0f);
        start.mAttrib.mPath = pLeader->mPath;
        start.mAttrib.mPointIndex = pLeader->mIndex;
        start.mAttrib.mSquareDist = 0.0f;
        start.mDirection = pLeader->mDirection;
        start.mNode.mSortValue = 1.0f;
    }
    else
    {
        int iPrev = mIndex - mDirection;
        const EntityPath::BranchRegion* pRegion = 0;
        const EntityPath* pExcludePath = 0;
        const PblVector3* pExclude = 0;
        const EntityPath::NodeProperties* pProps =
            mPath && iPrev >= 0 && iPrev < mPath->GetNumPoints() ? &mPath->GetNodeProperties(iPrev) : 0;
        if (!params.mOverrideBranching)
        {
            const EntityPathFollowerClass* pClass = params.mPathClass;
            params.mBranchPaths = pClass->mBranchPaths;
            params.mSquareRange = pClass->mBranchRange;
            params.mCosMaxAngle = pClass->mCosMaxAngle;
            if (pProps)
            {

                if ((pProps->mBranchSet & 1) > 0)
                {
                    params.mBranchPaths = pProps->mNumBranchPaths;
                }
                if ((pProps->mBranchSet & 4) > 0)
                {
                    pRegion = pProps->GetBranchRegion();
                    params.mSquareRange = pProps->GetRange();
                }
                if ((pProps->mBranchSet & 2) > 0)
                {
                    params.mCosMaxAngle = pProps->mCosMaxAngle;
                }
            }
        }
        if (pProps)
        {
            pExclude = mPath->GetPointPosition(iPrev);
            pExcludePath = mPath->mDifferentPath ? mPath : 0;
        }

        const PblVector3* pCenter;
        float fRange;
        if (pRegion)
        {
            fRange = pRegion->mSize;
            pCenter = &pRegion->mPosition;
        }
        else
        {
            fRange = params.mSquareRange;
            if (!(fRange > 0.0f))
            {
                fRange = FLT_MAX;
            }
            pCenter = &params.mMatrix.GetTranslation();
        }
        const EntityPathFollowerClass* pClass = params.mPathClass;
        unsigned int uClassID = bHasFlag && pClass->mEntityClassID_CTF ? pClass->mEntityClassID_CTF
                                                                         : pClass->mEntityClassID;
        FindPath(pCenter, &params.mMatrix.GetZAxis(), fRange, params.mBranchPaths, params.mCosMaxAngle,
                 pClass->mMinBranchDist, uClassID, pExclude, pExcludePath, params.mGameObject,
                 params.mFallbackToClosest, start);
    }

    bool bResult = false;
    if (start.mAttrib.mPath && MoveTo(start, pPosition ? pPosition : &params.mMatrix.GetTranslation(), 0, params,
                                      bHasFlag, false, bHavePosition))
    {
        bResult = true;
        if (mbStraightLine)
        {
            DelayPath(params, 100.0f);
        }
    }
    else
    {
        Reset(params.mPathClass);
    }
    params.mFallbackToClosest = false;
    return bResult;
}

// FUNCTION: SWBF2 0x004d4760
void EntityPathFollower::EndOfSegment(BranchParams& params, const PblVector3* const pPosition, bool bForceBranch,
                                      bool bSetupNext)
{
    bool bBranch = bForceBranch || (!mPath && !GetPointIsSet(2) && !GetPointIsSet(3));
    bBranch = bBranch || (mPath && (mIndex < 0 || mIndex >= mPath->GetNumPoints()));
    if (!bBranch && mPath)
    {
        int iPrev = mIndex - mDirection;
        const EntityPath::NodeProperties* pProps =
            iPrev >= 0 && iPrev < mPath->GetNumPoints() ? &mPath->GetNodeProperties(iPrev) : 0;
        bBranch = pProps && pProps->mBranchProbability > PblRandom::GetGlobalFloat();
    }

    if (bBranch)
    {
        if (!NetGame::IsEnabled() && !gAttractMode && params.mPathClass->mbFlyerVsFlyer &&
            IsFlyerPlayerCentric(params))
        {
            Character* pPlayer = NetGame::GetLocalPlayer(0);
            if (pPlayer && pPlayer->GetVehicle())
            {
                Controllable* pControllable = pPlayer->GetControllable();
                if (pControllable)
                {
                    PblVector3 vPosition = pPosition ? *pPosition : params.mMatrix.GetTranslation();
                    PblVector3 vTarget = pControllable->GetMatrix().GetTranslation();
                    if ((vTarget.x - vPosition.x) * (vTarget.x - vPosition.x) +
                                (vTarget.y - vPosition.y) * (vTarget.y - vPosition.y) +
                                (vTarget.z - vPosition.z) * (vTarget.z - vPosition.z) >
                            160000.0f &&
                        HeadTowards(vTarget, 150.0f, params))
                    {
                        return;
                    }
                }
            }
        }
        Branch(params, pPosition ? pPosition : &params.mMatrix.GetTranslation(), pPosition != 0);
    }
    else if (bSetupNext)
    {
        SetupNextSegment(pPosition ? pPosition : &params.mMatrix.GetTranslation(), params, true, false);
    }
}

// FUNCTION: SWBF2 0x004d65f0
void EntityPathFollower::SetPoint(const PblVector3& vPoint, unsigned char uIndex)
{
    mPathSpline.GetControlPoints()[uIndex] = vPoint;
    mSetPoints |= 1 << uIndex;
}

// FUNCTION: SWBF2 0x004d6650
const PblVector3& EntityPathFollower::GetPoint(unsigned char uIndex) const
{
    return mPathSpline.GetControlPoints()[uIndex];
}

// FUNCTION: SWBF2 0x004d6670
bool EntityPathFollower::GetPointIsSet(unsigned char uIndex) const
{
    return (mSetPoints & (1 << uIndex)) > 0;
}

// FUNCTION: SWBF2 0x004d66b0
void EntityPathFollower::RemoveFromSquadron()
{
    mSpeed = static_cast<const EntityPathFollowerClass*>(GetEntitySquadronClass())->mSpeed;
    EntitySquadron::RemoveFromSquadron();
}

// FUNCTION: SWBF2 0x004d2620
void* PblListDoubleSorted::Node::GetObjectA(PblListDoubleSorted const* pList, unsigned int uOffset) const
{
    if ((void const*)this == (void const*)pList)
    {
        return 0;
    }
    return (char*)this - uOffset;
}

namespace
{

    Character* GetFlagOwner(const FlagItem* pFlag)
    {
        return pFlag->mOwner;
    }

    const PblMatrix& GetFlagMatrix(const FlagItem* pFlag)
    {
        return static_cast<const CollisionObject*>(pFlag)->GetMatrix();
    }
}

// FUNCTION: SWBF2 0x004d49a0
bool EntityPathFollower::Update(BranchParams& params, float fElapsed, PblVector3& vVelocity,
                                PblMatrix* const pMatrix, bool b, float fSpeedLimit, float fSpeedHold)
{
    bool bForceBranch = false;
    if (fElapsed == 0.0f)
    {
        return true;
    }

    const PblVector3& vPosition = params.mMatrix.GetTranslation();
    if (mbTeleportNow && InSquadron())
    {
        RemovePathRef(params.mPathClass);
        mSetPoints = 0;
        ResetStateVars();
        BranchParams leader = static_cast<EntityPathFollower*>(GetSquadronLeader())->GetBranchParams();
        const PblMatrix& lead = leader.mMatrix;
        float fSlot = (float)mFormationSlot;
        PblVector3 vPos(lead.m[3][0] + lead.m[0][0] * fSlot * 15.0f, lead.m[3][1] + lead.m[0][1] * fSlot * 15.0f,
                        lead.m[3][2] + lead.m[0][2] * fSlot * 15.0f);
        PblMatrix& matrix = params.mMatrix;
        matrix.m[3][0] = vPos.x;
        matrix.m[3][1] = vPos.y;
        matrix.m[3][2] = vPos.z;
        matrix.m[3][3] = 1.0f;
        static_cast<CollisionObject*>(params.mGameObject)->mLastPosition = vPos;
        matrix.m[2][0] = lead.m[2][0];
        matrix.m[2][1] = lead.m[2][1];
        matrix.m[2][2] = lead.m[2][2];
        matrix.m[2][3] = 0.0f;
        if (params.mGameObject->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            static_cast<EntityFlyer*>(params.mGameObject)->TurnOffContrailEffects();
        }
        bForceBranch = true;
        mbTeleportNow = false;
        mBoostTimer = 10.0f;
    }

    if (mStaredownTimer > 0.0f)
    {
        mStaredownTimer -= fElapsed;
        if (mStaredownTimer < 0.0f)
        {
            bForceBranch = true;
        }
    }

    PickupCTFFlag(params);
    EndOfSegment(params, &vPosition, bForceBranch, false);
    ShotAtByEnemy(params);
    if (mBoostTimer > 0.0f)
    {
        mBoostTimer -= fElapsed;
    }

    if (!mPath && !GetPointIsSet(2) && !GetPointIsSet(3))
    {
        vVelocity.z = 0.0f;
        vVelocity.y = 0.0f;
        vVelocity.x = 0.0f;
        Reset(params.mPathClass);
        return false;
    }

    PblVector3 vOldPosition = vPosition;
    PblVector3 vPoint;
    mPathSpline.CalculatePoint(mT, vPoint);
    PblVector3 vTangent;
    mPathSpline.CalculateFirstDerivative(mT, vTangent);
    float fLength = sqrtf(vTangent.x * vTangent.x + vTangent.y * vTangent.y + vTangent.z * vTangent.z);

    float fSpeed = params.mWorldSpeed;
    if (mbQueuePoint || mbGoMinSpeed)
    {
        fSpeed *= 0.8f;
    }
    if (mBoostTimer > 0.0f)
    {
        fSpeed *= 1.5f;
    }
    float fStep;
    if (fLength == 0.0f)
    {
        fStep = 1.0f;
    }
    else
    {
        fStep = mCurSpeed * fSpeed / fLength;
        if (fStep >= 1.05f)
        {
            fStep = 1.05f;
        }
    }

    float fLookT = params.mPathClass->mPathRollValues.mRollLookAhead * fStep + mT;
    if (fLookT < 0.0f)
    {
        fLookT = 0.0f;
    }
    else if (fLookT > 1.0f)
    {
        fLookT = 1.0f;
    }
    PblVector3 vLookAhead;
    mPathSpline.CalculateFirstDerivative(fLookT, vLookAhead);
    fStep *= fElapsed;
    mT += fStep;

    if (fLength != 0.0f)
    {
        float fInv = 1.0f / fLength;
        vTangent = PblVector3(fInv * vTangent.x, vTangent.y * fInv, vTangent.z * fInv);
    }
    PblVector3 vNormal;
    D3DXVec3Normalize(&vNormal, &vLookAhead);
    vLookAhead = vNormal;

    PblMatrix rotation;
    PblMatrix* pRotation = pMatrix ? pMatrix : &rotation;
    float fTurn = Rotate(params.mPathClass, params.mMatrix, fElapsed, vTangent, vLookAhead, pRotation);
    PblMatrix& matrix = params.mMatrix;
    D3DXVec3Normalize(&vNormal, (const PblVector3*)&matrix.m[0][0]);
    matrix.m[0][0] = vNormal.x;
    matrix.m[0][1] = vNormal.y;
    matrix.m[0][2] = vNormal.z;
    matrix.m[0][3] = 0.0f;
    D3DXVec3Normalize(&vNormal, (const PblVector3*)&matrix.m[1][0]);
    matrix.m[1][0] = vNormal.x;
    matrix.m[1][1] = vNormal.y;
    matrix.m[1][2] = vNormal.z;
    matrix.m[1][3] = 0.0f;
    D3DXVec3Normalize(&vNormal, (const PblVector3*)&matrix.m[2][0]);
    matrix.m[2][0] = vNormal.x;
    matrix.m[2][1] = vNormal.y;
    matrix.m[2][2] = vNormal.z;
    matrix.m[2][3] = 0.0f;

    PblVector3 vTarget = vPoint;
    UpdateSquadron(params.mPathClass, *pRotation, fElapsed, vPosition, vTarget);
    if (mStrafeTimer > 0.0f)
    {
        const PblVector3& vRight = params.mMatrix.GetXAxis();
        float fWeave = PblMath::Sin(mStrafeTimer * 0.5f) + PblMath::Sin(mStrafeTimer);
        vTarget = PblVector3(vTarget.x + vRight.x * fWeave * 4.0f, vTarget.y + vRight.y * fWeave * 4.0f,
                             vTarget.z + vRight.z * fWeave * 4.0f);
        mStrafeTimer -= fElapsed * 4.0f;
    }

    if (InSquadron())
    {
        PblVector3 vDelta(vTarget.x - vOldPosition.x, vTarget.y - vOldPosition.y, vTarget.z - vOldPosition.z);
        D3DXVec3Normalize(&vNormal, &vDelta);
        vTangent = vNormal;
        fTurn = Rotate(params.mPathClass, params.mMatrix, fElapsed, vTangent, vLookAhead, pRotation);
    }

    PblVector3 vNew = vTarget;
    float fDx = vOldPosition.x - vTarget.x;
    float fDy = vOldPosition.y - vTarget.y;
    float fDz = vOldPosition.z - vTarget.z;
    if (fDx * fDx + fDy * fDy + fDz * fDz > 225.0f)
    {
        float fScale = 10.0f / sqrtf(fDx * fDx + fDy * fDy + fDz * fDz);
        vNew = PblVector3((vTarget.x - vOldPosition.x) * fScale + vOldPosition.x,
                          (vTarget.y - vOldPosition.y) * fScale + vOldPosition.y,
                          (vTarget.z - vOldPosition.z) * fScale + vOldPosition.z);
    }
    matrix.m[3][0] = vNew.x;
    matrix.m[3][1] = vNew.y;
    matrix.m[3][2] = vNew.z;
    matrix.m[3][3] = 1.0f;
    if (pMatrix)
    {
        pMatrix->m[3][0] = vNew.x;
        pMatrix->m[3][1] = vNew.y;
        pMatrix->m[3][2] = vNew.z;
        pMatrix->m[3][3] = 1.0f;
    }

    float fInvElapsed = 1.0f / fElapsed;
    vVelocity = PblVector3((vPosition.x - vOldPosition.x) * fInvElapsed, (vPosition.y - vOldPosition.y) * fInvElapsed,
                           (vPosition.z - vOldPosition.z) * fInvElapsed);

    float fTurnRatio = (float)fabs(fTurn / 3.1415927f);
    if (fTurnRatio < 0.0f)
    {
        fTurnRatio = 0.0f;
    }
    else if (fTurnRatio > 1.0f)
    {
        fTurnRatio = 1.0f;
    }
    float fOldSpeed = mCurSpeed;
    float fTurnScale = params.mPathClass->mTurnSpeed * fTurnRatio + (1.0f - fTurnRatio);
    float fPathSpeed = mPath ? mPath->mSpeedFactor : mOffPathSpeed;
    float fNodeSpeed = mPath ? mPath->GetNodeProperties(mIndex).mSpeedFactor : 1.0f;
    float fAccel = mAcceleration * fElapsed;
    float fBlend = fAccel;
    if (fBlend < 0.0f)
    {
        fBlend = 0.0f;
    }
    else if (fBlend > 1.0f)
    {
        fBlend = 1.0f;
    }
    mCurSpeed = (mSpeed * fPathSpeed * fNodeSpeed * fTurnScale - fOldSpeed) * fBlend + fOldSpeed;
    if (fSpeedLimit > 0.0f)
    {
        if (fSpeedLimit >= fOldSpeed && mCurSpeed > fSpeedLimit && fSpeedHold < 0.2f)
        {
            mCurSpeed = fSpeedLimit;
        }
        else if (mCurSpeed > fSpeedLimit && fSpeedHold == 0.0f)
        {
            mCurSpeed = fOldSpeed - fAccel;
            if (fSpeedLimit > mCurSpeed)
            {
                mCurSpeed = fSpeedLimit;
            }
        }
    }

    if (mT < 1.0f)
    {
        return true;
    }

    if (mPath && mIndex <= 0)
    {
        int iIndex = mIndex;
        if (iIndex < mPath->GetNumPoints() && mPath->GetNodeProperties(iIndex).mLandOnArrival)
        {
            mbLandNow = true;
        }
    }

    if (mAttacker)
    {
        PblVector3 vNext;
        GetNextTowardsAttackerPoint(params, vNext);
        mPathSpline.AddPoint(vNext);
        while (mT >= 1.0f)
        {
            mT -= 1.0f;
        }
        mIndex += mDirection;
        mT = fStep;
        mbQueuePoint = false;
        mbGoMinSpeed = true;
        mStrafeTimer = 0.0f;
        mStaredownTimer = 3.5f;
        mAttacker = 0;
        return true;
    }

    if (mbCirclingPoint)
    {
        CirclePoint(35.0f);
        return true;
    }

    while (mT >= 1.0f)
    {
        mT -= 1.0f;
    }
    if (!mbDelayIndexIncrement)
    {
        mIndex += mDirection;
    }
    else
    {
        if (mbStraightLine)
        {
            mbStraightLine = false;
            SmoothPath(params, &params.mMatrix.GetTranslation(), 20.0f, 125.0f, 300.0f, 25.0f, 0.5f);
        }
        mbDelayIndexIncrement = false;
    }
    EndOfSegment(params, &params.mMatrix.GetTranslation(), mPath == 0, true);
    return true;
}

// FUNCTION: SWBF2 0x004d54c0
void EntityPathFollower::UpdateSquadron(const EntityPathFollowerClass* const pClass, const PblMatrix& matrix,
                                        float fElapsed, const PblVector3& vPosition, PblVector3& vPoint)
{
    UpdatePosition(pClass, fElapsed, matrix, vPoint);
    if (!InSquadron() || IsSquadronLeader())
    {
        return;
    }

    EntityPathFollower* pLeader = static_cast<EntityPathFollower*>(GetSquadronLeader());
    float fFactor;
    if (mPath != pLeader->mPath)
    {
        fFactor = 3.0f;
    }
    else if (mIndex == pLeader->mIndex)
    {
        fFactor = mT - pLeader->mT < 0.0f ? 2.0f : 0.3f;
    }
    else if (mIndex * mDirection < pLeader->mIndex * pLeader->mDirection)
    {
        fFactor = 2.5f;
    }
    else
    {
        fFactor = 0.1f;
    }

    float fWorldSpeed = GetBranchParams().mWorldSpeed;
    if (fWorldSpeed == 0.0f)
    {
        fWorldSpeed = 1.0f;
    }
    float fLeaderSpeed = pLeader->mCurSpeed;
    mSpeed = fLeaderSpeed * fFactor * pLeader->GetBranchParams().mWorldSpeed / fWorldSpeed;
}

// FUNCTION: SWBF2 0x004d5630
bool EntityPathFollower::ShotAtByEnemy(BranchParams& params)
{
    if (!params.mPathClass->mbFlyerVsFlyer)
    {
        return false;
    }
    if (!mAttacker || !mAttacker->GetControllable())
    {
        return false;
    }
    if (mbQueuePoint || mbGoMinSpeed)
    {
        return false;
    }

    PblVector3 vStart;
    int iPrev = mIndex - mDirection;
    if (mPath && iPrev >= 0 && iPrev < mPath->GetNumPoints())
    {
        mPath->CalculatePointPosition(iPrev, mDirection, vStart);
    }
    else
    {
        const PblMatrix& m = params.mMatrix;
        vStart = PblVector3(m.m[3][0] - m.m[2][0] * 20.0f - mOffset.x, m.m[3][1] - m.m[2][1] * 20.0f - mOffset.y,
                            m.m[3][2] - m.m[2][2] * 20.0f - mOffset.z);
    }
    Reset(params.mPathClass);

    PblVector3 vAttacker = mAttacker->GetControllable()->GetMatrix().GetTranslation();
    const PblVector3& vPosition = params.mMatrix.GetTranslation();
    float fDist = sqrtf((vPosition.x - vAttacker.x) * (vPosition.x - vAttacker.x) +
                        (vPosition.y - vAttacker.y) * (vPosition.y - vAttacker.y) +
                        (vPosition.z - vAttacker.z) * (vPosition.z - vAttacker.z));
    float fRight = PblRandom::GetGlobalFloatRange(0.0f, 2.0f);
    float fUp = PblRandom::GetGlobalFloatRange(0.0f, 2.0f);
    const PblMatrix& attacker = mAttacker->GetControllable()->GetMatrix();
    PblVector3 vAhead(attacker.m[2][0] * fDist * 0.5f, attacker.m[2][1] * fDist * 0.5f,
                      attacker.m[2][2] * fDist * 0.5f);
    const PblMatrix& up = mAttacker->GetControllable()->GetMatrix();
    PblVector3 vUp(up.m[1][0] * fUp, up.m[1][1] * fUp, up.m[1][2] * fUp);
    const PblMatrix& right = mAttacker->GetControllable()->GetMatrix();
    PblVector3 vOffset(right.m[0][0] * fRight + vUp.x, right.m[0][1] * fRight + vUp.y,
                       right.m[0][2] * fRight + vUp.z);
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vOffset);
    PblVector3 vTarget(vDir.x * 30.0f + vAttacker.x + vAhead.x, vDir.y * 30.0f + vAttacker.y + vAhead.y,
                       vDir.z * 30.0f + vAttacker.z + vAhead.z);

    SavePoints(&params.mMatrix.GetTranslation(), 0, true);
    SetPoint(vStart, 0);
    SetPoint(vTarget, 2);
    SetPoint(vTarget, 3);
    SmoothPath(params, &params.mMatrix.GetTranslation(), 10.0f, 180.0f, 300.0f, 100.0f, 0.923f);
    mbQueuePoint = true;
    mbGoMinSpeed = false;
    return true;
}

// FUNCTION: SWBF2 0x004d59b0
static PblVector3 CalculateZeroPoint(const PblVector3& vPosition, const PblVector3& vDirection,
                                     const PblVector3& vTarget)
{
    float fDist = sqrtf((vTarget.x - vPosition.x) * (vTarget.x - vPosition.x) +
                        (vTarget.y - vPosition.y) * (vTarget.y - vPosition.y) +
                        (vTarget.z - vPosition.z) * (vTarget.z - vPosition.z));
    return PblVector3(vTarget.x - vDirection.x * fDist * 5.0f, vTarget.y - vDirection.y * fDist * 5.0f,
                      vTarget.z - vDirection.z * fDist * 5.0f);
}

// FUNCTION: SWBF2 0x004d5a70
void EntityPathFollower::PickupCTFFlag(BranchParams& params)
{
    if (FlagItem::sFlagGameplayType != FlagItem::FLAG_GAMEPLAY_1FLAG || !GetPath())
    {
        return;
    }

    for (PblList<FlagItem>::Itor it(*FlagItem::sList); (FlagItem*)it; ++it)
    {
        FlagItem* pFlag = it;
        if (GetFlagOwner(pFlag) || pFlag->IsInLandingZone())
        {
            continue;
        }
        const PblVector3& vFlag = GetFlagMatrix(pFlag).GetTranslation();
        const PblVector3& vForward = GetMatrix().GetZAxis();
        PblVector3 vPosition = GetMatrix().GetTranslation();
        PblVector3 vAhead(vPosition.x + vForward.x * 300.0f * 0.5f, vPosition.y + vForward.y * 300.0f * 0.5f,
                          vPosition.z + vForward.z * 300.0f * 0.5f);
        if ((vFlag.x - vAhead.x) * (vFlag.x - vAhead.x) + (vFlag.y - vAhead.y) * (vFlag.y - vAhead.y) +
                (vFlag.z - vAhead.z) * (vFlag.z - vAhead.z) >=
            90000.0f)
        {
            continue;
        }
        const GameObject* pIgnore = params.mGameObject;
        PblVector3 vEnd(vFlag.x + (vFlag.x - vPosition.x), vFlag.y + (vFlag.y - vPosition.y),
                        vFlag.z + (vFlag.z - vPosition.z));
        if (!CollisionManager::RayTest(vPosition, vEnd, &pIgnore, 1, -2))
        {
            continue;
        }

        Reset(params.mPathClass);
        SetPoint(CalculateZeroPoint(vPosition, vForward, vFlag), 0);
        SetPoint(vPosition, 1);
        SetPoint(vFlag, 2);
        PblVector3 vDelta(vFlag.x - vPosition.x, vFlag.y - vPosition.y, vFlag.z - vPosition.z);
        float fLength = sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
        float fInv = 1.0f / fLength;
        float fBeyond = fLength * 3.0f;
        if (!(200.0f > fBeyond))
        {
            fBeyond = 200.0f;
        }
        PblVector3 vBeyond(vFlag.x + vDelta.x * fInv * fBeyond, vFlag.y + vDelta.y * fInv * fBeyond,
                           vFlag.z + vDelta.z * fInv * fBeyond);
        SetPoint(vBeyond, 3);
        return;
    }
}

// FUNCTION: SWBF2 0x004d5d30
bool EntityPathFollower::CirclePoint(float fRadius)
{
    if (!mbCirclingPoint)
    {
        return false;
    }

    BranchParams params = GetBranchParams();
    PblVector3 vPosition = params.mMatrix.GetTranslation();
    PblVector3 vToCenter(mPointToCircle.x - vPosition.x, 0.0f, mPointToCircle.z - vPosition.z);
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vToCenter);
    PblVector3 vSideDir(vDir.z, 0.0f, -vDir.x);
    const PblVector3& vForward = params.mMatrix.GetZAxis();
    float fCross = vDir.z * vForward.x - vDir.x * vForward.z;
    if (fCross < 0.0f)
    {
        vSideDir = PblVector3(-vSideDir.x, -vSideDir.y, -vSideDir.z);
    }
    PblVector3 vSide(mPointToCircle.x + vSideDir.x * fRadius, mPointToCircle.y + vSideDir.y * fRadius,
                     mPointToCircle.z + vSideDir.z * fRadius);
    PblVector3 vNext(mPointToCircle.x + vDir.x * fRadius, mPointToCircle.y + vDir.y * fRadius,
                     mPointToCircle.z + vDir.z * fRadius);
    if (GetPointIsSet(3))
    {
        const PblVector3& vLast = GetPoint(3);
        PblVector3 vFromCenter(vLast.x - mPointToCircle.x, 0.0f, vLast.z - mPointToCircle.z);
        D3DXVec3Normalize(&vDir, &vFromCenter);
        vDir = PblVector3(vDir.z, 0.0f, -vDir.x);
        if (fCross > 0.0f)
        {
            vDir = PblVector3(-vDir.x, -vDir.y, -vDir.z);
        }
        vNext = PblVector3(mPointToCircle.x + vDir.x * fRadius, mPointToCircle.y + vDir.y * fRadius,
                           mPointToCircle.z + vDir.z * fRadius);
    }

    const GameObject* pIgnore = params.mGameObject;
    CollisionObject* pHit = 0;
    PblVector3 vDown(vNext.x - 0.0f, vNext.y - 100.0f, vNext.z - 0.0f);
    float fDrop = CollisionManager::RayHit(vNext, vDown, pHit, 0, &pIgnore, 1, 0x9a, false) * 100.0f;
    if (!pHit)
    {
        fDrop = -5.0f;
    }
    vSide.y = vSide.y - fDrop + 5.0f;
    vNext.y = vNext.y - fDrop + 5.0f;

    if (!GetPointIsSet(1))
    {
        SavePoints(&vPosition, 0, true);
        SetPoint(vSide, 2);
        SetPoint(vNext, 3);
        SmoothPath(params, &vPosition, 25.0f, 35.0f, 100.0f, 10.0f, 1.0f);
        return true;
    }

    if (!RayTestSpline(GetPoint(1), GetPoint(2), GetPoint(3), vNext, params.mGameObject, 2))
    {
        if (!(fRadius > 20.0f) || !GetPointIsSet(3))
        {
            mbCirclingPoint = false;
            return false;
        }
        float fCloser = fRadius - 10.0f;
        vNext = PblVector3(fCloser * vDir.x + mPointToCircle.x, mPointToCircle.y + vDir.y * fCloser,
                           mPointToCircle.z + vDir.z * fCloser);
    }
    mPathSpline.AddPoint(vNext);
    mT -= 1.0f;
    return true;
}

// FUNCTION: SWBF2 0x004d61d0
void EntityPathFollower::GetNextTowardsAttackerPoint(BranchParams& params, PblVector3& vPoint)
{
    if (mAttacker->GetControllable())
    {
        const PblMatrix& attacker = mAttacker->GetControllable()->GetMatrix();
        PblVector3 vAhead(attacker.m[2][0] * 30.0f, attacker.m[2][1] * 30.0f, attacker.m[2][2] * 30.0f);
        float fUp = PblRandom::GetGlobalFloatRange(0.0f, 2.0f);
        float fRight = PblRandom::GetGlobalFloatRange(0.0f, 2.0f);
        PblVector3 vOffset(attacker.m[0][0] * fRight + attacker.m[1][0] * fUp,
                           attacker.m[0][1] * fRight + attacker.m[1][1] * fUp,
                           attacker.m[0][2] * fRight + attacker.m[1][2] * fUp);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vOffset);
        const PblVector3& vPosition = params.mMatrix.GetTranslation();
        vPoint = PblVector3(
            (vPosition.x + attacker.m[3][0]) * 0.5f + vDir.x * 20.0f + vAhead.x - attacker.m[2][0] * 50.0f,
            (vPosition.y + attacker.m[3][1]) * 0.5f + vDir.y * 20.0f + vAhead.y - attacker.m[2][1] * 50.0f,
            (vPosition.z + attacker.m[3][2]) * 0.5f + vDir.z * 20.0f + vAhead.z - attacker.m[2][2] * 50.0f);
    }
    else
    {
        vPoint = GetPoint(3);
        mAttacker = 0;
    }
}

extern "C" PblMatrix* __stdcall D3DXMatrixRotationZ(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

// FUNCTION: SWBF2 0x004d64a0
bool EntityPathFollower::SetupNextSegment(const PblVector3* const pPosition, BranchParams& params, bool b1, bool b2)
{
    if (mPath->mPathRoll)
    {
        PblQuaternion qIdentity;
        int iPrev = mIndex - mDirection;
        const PblQuaternion& qPrev =
            iPrev >= 0 && iPrev < mPath->GetNumPoints() ? mPath->GetPointRotation(iPrev) : qIdentity;
        int iIndex = mIndex;
        const PblQuaternion& qNext =
            iIndex >= 0 && iIndex < mPath->GetNumPoints() ? mPath->GetPointRotation(iIndex) : qPrev;

        PblVector3 vPrevRight(1.0f, 0.0f, 0.0f);
        qPrev.RotateVector(vPrevRight);
        mCurRoll = PblMath::Atan2(vPrevRight.y, vPrevRight.x);
        PblVector3 vNextRight(1.0f, 0.0f, 0.0f);
        qNext.RotateVector(vNextRight);
        mSplineRoll = PblMath::Atan2(vNextRight.y, vNextRight.x);
    }
    return SavePoints(pPosition, b1 ? &params : 0, b2);
}

// FUNCTION: SWBF2 0x004d66d0
bool EntityPathFollower::SavePoints(const PblVector3* const pPosition, BranchParams* const pParams, bool b)
{
    unsigned char uKeep = 0xff;
    int iNumPoints;
    int iNext;
    if (mPath)
    {
        iNumPoints = mPath->GetNumPoints();
        iNext = mIndex + mDirection;
        if ((iNext < 0 || iNext >= iNumPoints) && pParams)
        {
            return Branch(*pParams, pPosition, b);
        }
    }
    else
    {
        iNumPoints = 0;
        iNext = 0;
    }

    bool bZeroPoint = false;
    PblVector3 vPoint;
    if (b)
    {
        int iPrev;
        if (GetPointIsSet(0) && (iPrev = mIndex - mDirection * 2, mPath) && iPrev >= 0 && iPrev < iNumPoints)
        {
            mPath->CalculatePointPosition(iPrev, mDirection, vPoint);
            const PblVector3& vOld = GetPoint(0);
            if (250.0f > (vOld.x - vPoint.x) * (vOld.x - vPoint.x) + (vOld.y - vPoint.y) * (vOld.y - vPoint.y) +
                             (vOld.z - vPoint.z) * (vOld.z - vPoint.z) &&
                GetPointIsSet(1))
            {
                SetPoint(GetPoint(1), 0);
            }
            else
            {
                SetPoint(vPoint, 0);
            }
        }
        else
        {
            bZeroPoint = true;
        }
        SetPoint(PblVector3(pPosition->x - mOffset.x, pPosition->y - mOffset.y, pPosition->z - mOffset.z), 1);
        mT = 0.005f;
    }
    else
    {
        SetPoint(GetPointIsSet(1) ? GetPoint(1) : *pPosition, 0);
        SetPoint(GetPointIsSet(2) ? GetPoint(2) : *pPosition, 1);
    }

    if (GetPointIsSet(3))
    {
        SetPoint(GetPoint(3), 2);
    }
    else
    {
        if (mPath && mIndex >= 0 && mIndex < iNumPoints)
        {
            mPath->CalculatePointPosition(mIndex, mDirection, vPoint);
        }
        else
        {
            vPoint = GetPoint(1);
            uKeep &= ~(1 << 2);
        }
        SetPoint(vPoint, 2);
    }

    if (bZeroPoint)
    {
        PblVector3 vDirection;
        if (pParams)
        {
            vDirection = pParams->mMatrix.GetZAxis();
        }
        else
        {
            BranchParams params = GetBranchParams();
            const PblMatrix& matrix =
                params.mGameObject ? static_cast<CollisionObject*>(params.mGameObject)->GetMatrix() : params.mMatrix;
            vDirection = matrix.GetZAxis();
        }
        SetPoint(CalculateZeroPoint(GetPoint(1), vDirection, GetPoint(2)), 0);
    }

    if (mPath && iNext >= 0 && iNext < iNumPoints)
    {
        mPath->CalculatePointPosition(iNext, mDirection, vPoint);
    }
    else
    {
        vPoint = GetPoint(2);
        uKeep &= ~(1 << 3);
    }
    SetPoint(vPoint, 3);
    mSetPoints &= uKeep;
    return false;
}

// FUNCTION: SWBF2 0x004d6a10
float EntityPathFollower::Roll(const EntityPathFollowerClass* const pClass, float fYaw, float fElapsed,
                               float fTurnRate)
{
    if (mPath && mPath->mPathRoll)
    {
        return (mSplineRoll - mCurRoll) * mT + mCurRoll;
    }

    float fBank = mCurSpeed * fTurnRate * pClass->mPathRollValues.mRollSpeedFactor;
    if (-1.5707964f > fBank)
    {
        fBank = -1.5707964f;
    }
    else if (fBank > 1.5707964f)
    {
        fBank = 1.5707964f;
    }

    float fWeave = 1.0f;
    float fTime = GameLoop::GetMissionTime() + (float)(((unsigned int)this >> 2) & 0xf);
    if (!AIUtil::gGroundFlyerMap)
    {
        fWeave = 2.0f;
    }
    float fTarget = PblMath::Sin(fTime * 0.43f) *
                        (PblMath::Cos(fTime * 2.33f) * (PblMath::Sin(fTime * 1.17f) * fWeave)) +
                    fBank;

    float fFactor = pClass->mPathRollValues.mRollTurnFactor * fElapsed;
    if (0.0f > fFactor)
    {
        fFactor = 0.0f;
    }
    else if (fFactor > 1.0f)
    {
        fFactor = 1.0f;
    }
    mSplineRoll = (fTarget - mSplineRoll) * fFactor + mSplineRoll;

    float fDelta = mSplineRoll - mCurRoll;
    float fAccel;
    if (((float)fabs(mRollVelocity) / pClass->mPathRollValues.mRollSpeed + 1.0f) *
            pClass->mPathRollValues.mRollTolerance >
        (float)fabs(fDelta))
    {
        fAccel = -((0.0f > mRollVelocity ? -1.0f : 1.0f) * pClass->mPathRollValues.mRollAcceleration);
    }
    else
    {
        fAccel = (0.0f > fDelta ? -1.0f : 1.0f) * pClass->mPathRollValues.mRollAcceleration;
    }

    float fResp = pClass->mPathRollValues.mRollAccelResp * fElapsed;
    if (0.0f > fResp)
    {
        fResp = 0.0f;
    }
    else if (fResp > 1.0f)
    {
        fResp = 1.0f;
    }
    mCurRollAcceleration = (fAccel - mCurRollAcceleration) * fResp + mCurRollAcceleration;

    float fSpeed = pClass->mPathRollValues.mRollSpeed;
    float fVelocity = mCurRollAcceleration * fElapsed + mRollVelocity;
    if (-fSpeed > fVelocity)
    {
        fVelocity = -fSpeed;
    }
    else if (fVelocity > fSpeed)
    {
        fVelocity = fSpeed;
    }
    mRollVelocity = fVelocity;
    mCurRoll = mRollVelocity * fElapsed + mCurRoll;

    float fDamp = pClass->mPathRollValues.mRollDampening * fElapsed;
    if (0.0f > fDamp)
    {
        fDamp = 0.0f;
    }
    else if (fDamp > 1.0f)
    {
        fDamp = 1.0f;
    }
    mYaw = fYaw;
    mCurRoll = (fTarget - mCurRoll) * fDamp + mCurRoll;
    return mCurRoll;
}

// FUNCTION: SWBF2 0x004d6cc0
float EntityPathFollower::Rotate(const EntityPathFollowerClass* const pClass, PblMatrix& matrix, float fElapsed,
                                 const PblVector3& vDirection, const PblVector3& vLookAhead, PblMatrix* const pMatrix)
{
    if (mYaw == FLT_MAX)
    {
        mYaw = PblMath::Atan2(matrix.m[2][0], matrix.m[2][2]);
    }
    EntityPath::BuildMatrix(vDirection, matrix, pMatrix);

    float fYaw;
    if (vLookAhead.x != 0.0f && vLookAhead.z != 0.0f)
    {
        fYaw = PblMath::Atan2(vLookAhead.x, vLookAhead.z);
    }
    else
    {
        fYaw = mYaw;
    }
    float fTurnRate = (mYaw - fYaw) / fElapsed;

    PblMatrix roll;
    D3DXMatrixRotationZ(&roll, Roll(pClass, fYaw, fElapsed, fTurnRate));
    PblMatrix result;
    D3DXMatrixMultiply(&result, &roll, &matrix);
    matrix = result;
    return fTurnRate;
}

// FUNCTION: SWBF2 0x004d6df0
bool EntityPathFollower::CalculateDistance(const PblVector3& vPosition, const EntityPath* const pPath,
                                           unsigned int uIndex, float fMinDist, PathAttributes& attrib,
                                           float& fBestSquareDist)
{
    PblVector3 vPoint = *pPath->GetPointPosition(uIndex);
    PblVector3 vDist(vPoint.x - vPosition.x, vPoint.y - vPosition.y, vPoint.z - vPosition.z);
    attrib.mPath = pPath;
    attrib.mPointIndex = uIndex;
    attrib.mVectorDist = vDist;
    attrib.mSquareDist = vDist.x * vDist.x + vDist.y * vDist.y + vDist.z * vDist.z;
    if (fBestSquareDist > attrib.mSquareDist && attrib.mSquareDist > fMinDist * fMinDist)
    {
        fBestSquareDist = attrib.mSquareDist;
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x004d6eb0
void EntityPathFollower::FindPath(const PblVector3* const pPosition, const PblVector3* const pDirection, float fRange,
                                  unsigned int uBranchPaths, float fCosMaxAngle, float fMinBranchDist,
                                  unsigned int uClassID, const PblVector3* const pExclude,
                                  const EntityPath* const pExcludePath, const GameObject* const pIgnore,
                                  bool bFallbackToClosest, PathStart& start)
{
    const PblList<EntityPath>& paths = EntityPath::GetPathList();
    RedStack stack;
    unsigned int uMaxStarts = paths.GetCount() * 2;
    PathAttributes* pAttribs = (PathAttributes*)stack.Alloc(uMaxStarts * sizeof(PathAttributes));
    PathStart* pStarts = (PathStart*)stack.Alloc(uMaxStarts * sizeof(PathStart));
    for (unsigned int i = 0; i < uMaxStarts; ++i)
    {
        new (&pAttribs[i]) PathAttributes;
    }
    for (unsigned int i = 0; i < uMaxStarts; ++i)
    {
        new (&pStarts[i]) PathStart;
    }
    PblListDoubleSorted list;

    pAttribs[0].mPath = 0;
    pAttribs[0].mPointIndex = 0;
    pAttribs[0].mSquareDist = 0.0f;
    pAttribs[0].mVectorDist = PblVector3(0.0f, 0.0f, 0.0f);

    unsigned int uNumAttribs = 0;
    unsigned int uClosest = 0;
    float fClosestSquareDist = FLT_MAX;
    PathAttributes* pAttrib = pAttribs;
    for (PblList<EntityPath>::Itor it(paths); (EntityPath*)it; ++it)
    {
        EntityPath* pPath = it;
        if (uClassID && !pPath->ClassIsSupported(uClassID, true))
        {
            continue;
        }
        if (!pPath->IsEnabled())
        {
            continue;
        }

        if (pExclude != pPath->GetPointPosition(0) && pExcludePath != pPath && pPath->GetDirection() >= 0)
        {
            if (CalculateDistance(*pPosition, pPath, 0, fMinBranchDist, *pAttrib, fClosestSquareDist))
            {
                uClosest = uNumAttribs;
            }
            ++uNumAttribs;
            ++pAttrib;
        }
        int iLast = pPath->GetNumPoints() - 1;
        if (pExclude != pPath->GetPointPosition(iLast) && pExcludePath != pPath && pPath->GetDirection() <= 0)
        {
            if (CalculateDistance(*pPosition, pPath, iLast, fMinBranchDist, *pAttrib, fClosestSquareDist))
            {
                uClosest = uNumAttribs;
            }
            ++uNumAttribs;
            ++pAttrib;
        }
    }

    if (!(FLT_MAX > fRange) && uBranchPaths <= 1 && !(fCosMaxAngle > -1.0f))
    {
        start.mNode.mSortValue = 1.0f;
        start.mAttrib = pAttribs[uClosest];
        start.mDirection = pAttribs[uClosest].mPointIndex == 0 ? 1 : -1;
        return;
    }

    PathStart best;
    best.mAttrib = pAttribs[uClosest];
    best.mDirection = best.mAttrib.mPointIndex == 0 ? 1 : -1;
    float fBestHeading = 0.0f;
    unsigned int uNumStarts = 0;
    float fMinSquareDist = fMinBranchDist * fMinBranchDist;
    for (unsigned int i = 0; i < uNumAttribs; ++i)
    {
        const PathAttributes& attrib = pAttribs[i];
        char cDirection = attrib.mPointIndex == 0 ? 1 : -1;
        float fSuitability =
            attrib.mPath->CalculateSuitability(pPosition, pDirection, fCosMaxAngle, fRange, attrib.mPointIndex,
                                               cDirection, &attrib.mVectorDist, attrib.mSquareDist);
        float fHeading = attrib.mPath->CalculateSuitability(pPosition, pDirection, -1.0f, fRange, attrib.mPointIndex,
                                                            cDirection, &attrib.mVectorDist, attrib.mSquareDist);
        if (fMinSquareDist > attrib.mVectorDist.x * attrib.mVectorDist.x + attrib.mVectorDist.y * attrib.mVectorDist.y +
                                 attrib.mVectorDist.z * attrib.mVectorDist.z)
        {
            fHeading *= 0.4f;
            fSuitability *= 0.4f;
        }
        float fChance = attrib.mPath->mFrequency * fSuitability;
        if (0.0f > fChance)
        {
            fChance = 0.0f;
        }
        else if (fChance > 1.0f)
        {
            fChance = 1.0f;
        }

        if (fRange >= attrib.mSquareDist && fChance != 0.0f)
        {
            PathStart& candidate = pStarts[uNumStarts++];
            candidate.mAttrib = attrib;
            candidate.mDirection = cDirection;
            candidate.mNode.mSortValue = fChance;
            list.Add(&candidate.mNode);
        }
        else if (uNumStarts < 4 && fHeading > fBestHeading)
        {
            PblVector3 vNext = *attrib.mPath->GetPointPosition(attrib.mPointIndex + cDirection);
            PblVector3 vPoint = *attrib.mPath->GetPointPosition(attrib.mPointIndex);
            PblVector3 vBehind(pPosition->x - pDirection->x * 20.0f, pPosition->y - pDirection->y * 20.0f,
                               pPosition->z - pDirection->z * 20.0f);
            if (RayTestSpline(vBehind, *pPosition, vPoint, vNext, pIgnore, 2))
            {
                fBestHeading = fHeading;
                best.mNode.mSortValue = fChance;
                best.mAttrib = attrib;
                best.mDirection = cDirection;
            }
        }
    }

    if (uNumStarts)
    {
        bool bFound = false;
        int iTries = 4;
        while (iTries > 0)
        {
            --iTries;
            float fRandom = PblRandom::GetGlobalFloat();
            int iPick = (int)((int)(uBranchPaths > uNumStarts ? uNumStarts : uBranchPaths) * fRandom);
            PblListDouble::Node* pLink = list.mTerminator.mNext;
            for (; pLink != &list.mTerminator; pLink = pLink->mNext, --iPick)
            {
                if ((PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode)) ==
                        list.GetTerminator() ||
                    !iPick)
                {
                    break;
                }
            }
            PblListDoubleSorted::Node* pNode = (PblListDoubleSorted::Node*)list.GetTerminator();
            if (pLink != &list.mTerminator)
            {
                pNode = (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
            }
            start = *(PathStart*)pNode->GetObjectA(&list, offsetof(PathStart, mNode));

            PblVector3 vPoint = *start.mAttrib.mPath->GetPointPosition(start.mAttrib.mPointIndex);
            PblVector3 vNext = *start.mAttrib.mPath->GetPointPosition(start.mAttrib.mPointIndex + start.mDirection);
            PblVector3 vBehind(pPosition->x - pDirection->x * 20.0f, pPosition->y - pDirection->y * 20.0f,
                               pPosition->z - pDirection->z * 20.0f);
            if (RayTestSpline(vBehind, *pPosition, vPoint, vNext, pIgnore, 3))
            {
                bFound = true;
                break;
            }
            pNode->mNode.Remove();
            if (--uNumStarts == 0)
            {
                break;
            }
        }
        if (!bFound)
        {
            start = best;
        }
        while (list.mTerminator.mNext != &list.mTerminator)
        {
            list.mTerminator.mNext->Remove();
        }
    }
    else if (bFallbackToClosest)
    {
        start = best;
    }
    else
    {

        PathAttributes none;
        none.mPath = 0;
        none.mPointIndex = 0;
        none.mSquareDist = 0.0f;
        start.mAttrib = none;
        start.mDirection = 0;
        start.mNode.mSortValue = 0.0f;
    }
}

// FUNCTION: SWBF2 0x004d77c0
bool EntityPathFollower::RayTestSpline(const PblVector3& p0, const PblVector3& p1, const PblVector3& p2,
                                       const PblVector3& p3, const GameObject* const pIgnore, int iSteps)
{
    PblCatmullRom spline;
    spline.Init(p0, p1, p2, p3);
    PblVector3 vLast = p1;
    const GameObject* pIgnoreList = pIgnore;
    float fStep = 1.0f / (float)iSteps;
    for (float t = fStep; 1.0f >= t; t += fStep)
    {
        PblVector3 vPoint;
        spline.CalculatePoint(t, vPoint);
        CollisionObject* pHit = 0;
        CollisionManager::RayHit(vLast, vPoint, pHit, 0, &pIgnoreList, 1, 0, false);
        if (pHit && pHit->GetCollisionModel())
        {
            const CollisionModel* pModel = pHit->GetCollisionModel();
            if (!pModel->GetCollisionMask(COLL_SOFT).IsEmpty() || !pModel->GetCollisionMask(COLL_RIGID).IsEmpty())
            {
                return false;
            }
        }
        vLast = vPoint;
    }
    return true;
}

// FUNCTION: SWBF2 0x004fc6d0
int __fastcall ReturnEdxValue(int, int value)
{
    return value;
}
