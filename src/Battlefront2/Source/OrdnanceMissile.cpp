#include "OrdnanceMissile.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9math.h>

#include "AIUtil.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "EntityCarrier.h"
#include "FLEffect.h"
#include "GameObject.h"
#include "GameLoop.h"
#include "UnitController.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

class Entity;
class WeaponClass;

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

extern PblVector3 gVectorZero;

// GLOBAL: SWBF2 0x01ef265c
// SYNTHETIC: SWBF2 0x00405b40
// `dynamic initializer for 'OrdnanceMissile::rttiHashOrdnanceMissile''
PblHash OrdnanceMissile::rttiHashOrdnanceMissile("OrdnanceMissile");
char OrdnanceMissile::rttiNameOrdnanceMissile[] = "OrdnanceMissile";

// FUNCTION: SWBF2 0x006048d0
bool OrdnanceMissile::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceMissile || OrdnanceBullet::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00604900
unsigned int OrdnanceMissile::GetDerivedRtti() const
{
    return rttiHashOrdnanceMissile;
}

// FUNCTION: SWBF2 0x00604910
const char* OrdnanceMissile::GetDerivedRttiName() const
{
    return rttiNameOrdnanceMissile;
}

void Controllable::SetEnemyLockedOnMeState(float fState, OrdnanceMissile* pMissile, float fDistance)
{
    mEnemyLockedOnMeState = fState;
    mEnemyLockedOnMeTimestamp = GameLoop::GetMissionTime();
    if (mEnemyLockedOnMeMissile == pMissile)
    {
        mEnemyLockedOnMeDistance = fDistance;
    }
    else if (!mEnemyLockedOnMeMissile || mEnemyLockedOnMeDistance > fDistance)
    {
        mEnemyLockedOnMeMissile = pMissile;
        mEnemyLockedOnMeDistance = fDistance;
    }
}

static float Distance(const PblVector3& a, const PblVector3& b)
{
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return sqrtf(dy * dy + dx * dx + dz * dz);
}

