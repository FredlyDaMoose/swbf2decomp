#include "IdleHelper.h"

#include <stddef.h>

#include <float.h>
#include <math.h>

#include "AILowLevel.h"
#include "AIUtil.h"
#include "Character.h"
#include "Controllable.h"
#include "Entity.h"
#include "GameLoop.h"
#include "Team.h"
#include "UnitAgent.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMath.h"

class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

namespace AI
{
enum VOType
{
    VOT_DefendIdleLookWeapon = 19,
    VOT_DefendIdleLookAround = 20,
    VOT_DefendIdleLookPlayer = 21
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);
}
}

namespace
{

    AILowLevel& LowLevel(UnitController* pCtrl)
    {
        return *reinterpret_cast<AILowLevel*>(pCtrl->mLowLevel);
    }
}

// FUNCTION: SWBF2 0x00569970
IdleHelper::IdleHelper(UnitAgent* pOwner)
    : HelperFsm(STATE_START, STATE_START)
    , mOwner(pOwner)
    , mCoverHelper(pOwner)
{
    mIdleTime = AIUtil::GetRandomFloat(1.0f, 5.0f);
    mWanderSuppressTime = 0.0f;
    mAllowedIdles = (unsigned int)IH_ALL;
    mHintNode = NULL;
}

// FUNCTION: SWBF2 0x005699d0
void IdleHelper::Kill()
{
    mCoverHelper.Kill();
    HelperFsm::Kill();
}

// FUNCTION: SWBF2 0x005699f0
void IdleHelper::Init(unsigned int uAllowedIdles, const BaseHint* pHint)
{
    mAllowedIdles = uAllowedIdles;
    mHintNode = const_cast<BaseHint*>(pHint);
    mIdleTime = 0.0f;
    mWanderSuppressTime = 0.0f;
    SetNextState(STATE_START);
    FSMChangeState();
}

// FUNCTION: SWBF2 0x00569b00
void IdleHelper::ExitState()
{
    if (mCurState == STATE_Cover)
    {
        mCoverHelper.HELPERClean();
    }
}

// FUNCTION: SWBF2 0x00569a30
void IdleHelper::Clean()
{
    ExitState();
}

// FUNCTION: SWBF2 0x00569a40
void IdleHelper::EnterState()
{
    switch (mCurState)
    {
    case STATE_START:
        if (!mOwner->mCtrl->mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            SetNextState(STATE_VehicleIdle);
        }
        else if (mOwner->mCtrl->mLodState < 3)
        {
            mIdleTime = 2.0f;
        }
        else if (mAllowedIdles & IH_Stationary)
        {
            if (mHintNode && AIUtil::GetRandomInt(0, 100) > 40)
            {
                SetNextState(STATE_Cover);
            }
            else if (mAllowedIdles & IH_Stationary)
            {
                SetNextState(STATE_Stationary);
            }
        }
        break;

    case STATE_Stationary:
        StartStationaryIdle(mOwner->mCtrl, mIdleTime, mWanderSuppressTime);
        break;

    case STATE_Cover:
        mCoverHelper.InitAtHint(COVERTYPE_Idle, mHintNode);
        break;
    }
}

