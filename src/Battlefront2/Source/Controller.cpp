#include "Controller.h"
#include "AIDifficulty.h"
#include "AIUtil.h"
#include "Controllable.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PathFinder.h"
#include "Trackable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"

#include <math.h>

// FUNCTION: SWBF2 0x00485a50
void Controller::SetRegionVisFactor(float fCrouch, float fStand)
{
    mRegionVisCrouch = fCrouch;
    mRegionVisStand = fStand;
    mRegionVisTime = GameLoop::GetMissionTime();
}

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// GLOBAL: SWBF2 0x01eb97a8
// SYNTHETIC: SWBF2 0x00402790
// `dynamic initializer for 'Controller::rttiHashController''
PblHash Controller::rttiHashController("Controller");
char Controller::rttiNameController[] = "Controller";

static Trackable* GetTrackable(Controllable* pControllable)
{
    return pControllable;
}

static int GetTeam(const GameObject* pObject)
{
    return pObject->mTeam;
}

// FUNCTION: SWBF2 0x00485100
bool Controller::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashController;
}

// FUNCTION: SWBF2 0x00485120
unsigned int Controller::GetDerivedRtti() const
{
    return rttiHashController;
}

// FUNCTION: SWBF2 0x00485130
const char* Controller::GetDerivedRttiName() const
{
    return rttiNameController;
}

// FUNCTION: SWBF2 0x00485150
void Controller::KillController()
{
}

Controller::~Controller()
{
}

// SYNTHETIC: SWBF2 0x00485160
// Controller::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00485190
Controller::Controller(Controllable* pOwner)
    : mOwner(pOwner),
      mTargetBubbleLastVel(0.0f, 0.0f, 0.0f),
      mTargetBubbleSize(0.0f),
      mAvailableBubbleCharge(0.0f),
      mTargetBubbleOffset(0.0f, 0.0f, 0.0f),
      mOpenRadius(0.0f),
      mOpenRadiusTime(-100.0f),
      mFoliageVisCrouch(1.0f),
      mFoliageVisStand(1.0f),
      mRegionVisCrouch(1.0f),
      mRegionVisStand(1.0f),
      mTotalVisFactor(1.0f),
      mForceAudible(false),
      mCrowdTestTimer(-100.0f),
      mCrowdEnemies(0),
      mCrowdAllies(0)
{
    mTargetBubbleRotation._fX = 0.0f;
    mTargetBubbleRotation._fY = 0.0f;
    mTargetBubbleRotation._fZ = 0.0f;
    mTargetBubbleRotation._fS = 1.0f;
}

// FUNCTION: SWBF2 0x004856d0
PblVector3 Controller::GetTargetBubbleCircle() const
{
    PblVector3 vOffset = mTargetBubbleOffset;
    vOffset.z = 0.0f;
    PblVector3 vCircle;
    D3DXVec3Normalize(&vCircle, &vOffset);
    return vCircle;
}

// FUNCTION: SWBF2 0x00485710
void Controller::IncreaseTargetBubbleSize(AI::TargetBubbleIncreaseType eType)
{
    switch (eType)
    {
    case AI::TBI_ROLL:
    case AI::TBI_SPRINT:
        IncreaseTargetBubbleSize(2.0f);
        break;
    case AI::TBI_JUMP:
        IncreaseTargetBubbleSize(1.0f);
        break;
    case AI::TBI_JETJUMP:
    case AI::TBI_FLYERTRICK:
        IncreaseTargetBubbleSize(5.0f);
        break;
    }
}

// FUNCTION: SWBF2 0x00485770
void Controller::IncreaseTargetBubbleSize(float fIncrease)
{
    if (fIncrease > 0.0f)
    {
        if (fIncrease > mAvailableBubbleCharge)
        {
            fIncrease = mAvailableBubbleCharge;
        }
        mAvailableBubbleCharge -= fIncrease;
    }
    mTargetBubbleSize += fIncrease;

    float fMin = 1.5f;
    if (mOwner)
    {
        Trackable* pTrackable = GetTrackable(mOwner);
        fMin = AIDifficulty::GetMinBubbleSize(
            !(pTrackable->GetGameObject() && AIUtil::IsPlayerTeam(GetTeam(pTrackable->GetGameObject()))));
    }

    if (mTargetBubbleSize < fMin)
    {
        mTargetBubbleSize = fMin;
    }
    else if (mTargetBubbleSize > 2.0f)
    {
        mTargetBubbleSize = 2.0f;
    }
}

