#include "UnitAttackPatterns.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityFlyer.h"
#include "GameObject.h"
#include "PathManager.h"
#include "UnitAgent.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

class GameObject;

namespace
{
    const PblVector3& GetPos(const PblMatrix& m)
    {
        return *(const PblVector3*)&m.m[3][0];
    }

    const PblVector3& GetFront(const PblMatrix& m)
    {
        return *(const PblVector3*)&m.m[2][0];
    }

    const PblVector3& GetTargetVelocity(const GameObject* pTarget)
    {
        typedef const Entity*(__thiscall * EntityFn)(const GameObject*);
        typedef const PblVector3&(__thiscall * VelocityFn)(const Entity*);
        const Entity* pEntity = pTarget->GetEntity();
        return static_cast<const EntityGeometry*>(pEntity)->GetVelocity();
    }
}

// FUNCTION: SWBF2 0x00667a30
void AttackPatterns::DoFlyDirect()
{
    const EntityFlyerClass* pClass =
        static_cast<const EntityFlyerClass*>(GetMe()->GetEntity()->GetEntityClass());

    PblVector3 vPos = GetPos(GetMe()->GetMatrix());
    PblVector3 vTarget = GetAttackHelper()->GetLastSeenTargetPos();
    PblVector3 vToTarget;
    vToTarget.x = vTarget.x - vPos.x;
    vToTarget.y = vTarget.y - vPos.y;
    vToTarget.z = vTarget.z - vPos.z;
    PblVector3 vFront = GetFront(GetMe()->GetMatrix());

    if (vFront.y * vToTarget.y + vFront.x * vToTarget.x + vFront.z * vToTarget.z > 0.0f ||
        AIUtil::GetRandomIntPerSecond(1, 3) == 1)
    {
        mOwner->GetLowLevel().SetFireRequest(true, false);
        mOverrideFire = true;
    }

    if (!mDoLoop && AIUtil::GetRandomIntPerSecond(1, 13) == 1)
    {
        mDoLoop = true;
    }

    if (mDoLoop)
    {
        if (GetMe()->GetMatrix().m[1][1] < 0.0f)
        {
            mUpsideDown = true;
        }
        else if (mUpsideDown && GetMe()->GetMatrix().m[1][1] > 0.0f)
        {
            mDoLoop = false;
            mUpsideDown = false;
        }
    }
    else
    {
        PblVector3 vDest;
        float fDistSq = vToTarget.y * vToTarget.y + vToTarget.x * vToTarget.x + vToTarget.z * vToTarget.z;
        if (fDistSq > (pClass->mMaxFlyerSpeed * 2.0f) * (pClass->mMaxFlyerSpeed * 2.0f))
        {
            const PblVector3& vVelocity = GetTargetVelocity(GetAttackHelper()->GetTarget());
            vDest.x = vVelocity.x + vTarget.x;
            vDest.y = vVelocity.y + vTarget.y;
            vDest.z = vVelocity.z + vTarget.z;
            if (vDest.y > AIUtil::gMaxFlyHeight)
            {
                vDest.y = AIUtil::gMaxFlyHeight;
            }
        }
        else
        {
            float fAhead = pClass->mMaxFlyerSpeed * 5.0f;
            vDest.x = vFront.x * fAhead + vPos.x;
            vDest.y = vFront.y * fAhead + vPos.y;
            vDest.z = vFront.z * fAhead + vPos.z;
            float fMid = (AIUtil::gMinFlyHeight + AIUtil::gMaxFlyHeight) * 0.5f;
            if (vDest.y > fMid)
            {
                vDest.y = fMid;
            }
        }
        mOwner->GetLowLevel().SetNavigator_GotoDirect(vDest, false, false);
    }

    mOwner->GetLowLevel().SetMinControlSpeed(pClass->mMaxFlyerSpeed * 0.5f);
}