// FUNCTION: SWBF2 0x00569b10
void IdleHelper::StartStationaryIdle(UnitController* pCtrl, float& fIdleTime, float& fWanderSuppressTime)
{
    float fTurnToTime = AIUtil::GetRandomFloat(0.5f, 2.0f);
    float fStayTime = AIUtil::GetRandomFloat(1.0f, 4.0f);
    float fTurnBackTime = AIUtil::GetRandomFloat(0.5f, 2.0f);
    float fTime = GameLoop::GetMissionTime();

    switch (AIUtil::GetRandomInt(0, 15))
    {
    case 3:
    case 4:
    case 5:
        LowLevel(pCtrl).HeadLook_Glance(false, fTurnToTime, fStayTime, fTurnBackTime);
        fWanderSuppressTime = fTime + fTurnToTime + fStayTime + fTurnBackTime;
        break;

    case 6:
        LowLevel(pCtrl).mDoIdle = SoldierAnimType::IDLE_LOOK_AROUND;
        AI::VOHelper::Play(AI::VOT_DefendIdleLookAround, pCtrl->mOwner->GetGameObject(), 0, 0, 0.0f);
        fWanderSuppressTime = fTime + 7.0f;
        break;

    case 7:
        LowLevel(pCtrl).mDoIdle = SoldierAnimType::IDLE_CHECK_WEAPON;
        AI::VOHelper::Play(AI::VOT_DefendIdleLookWeapon, pCtrl->mOwner->GetGameObject(), 0, 0, 0.0f);
        fWanderSuppressTime = fTime + 7.0f;
        break;

    case 8:
    case 9:
    case 10:
    case 11:
    {
        const Character* pPlayer = pCtrl->mLodHumanPlayer;
        if (pPlayer && pPlayer->GetControllable() && pPlayer->GetControllable()->GetGameObject())
        {
            LowLevel(pCtrl).HeadLook_Object(pPlayer->GetControllable()->GetGameObject(),
                                            fTurnToTime, fStayTime, fTurnBackTime);
            AI::VOHelper::Play(AI::VOT_DefendIdleLookPlayer, pCtrl->mOwner->GetGameObject(), 0,
                               pPlayer->GetControllable()->GetGameObject(), 0.0f);
            fWanderSuppressTime = fTime + fTurnToTime + fStayTime + fTurnBackTime;
        }
        break;
    }

    case 12:
    case 13:
    case 14:
    case 15:
    {
        Controllable* pMe = pCtrl->mOwner;
        float fBestDistSq = FLT_MAX;
        PblVector3 vMyPos = *(const PblVector3*)&pMe->GetMatrix().m[3][0];
        const Character* pBest = NULL;
        for (int i = 0; i < Character::sCharacterLimit; ++i)
        {
            const Character* pChar = Character::GetPtr(i);
            if (pChar && pChar->mTeamPtr &&
                pChar->mTeamPtr->GetAffiliation(pMe->mCharacter->mTeamNumber) >= 0 &&
                pChar != pMe->mCharacter && pChar->GetControllable() &&
                pChar->GetControllable()->GetGameObject())
            {
                const PblMatrix& m = pChar->GetControllable()->GetMatrix();
                float fX = m.m[3][0] - vMyPos.x;
                float fY = m.m[3][1] - vMyPos.y;
                float fZ = m.m[3][2] - vMyPos.z;
                float fDistSq = fX * fX + fY * fY + fZ * fZ;
                if (fDistSq < fBestDistSq)
                {
                    fBestDistSq = fDistSq;
                    pBest = pChar;
                }
            }
        }
        if (pBest)
        {
            LowLevel(pCtrl).HeadLook_Object(pBest->GetControllable()->GetGameObject(),
                                            fTurnToTime, fStayTime, fTurnBackTime);
            AI::VOHelper::Play(AI::VOT_DefendIdleLookPlayer, pMe->GetGameObject(), 0,
                               pBest->GetControllable()->GetGameObject(), 0.0f);
            fWanderSuppressTime = fTime + fTurnToTime + fStayTime + fTurnBackTime;
        }
        break;
    }

    default:
        LowLevel(pCtrl).HeadLook_Glance(true, fTurnToTime, fStayTime, fTurnBackTime);
        fWanderSuppressTime = fTime + fTurnToTime + fStayTime + fTurnBackTime;
        break;
    }

    fIdleTime = AIUtil::GetRandomFloat(0.5f, 2.0f) + (fWanderSuppressTime - fTime);
}

// FUNCTION: SWBF2 0x00569f60
void IdleHelper::UpdateState(float dt)
{
    LowLevel(mOwner->mCtrl).SetHeight(HEIGHT_STAND);

    switch (mCurState)
    {
    case STATE_START:
        mIdleTime -= dt;
        if (mIdleTime < 0.0f)
        {
            SetNextState(STATE_START);
        }
        break;

    case STATE_Stationary:
    {
        mIdleTime -= dt;
        if (mIdleTime < 0.0f)
        {
            SetNextState(STATE_START);
            break;
        }
        float fTime = GameLoop::GetMissionTime();
        if (mWanderSuppressTime > fTime)
        {
            LowLevel(mOwner->mCtrl).mHeading.mDestWander = 0.0f;
        }
        else
        {
            float fWander = (PblMath::Cos(mIdleTime / 5.0f * 0.3f) + PblMath::Sin(mIdleTime / 5.0f)) * 0.2f;
            float fSince = fTime - mWanderSuppressTime;
            if (fSince < 2.0f)
            {
                fWander = fSince * 0.5f * fWander;
            }
            LowLevel(mOwner->mCtrl).mHeading.mDestWander = (float)fabs(fWander);
        }
        break;
    }

    case STATE_Cover:
        mCoverHelper.FSMUpdate(dt, false);
        if (mCoverHelper.IsDone())
        {
            SetNextState(STATE_START);
        }
        break;

    case STATE_VehicleIdle:
        mOwner->FaceTarget(HEIGHT_STAND);
        break;
    }
}