// FUNCTION: SWBF2 0x00485520
float Controller::_GetTargetBubbleSize() const
{
    const GameObject* pObject = mOwner ? ((const Controllable*)mOwner)->GetGameObject() : 0;

    float fStance = 1.0f;
    float fMaxSize = 1.5f;
    if (mOwner)
    {
        fStance = mOwner->GetTargetBubbleStanceModifier();
        bool bEnemy = !(pObject ? AIUtil::IsPlayerTeam(GetTeam(pObject)) : true);
        if (pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            fMaxSize = AIDifficulty::GetMaxBubbleSize_Flyer(bEnemy);
        }
        else
        {
            fMaxSize = AIDifficulty::GetMaxBubbleSize(bEnemy);
        }
    }

    const PblVector3& vVelocity = pObject->GetVelocity();
    float fSpeed = sqrtf(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y
                         + vVelocity.z * vVelocity.z);
    float fMaxSpeed = pObject->GetMaxSpeed();
    if (fMaxSpeed < 0.1f)
    {
        fMaxSpeed = 0.1f;
    }
    float fSpeedFactor = (fSpeed / fMaxSpeed * 0.6f + 1.0f) * fStance;

    float fRadius = AIUtil::GetCollisionRadius(pObject) * 2.0f;
    if (pObject->GetEntityType() == ENT_HOVER)
    {
        fRadius *= 0.4f;
    }
    if (pObject->GetEntityType() == ENT_FLYER)
    {
        fRadius *= 2.0f;
    }

    float fSize = mTargetBubbleSize * fSpeedFactor;
    if (fSize > fMaxSize)
    {
        fSize = fMaxSize;
    }
    return (3.0f - mTotalVisFactor * 2.0f) * fRadius * fSize;
}

// FUNCTION: SWBF2 0x00485250
void Controller::UpdateTargetBubbleSize(float dt)
{
    if (!mOwner)
    {
        return;
    }

    float fShrink = AIDifficulty::GetTargetBubbleShrinkRate(
                        !AIUtil::IsPlayerTeam(GetTeam(mOwner->GetGameObject()))) * dt;
    float fIncrease = fShrink;
    if (mOwner->GetGameObject()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        fIncrease *= 0.5f;
    }
    IncreaseTargetBubbleSize(fIncrease);

    float fCharge = mAvailableBubbleCharge - fShrink * 0.5f;
    if (fCharge < 0.0f)
    {
        fCharge = 0.0f;
    }
    else if (fCharge > 5.0f)
    {
        fCharge = 5.0f;
    }
    mAvailableBubbleCharge = fCharge;

    if (mOwner->GetGameObject())
    {
        PblVector3 vVelocity = mOwner->GetGameObject()->GetVelocity();
        float fMaxSpeed = mOwner->GetGameObject()->GetMaxSpeed();
        if (fMaxSpeed < 0.1f)
        {
            fMaxSpeed = 0.1f;
        }
        PblVector3 vDelta(mTargetBubbleLastVel.x - vVelocity.x,
                          mTargetBubbleLastVel.y - vVelocity.y,
                          mTargetBubbleLastVel.z - vVelocity.z);
        mTargetBubbleLastVel = vVelocity;
        IncreaseTargetBubbleSize(
            sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z) * 0.455f
            / fMaxSpeed);
    }

    PblVector3 vRandom = PblRandom::_Global.GetVector3();
    PblVector3 vAxis(mTargetBubbleAxis.x + vRandom.x, mTargetBubbleAxis.y + vRandom.y,
                     mTargetBubbleAxis.z + vRandom.z);
    D3DXVec3Normalize(&vRandom, &vAxis);
    mTargetBubbleAxis = vRandom;

    float fAngle = dt * 2.0f;
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle * 0.5f);
    PblQuaternion qTurn;
    qTurn._fX = mTargetBubbleAxis.x * fSin;
    qTurn._fY = mTargetBubbleAxis.y * fSin;
    qTurn._fZ = mTargetBubbleAxis.z * fSin;
    qTurn._fS = fCos;
    mTargetBubbleRotation = mTargetBubbleRotation * qTurn;
    mTargetBubbleOffset = mTargetBubbleRotation.RotateVector(PblVector3(0.0f, 1.0f, 0.0f));

    mTargetBubbleSize_Cached = _GetTargetBubbleSize();
}