// FUNCTION: SWBF2 0x006049b0
OrdnanceMissile::OrdnanceMissile(OrdnanceMissileClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceBullet(pClass, desc),
      mTarget(desc.mTarget),
      mBodyID(desc.mBodyID),
      mWaverPitch(PblRandom::GetSim().GetFloat() * PblMath::fTwoPi),
      mWaverYaw(PblRandom::GetSim().GetFloat() * PblMath::fTwoPi)
{
    OrdnanceMissileClass* pMissileClass = (OrdnanceMissileClass*)mClass;
    mTargetDir = desc.mDirection;
    mVelocityDir = desc.mDirection;
    if (pMissileClass->mAcceleration > 9000.0f)
    {
        mSpeed = pMissileClass->mVelocity;
    }
    else
    {
        mSpeed = pMissileClass->mMinSpeed;
    }
    mVelocity.x = mVelocityDir.x * mSpeed;
    mVelocity.y = mVelocityDir.y * mSpeed;
    mVelocity.z = mVelocityDir.z * mSpeed;
    mGoalSpeed = pMissileClass->mVelocity;
    mScale = 0.0f;

    if (mTarget)
    {
        mLockedOnSoundCtrl.Play(&pMissileClass->mLockedOnSound, &mPosition, &mVelocity, true);
        for (PblList<Controllable>::Itor it(mTarget->GetControllableList()); it; ++it)
        {
            if (it->GetEnemyLockedOnMeState() < 2.0f || it->GetEnemyLockedOnMeState() > 0.2f)
            {
                it->SetEnemyLockedOnMeState(2.0f, this, Distance(static_cast<const CollisionObject*>(mTarget)->GetCollisionCenter(), mPosition));
            }
        }
    }
}

// FUNCTION: SWBF2 0x00604d60
OrdnanceMissile::~OrdnanceMissile()
{
    if (mLockedOnSoundCtrl.mFlags & GameSoundControllable::flags_playing)
    {
        mLockedOnSoundCtrl.Stop(true);
    }
    if (mTarget)
    {
        for (PblList<Controllable>::Itor it(mTarget->GetControllableList()); it; ++it)
        {
            if (it->mEnemyLockedOnMeState > 0.2f)
            {
                it->mEnemyLockedOnMeState = 0.0f;
                it->mEnemyLockedOnMeTimestamp = GameLoop::GetMissionTime();
                it->mEnemyLockedOnMeDistance = FLT_MAX;
            }
        }
    }
}

// SYNTHETIC: SWBF2 0x00605b26
// [thunk]:OrdnanceMissile::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x00605b2e
// [thunk]:OrdnanceMissile::`scalar deleting destructor' adjustor{152}
// SYNTHETIC: SWBF2 0x00604920
// OrdnanceMissile::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00604ea0
bool OrdnanceMissile::Update(float fDeltaTime)
{
    OrdnanceMissileClass* pClass = (OrdnanceMissileClass*)mClass;
    OrdnanceMissileClass* pNext = (OrdnanceMissileClass*)pClass->mNextCharge;

    const float fScale = pClass->mScaleRate * fDeltaTime + mScale;
    mScale = fScale < 1.0f ? fScale : 1.0f;

    const float fTurnRate = ((pNext->mTurnRate - pClass->mTurnRate) * mRatio + pClass->mTurnRate) * mScale;
    if (fTurnRate > 0.0f)
    {
        const GameObject* pTarget = mTarget;
        if (pTarget)
        {
            const PblVector3 vMiss = pTarget->GetTargetPointMissVector();
            const PblVector3 vPoint = pTarget->GetTargetPoint(mPosition, mVelocityDir, mBodyID);
            PblVector3 vAim(vPoint.x + vMiss.x, vPoint.y + vMiss.y, vPoint.z + vMiss.z);
            if (mOwner)
            {
                if (UnitControllerPtr(mOwner->GetControllable()) &&
                    !pTarget->IsRtti(EntityFlyer::rttiHashEntityFlyer))
                {
                    const WeaponClass* pWeaponClass = mOwner->GetControllable()->GetCurWpn()->mClass;
                    const PblVector3 vBubble = AIUtil::CalculateBubbleOffset(
                        mOwner->GetControllable(), vAim, pTarget, pWeaponClass);
                    vAim.x += vBubble.x;
                    vAim.y += vBubble.y;
                    vAim.z += vBubble.z;
                }
            }

            PblVector3 vToAim(vAim.x - mPosition.x, vAim.y - mPosition.y, vAim.z - mPosition.z);
            PblVector3 vTargetDir;
            D3DXVec3Normalize((D3DXVECTOR3*)&vTargetDir, (const D3DXVECTOR3*)&vToAim);
            mTargetDir = vTargetDir;

            for (PblList<Controllable>::Itor it(mTarget->GetControllableList()); it; ++it)
            {
                if (it->GetEnemyLockedOnMeState() < 2.0f || it->GetEnemyLockedOnMeState() > 0.2f)
                {
                    it->SetEnemyLockedOnMeState(2.0f, this, Distance(static_cast<const CollisionObject*>(pTarget)->GetCollisionCenter(), mPosition));
                }
            }
        }

        PblVector3 vAxis(mVelocityDir.y * mTargetDir.z - mVelocityDir.z * mTargetDir.y,
                         mVelocityDir.z * mTargetDir.x - mVelocityDir.x * mTargetDir.z,
                         mVelocityDir.x * mTargetDir.y - mVelocityDir.y * mTargetDir.x);
        const float fLengthSq = vAxis.y * vAxis.y + vAxis.x * vAxis.x + vAxis.z * vAxis.z;
        if (fLengthSq > 1.0e-8f)
        {
            const float fLength = (float)sqrt((double)fLengthSq);
            const float fInv = 1.0f / fLength;
            vAxis.x *= fInv;
            vAxis.y *= fInv;
            vAxis.z *= fInv;
            const float fMaxTurn = fTurnRate * fDeltaTime;
            D3DXMATRIX mRotation;
            D3DXMatrixRotationAxis(&mRotation, (const D3DXVECTOR3*)&vAxis,
                                   fMaxTurn > fLength ? fLength : fMaxTurn);
            PblVector3 vDir;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocityDir, &mRotation);
            mVelocityDir = vDir;
        }
    }

    const float fWaverTurn = ((pNext->mWaverTurn - pClass->mWaverTurn) * mRatio + pClass->mWaverTurn) * mScale;
    if (fWaverTurn > 0.0f)
    {
        const PblMatrix mDir = DirectionMatrix(mVelocityDir, gVectorUnitY, gVectorZero);
        const float fSinYaw = PblMath::Sin(mWaverYaw);
        const PblVector3 vUp(mDir.m[1][0] * fSinYaw, mDir.m[1][1] * fSinYaw, mDir.m[1][2] * fSinYaw);
        const float fSinPitch = PblMath::Sin(mWaverPitch);
        PblVector3 vAxis(mDir.m[0][0] * fSinPitch + vUp.x, mDir.m[0][1] * fSinPitch + vUp.y,
                         mDir.m[0][2] * fSinPitch + vUp.z);
        const float fLengthSq = vAxis.y * vAxis.y + vAxis.x * vAxis.x + vAxis.z * vAxis.z;
        if (fLengthSq > 1.0e-8f)
        {
            const float fLength = (float)sqrt((double)fLengthSq);
            const float fInv = 1.0f / fLength;
            vAxis.x *= fInv;
            vAxis.y *= fInv;
            vAxis.z *= fInv;
            D3DXMATRIX mRotation;
            D3DXMatrixRotationAxis(&mRotation, (const D3DXVECTOR3*)&vAxis, fWaverTurn * fDeltaTime * fLength);
            PblVector3 vDir;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocityDir, &mRotation);
            mVelocityDir = vDir;
        }

        const float fWaverRate =
            ((pNext->mWaverRate - pClass->mWaverRate) * mRatio + pClass->mWaverRate) * fDeltaTime * 2.0f;
        mWaverPitch += PblRandom::GetSim().GetFloat() * fWaverRate;
        mWaverYaw += PblRandom::GetSim().GetFloat() * fWaverRate;
    }

    mSpeed += pClass->mAcceleration * fDeltaTime;
    if (mSpeed > mGoalSpeed)
    {
        mSpeed = mGoalSpeed;
    }

    PblVector3 vDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocityDir);
    mVelocityDir = vDir;
    mVelocity.x = mVelocityDir.x * mSpeed;
    mVelocity.y = mVelocityDir.y * mSpeed;
    mVelocity.z = mVelocityDir.z * mSpeed;

    if (mLockedOnSoundCtrl.mFlags & GameSoundControllable::flags_playing)
    {
        mLockedOnSoundCtrl.Update(&pClass->mLockedOnSound, &mPosition, &mVelocity);
    }

    if (mTrailEmitterObj)
    {
        mTrailEmitterObj->SetOrientation(mVelocity);
    }

    return OrdnanceBullet::Update(fDeltaTime);
}