// FUNCTION: SWBF2 0x00667550
void AttackPatterns::GeneratePath()
{
    PblVector3 vPos = GetPos(GetMe()->GetMatrix());
    PblVector3 vTarget = GetAttackHelper()->GetLastSeenTargetPos();
    PblVector3 vToTarget;
    vToTarget.x = vTarget.x - vPos.x;
    vToTarget.y = vTarget.y - vPos.y;
    vToTarget.z = vTarget.z - vPos.z;
    PblVector3 vDir = AIUtil::GetSafeNormalXZ(vToTarget);

    const EntityFlyerClass* pClass =
        static_cast<const EntityFlyerClass*>(GetMe()->GetEntity()->GetEntityClass());
    static const float fBack = (pClass->mMaxFlyerSpeed * 4.0f < 160.0f) ? 160.0f : pClass->mMaxFlyerSpeed * 4.0f;
    static const float fSide = (pClass->mMaxFlyerSpeed < 40.0f) ? 40.0f : pClass->mMaxFlyerSpeed;

    PblVector3 vBehind;
    vBehind.x = (vTarget.x - vDir.x * fBack) + gVectorUnitY.x * 90.0f;
    vBehind.y = (vTarget.y - vDir.y * fBack) + gVectorUnitY.y * 90.0f;
    vBehind.z = (vTarget.z - vDir.z * fBack) + gVectorUnitY.z * 90.0f;
    PblVector3 vNear;
    vNear.x = (vTarget.x - vDir.x * fSide) + gVectorUnitY.x * 0.0f;
    vNear.y = (vTarget.y - vDir.y * fSide) + gVectorUnitY.y * 0.0f;
    vNear.z = (vTarget.z - vDir.z * fSide) + gVectorUnitY.z * 0.0f;
    PblVector3 vPast;
    vPast.x = (vDir.x * fSide + vTarget.x) + gVectorUnitY.x * 30.0f;
    vPast.y = (vDir.y * fSide + vTarget.y) + gVectorUnitY.y * 30.0f;
    vPast.z = (vDir.z * fSide + vTarget.z) + gVectorUnitY.z * 30.0f;

    if (vBehind.y >= AIUtil::gMaxFlyHeight)
    {
        vBehind.y = AIUtil::gMaxFlyHeight;
    }
    if (vNear.y >= AIUtil::gMaxFlyHeight)
    {
        vNear.y = AIUtil::gMaxFlyHeight;
    }
    if (vPast.y >= AIUtil::gMaxFlyHeight)
    {
        vPast.y = AIUtil::gMaxFlyHeight;
    }

    AIPath* pPath = new AIPath();
    if (pPath)
    {
        pPath->Prepend(vPast, 0);
        pPath->Prepend(vNear, 0);
        pPath->Prepend(vBehind, 0);
        pPath->Prepend(vPos, 0);
        mOwner->GetLowLevel().SetNavigator_FollowPath(pPath, false, false, false);
        pPath->Release();
    }
}

// FUNCTION: SWBF2 0x006678b0
void AttackPatterns::DoFlyStrafe()
{
    const GameObject* pTarget = GetAttackHelper()->GetTarget();
    PblVector3 vPos = GetPos(GetMe()->GetMatrix());
    PblVector3 vTarget = GetPos(static_cast<const CollisionObject*>(pTarget)->GetMatrix());
    PblVector3 vToTarget;
    vToTarget.x = vTarget.x - vPos.x;
    vToTarget.y = vTarget.y - vPos.y;
    vToTarget.z = vTarget.z - vPos.z;
    PblVector3 vFront = GetFront(GetMe()->GetMatrix());

    if (vFront.y * vToTarget.y + vFront.x * vToTarget.x + vFront.z * vToTarget.z > 0.5f)
    {
        mOwner->GetLowLevel().SetFireRequest(true, false);
        mOverrideFire = true;
    }

    if (mOwner->GetLowLevel().IsAtDest())
    {
        GeneratePath();
        return;
    }

    const EntityFlyerClass* pClass =
        static_cast<const EntityFlyerClass*>(GetMe()->GetEntity()->GetEntityClass());
    mOwner->GetLowLevel().SetMinControlSpeed(pClass->mMaxFlyerSpeed * 0.5f);
}

// FUNCTION: SWBF2 0x006679f0
void AttackPatterns::CleanFlyStrafe()
{
    GetAttackHelper()->ResetWaitDelay();
}

// FUNCTION: SWBF2 0x00667530
void AttackPatterns::InitFlyStrafe()
{
    GeneratePath();
    GetAttackHelper()->SetWaitDelay(0.5f);
}

// FUNCTION: SWBF2 0x00667a00
void AttackPatterns::InitFlyDirect()
{
    GetAttackHelper()->SetWaitDelay(0.5f);
    mDoLoop = false;
    mUpsideDown = false;
}

// FUNCTION: SWBF2 0x00667ce0
void AttackPatterns::CleanFlyDirect()
{
    GetAttackHelper()->ResetWaitDelay();
}