// FUNCTION: SWBF2 0x00485810
void Controller::UpdateFreeSpace()
{
    float fTime = GameLoop::GetMissionTime();
    if (mOpenRadiusTime + 2.0f > fTime)
    {
        return;
    }
    mOpenRadiusTime = fTime;

    if (!mOwner || !mOwner->GetGameObject())
    {
        mOpenRadius = 0.0f;
        return;
    }

    float fStep = 5.0f;
    float fRadius = 10.0f;
    for (int i = 2; i != 0; --i)
    {
        const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(mOwner->GetGameObject());
        const PblMatrix& mMatrix = mOwner->GetMatrix();
        GameObject* pObject = mOwner->GetGameObject();
        if (PathFinder::IsPointValid(*(const PblVector3*)&mMatrix.m[3][0],
                                     fRadius + AIUtil::GetCollisionRadius(pObject),
                                     AIUtil::GetAIType(pObject), 0, &pIgnore, 1))
        {
            fRadius += fStep;
        }
        else
        {
            fRadius -= fStep;
        }
        fStep *= 0.5f;
    }
    mOpenRadius = fRadius;
}

// FUNCTION: SWBF2 0x00485940
void Controller::SetFoliageVisFactor(float fCrouch, float fStand)
{
    mFoliageVisCrouch = fCrouch;
    mFoliageVisStand = fStand;
}

// FUNCTION: SWBF2 0x00485950
bool Controller::GetCrouchedForVis()
{
    if (mOwner->GetGameObject() &&
        mOwner->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        SoldierState::SoldierState eState = static_cast<EntitySoldier*>(mOwner->GetGameObject())->mState;
        if (eState == SoldierState::CROUCH || eState == SoldierState::PRONE)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004859b0
float Controller::GetFoliageVisFactor()
{
    return GetCrouchedForVis() ? mFoliageVisCrouch : mFoliageVisStand;
}

// FUNCTION: SWBF2 0x004859d0
float Controller::GetInvisibilityVisFactor()
{
    if (!mOwner->GetGameObject() ||
        !mOwner->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return 1.0f;
    }
    float fVis = 1.0f - static_cast<EntitySoldier*>(mOwner->GetGameObject())->GetInvisibilityFactor();
    if (fVis < 0.0f)
    {
        return 0.0f;
    }
    if (fVis > 1.0f)
    {
        return 1.0f;
    }
    return fVis;
}

// FUNCTION: SWBF2 0x00485a70
float Controller::GetRegionVisFactor()
{
    if (GameLoop::GetMissionTime() - 1.0f > mRegionVisTime)
    {
        return 1.0f;
    }
    return GetCrouchedForVis() ? mRegionVisCrouch : mRegionVisStand;
}

// FUNCTION: SWBF2 0x00485ab0
void Controller::UpdateTotalVisFactor()
{
    float fVis = GetInvisibilityVisFactor() * GetFoliageVisFactor();
    mTotalVisFactor = GetRegionVisFactor() * fVis;
}

// FUNCTION: SWBF2 0x00485af0
bool Controller::IsInCrowd() const
{
    return mCrowdAllies > 3 && !mCrowdEnemies;
}

// FUNCTION: SWBF2 0x00485b10
bool IsPilotControlState(const void* pControllable)
{
    const unsigned int state = static_cast<const Controllable*>(pControllable)->mPilotType;
    return state == 1 || state == 4;
}