// GLOBAL: SWBF2 0x01ef2658
// SYNTHETIC: SWBF2 0x00405b50
// `dynamic initializer for 'OrdnanceMissileClass::rttiHashOrdnanceMissileClass''
PblHash OrdnanceMissileClass::rttiHashOrdnanceMissileClass("OrdnanceMissileClass");
char OrdnanceMissileClass::rttiNameOrdnanceMissileClass[] = "OrdnanceMissileClass";

// FUNCTION: SWBF2 0x00605790
OrdnanceMissileClass::OrdnanceMissileClass(unsigned int uHash)
    : OrdnanceBulletClass(uHash)
{
    mLifeSpan = 3.0f;
    mModel = 0;
    mStraightBullet = false;
    mVelocity = 35.0f;

    mScaleRate = 1.0f;
    mTurnRate = 1.0f;
    mWaverRate = 0.0f;
    mWaverTurn = 0.0f;
    mMinSpeed = 0.1f;
    mAcceleration = 10000.0f;
}

// FUNCTION: SWBF2 0x00605850
OrdnanceMissileClass::OrdnanceMissileClass(OrdnanceMissileClass* pBase, unsigned int uHash)
    : OrdnanceBulletClass(pBase, uHash)
{
    mScaleRate = pBase->mScaleRate;
    mTurnRate = pBase->mTurnRate;
    mWaverRate = pBase->mWaverRate;
    mWaverTurn = pBase->mWaverTurn;
    mLockedOnSound = pBase->mLockedOnSound;
    mMinSpeed = pBase->mMinSpeed;
    mAcceleration = pBase->mAcceleration;
}

// FUNCTION: SWBF2 0x00604950
bool OrdnanceMissileClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceMissileClass || OrdnanceBulletClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00604980
unsigned int OrdnanceMissileClass::GetDerivedRtti()
{
    return rttiHashOrdnanceMissileClass;
}

// FUNCTION: SWBF2 0x00604990
const char* OrdnanceMissileClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceMissileClass;
}

// FUNCTION: SWBF2 0x006049a0
float OrdnanceMissileClass::GetVelocity()
{
    return 3.0e8f;
}

// FUNCTION: SWBF2 0x00605910
void OrdnanceMissileClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x73E5A988:
        if (sscanf(pszValue, "%f", &mScaleRate))
        {
            mScaleRate = 1.0f / mScaleRate;
        }
        return;
    case 0x0F77C4E8:
        sscanf(pszValue, "%f", &mTurnRate);
        return;
    case 0x6D4A8840:
        sscanf(pszValue, "%f", &mWaverRate);
        return;
    case 0x129F08C1:
        sscanf(pszValue, "%f", &mWaverTurn);
        return;
    case 0x9371AECD:
        mLockedOnSound.SetID(pszValue, false, false);
        return;
    case 0xFBE2EB62:
        sscanf(pszValue, "%f", &mMinSpeed);

    case 0x2FA0BA9D:
        sscanf(pszValue, "%f", &mAcceleration);

    default:
        OrdnanceBulletClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00605a20
OrdnanceClass* OrdnanceMissileClass::Derive(unsigned int uHash)
{
    return new OrdnanceMissileClass(this, uHash);
}

// FUNCTION: SWBF2 0x00605a90
Ordnance* OrdnanceMissileClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceMissile* pMissile = new OrdnanceMissile(this, desc);
    pMissile->ActivateThread(-40, PER_LOCAL_TURN);
    pMissile->AddOrdnanceToCollisionManager();
    return pMissile;
}

// SYNTHETIC: SWBF2 0x00605820
// OrdnanceMissileClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00605900
OrdnanceMissileClass::~OrdnanceMissileClass()
{
}
