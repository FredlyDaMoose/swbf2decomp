#include "UnitWaitAgent.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"

// GLOBAL: SWBF2 0x01f9a258
// SYNTHETIC: SWBF2 0x00406d50
// `dynamic initializer for 'UnitWaitAgent::rttiHashUnitWaitAgent''
PblHash UnitWaitAgent::rttiHashUnitWaitAgent("UnitWaitAgent");
char UnitWaitAgent::rttiNameUnitWaitAgent[] = "UnitWaitAgent";

// FUNCTION: SWBF2 0x0066b330
bool UnitWaitAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitWaitAgent || UnitAgent::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0066b360
unsigned int UnitWaitAgent::GetDerivedRtti() const
{
    return rttiHashUnitWaitAgent;
}

// FUNCTION: SWBF2 0x0066b370
const char* UnitWaitAgent::GetDerivedRttiName() const
{
    return rttiNameUnitWaitAgent;
}

// FUNCTION: SWBF2 0x0066b3b0
UnitWaitAgent::UnitWaitAgent(UnitController* pCtrl)
    : UnitAgent(pCtrl)
{
    mCurState = 1;
    mNextState = 2;
}

// FUNCTION: SWBF2 0x0066b3e0
void UnitWaitAgent::EnterState()
{
    if (mCurState == 2)
    {
        mTargetTimer = GameLoop::GetMissionTime();
        SetNewSearchHeading();
    }
}

// FUNCTION: SWBF2 0x0066b400
void UnitWaitAgent::ExitState()
{
}

// FUNCTION: SWBF2 0x0066b410
void UnitWaitAgent::SetNewSearchHeading()
{
    float fRange = 1.3089969f;
    if (mCtrl->IsLookAheadOnly())
    {
        fRange = 0.52359879f;
    }
    float fAngle = AIUtil::GetRandomFloat(-fRange, fRange);

    const PblVector3& vAhead = mCtrl->GetOriginalAheadDir();
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    PblVector3 vOffset((vAhead.x * fCos - vAhead.z * fSin) * 1000.0f,
                       vAhead.y * 1000.0f,
                       (vAhead.z * fCos + vAhead.x * fSin) * 1000.0f);

    const PblMatrix& mat = GetMe()->GetMatrix();
    mSearchPos = PblVector3(mat.m[3][0] + vOffset.x, mat.m[3][1] + vOffset.y, mat.m[3][2] + vOffset.z);
}

// FUNCTION: SWBF2 0x0066b500
void UnitWaitAgent::UpdateStatePostVision(float dt)
{
    AttackHelper& attack = *reinterpret_cast<AttackHelper*>(mAttackHelper);
    attack.Update();
    if (mCurState != 2)
    {
        return;
    }

    if (AIUtil::GetRandomIntPerSecond(1, 9) == 2)
    {
        SetNewSearchHeading();
    }

    GameObject* pObj = GetMe()->GetGameObject();
    if (pObj && pObj->GetMaxSpeed() == 0.0f)
    {
        if (attack.HasTarget())
        {
            mTargetTimer = GameLoop::GetMissionTime();
        }
        else if (GameLoop::GetMissionTime() > mTargetTimer + AIUtil::gStationaryWaitTime)
        {
            GetLowLevel().SetExitVehicle(0);
            return;
        }
    }

    UnitAgent::FaceTargetCommon(HEIGHT_COUNT, true,
                                PblVector3(mSearchPos.x + 0.0f, mSearchPos.y + 1.6f, mSearchPos.z + 0.0f));
}

// FUNCTION: SWBF2 0x0066b660
const char* UnitWaitAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}

// SYNTHETIC: SWBF2 0x0066b380
// UnitWaitAgent::`scalar deleting destructor'
