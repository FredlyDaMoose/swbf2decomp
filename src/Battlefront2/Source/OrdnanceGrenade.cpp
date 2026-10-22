#include "OrdnanceGrenade.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "CollisionManager.h"
#include "Damageable.h"
#include "GameObject.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "NetPredict.h"
#include "WaterEffect.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Graphics/RedModel.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern bool netIsLateTurn;
extern bool netFrameLock;

namespace AIUtil
{

    void GrenadeAlert(const PblVector3& vPos, const GameObject* pObject, float* pBeginTime,
                      float* pLastEventTime);
}

class PblMatrix;
extern PblMatrix gMatrixIdentity;
#define _IdentityMatrix (gMatrixIdentity)

namespace
{

    Damageable* AsDamageable(GameObject* pObject)
    {
        return static_cast<Damageable*>(pObject);
    }
}

// GLOBAL: SWBF2 0x01ef263c
// SYNTHETIC: SWBF2 0x00405ac0
// `dynamic initializer for 'OrdnanceGrenade::rttiHashOrdnanceGrenade''
PblHash OrdnanceGrenade::rttiHashOrdnanceGrenade("OrdnanceGrenade");
char OrdnanceGrenade::rttiNameOrdnanceGrenade[] = "OrdnanceGrenade";

// GLOBAL: SWBF2 0x01ef2638
// SYNTHETIC: SWBF2 0x00405ad0
// `dynamic initializer for 'OrdnanceGrenadeClass::rttiHashOrdnanceGrenadeClass''
PblHash OrdnanceGrenadeClass::rttiHashOrdnanceGrenadeClass("OrdnanceGrenadeClass");
char OrdnanceGrenadeClass::rttiNameOrdnanceGrenadeClass[] = "OrdnanceGrenadeClass";

// FUNCTION: SWBF2 0x006009c0
OrdnanceGrenadeClass::OrdnanceGrenadeClass(unsigned int uHash)
    : OrdnanceShellClass(uHash)
{
    mRebound = 0.5f;
    mFriction = 0.2f;
    mImpactEffectWater = 0;
    mStartTimerOnContact = true;
    mAlignVertical = true;
}

// FUNCTION: SWBF2 0x00600a50
OrdnanceGrenadeClass::OrdnanceGrenadeClass(OrdnanceGrenadeClass* pBase, unsigned int uHash)
    : OrdnanceShellClass(pBase, uHash)
{
    mRebound = pBase->mRebound;
    mFriction = pBase->mFriction;
    mSoundCollisionWater = pBase->mSoundCollisionWater;
    mSoundCollisionLeaf = pBase->mSoundCollisionLeaf;
    mSoundCollisionOther = pBase->mSoundCollisionOther;
    mStartTimerOnContact = pBase->mStartTimerOnContact;
    mAlignVertical = pBase->mAlignVertical;
    mImpactEffectWater = pBase->mImpactEffectWater;
}

// FUNCTION: SWBF2 0x006000c0
bool OrdnanceGrenadeClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceGrenadeClass || OrdnanceShellClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00600100
unsigned int OrdnanceGrenadeClass::GetDerivedRtti()
{
    return rttiHashOrdnanceGrenadeClass;
}

// FUNCTION: SWBF2 0x00600110
const char* OrdnanceGrenadeClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceGrenadeClass;
}

// FUNCTION: SWBF2 0x00600b40
void OrdnanceGrenadeClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x678DE338)
    {
        sscanf(pszValue, "%f", &mRebound);
    }
    else if (uProperty == 0xA51BE2BB)
    {
        sscanf(pszValue, "%f", &mFriction);
    }
    else if (uProperty == 0xB8C524F1)
    {
        mSoundCollisionLeaf.SetID(pszValue, false, false);
    }
    else if (uProperty == 0xEDC6251A)
    {
        mSoundCollisionOther.SetID(pszValue, false, false);
    }
    else if (uProperty == 0xECAAA59D)
    {
        mImpactEffectWater = PblHash(pszValue);
    }
    else if (uProperty == 0x0D0CDE2B)
    {
        mStartTimerOnContact = atoi(pszValue) != 0;
    }
    else if (uProperty == 0xDB873344)
    {
        mAlignVertical = atoi(pszValue) != 0;
    }
    else
    {
        OrdnanceShellClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x00600c30
OrdnanceClass* OrdnanceGrenadeClass::Derive(unsigned int uHash)
{
    return new OrdnanceGrenadeClass(this, uHash);
}

// FUNCTION: SWBF2 0x00600ca0
Ordnance* OrdnanceGrenadeClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceGrenade* pGrenade = new OrdnanceGrenade(this, desc);
    pGrenade->ActivateThread(-40, PER_LOCAL_TURN);
    pGrenade->AddOrdnanceToCollisionManager();
    return pGrenade;
}

// SYNTHETIC: SWBF2 0x00600a20
// OrdnanceGrenadeClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00600b30
// OrdnanceGrenadeClass::~OrdnanceGrenadeClass

// FUNCTION: SWBF2 0x00600120
OrdnanceGrenade::OrdnanceGrenade(OrdnanceGrenadeClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceShell(pClass, desc)
{
    mNumBounces = 0;
    mDecrementLifeTime = !pClass->mStartTimerOnContact;
    mLastEventTime = -1.0f;
    mBeginTime = FLT_MAX;
}

// FUNCTION: SWBF2 0x00600180
OrdnanceGrenade::~OrdnanceGrenade()
{
}

// SYNTHETIC: SWBF2 0x00600d36
// [thunk]:OrdnanceGrenade::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x00600d3e
// [thunk]:OrdnanceGrenade::`scalar deleting destructor' adjustor{152}
// SYNTHETIC: SWBF2 0x00600090
// OrdnanceGrenade::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00600030
bool OrdnanceGrenade::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceGrenade || OrdnanceShell::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00600070
unsigned int OrdnanceGrenade::GetDerivedRtti() const
{
    return rttiHashOrdnanceGrenade;
}

// FUNCTION: SWBF2 0x00600080
const char* OrdnanceGrenade::GetDerivedRttiName() const
{
    return rttiNameOrdnanceGrenade;
}

// FUNCTION: SWBF2 0x006001a0
void OrdnanceGrenade::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    OrdnanceGrenadeClass* pClass = (OrdnanceGrenadeClass*)mClass;
    if (!pClass->mAlignVertical)
    {
        OrdnanceBullet::Render(ucViewport, fDist, uiFlags);
        return;
    }
    if (!pClass->mModel)
    {
        return;
    }

    PblVector3 vPos = mPosition;
    if (!netFrameLock && NetGame::IsEnabled() &&
        netInterpolator.GetIndex(mInterpIndex, 0, PblHandled::mHandleId))
    {
        netInterpolator.GetVec(mInterpIndex, vPos);
    }

    PblMatrix mat = _IdentityMatrix;
    mat.m[3][0] = vPos.x;
    mat.m[3][1] = vPos.y;
    mat.m[3][2] = vPos.z;
    mat.m[3][3] = 1.0f;
    ((OrdnanceGrenadeClass*)mClass)->mModel->Render(&mat, 0, RedColor::WHITE, uiFlags, 0);
}

// FUNCTION: SWBF2 0x006002c0
bool OrdnanceGrenade::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }
    if (GameLoop::GetMissionTime() > mBeginTime)
    {
        SoundEvent();
    }
    return OrdnanceShell::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x00600310
bool OrdnanceGrenade::CheckCollision()
{
    PblVector3 vStart = GetOldPosition();
    const PblVector3& vEnd = GetPosition();
    PblVector3 vDir(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
    const float fDist = sqrtf(vDir.y * vDir.y + vDir.x * vDir.x + vDir.z * vDir.z);
    if (fDist > 0.0f)
    {
        const float fInv = 1.0f / fDist;
        vDir.x *= fInv;
        vDir.y *= fInv;
        vDir.z *= fInv;

        CollisionObject* pHitObject = 0;
        const GameObject* pIgnore = GetOwner();
        const float fLength = fDist + 0.2f;
        vStart.x -= vDir.x * 0.1f;
        vStart.y -= vDir.y * 0.1f;
        vStart.z -= vDir.z * 0.1f;
        PblVector3 vNormal;
        const float fHit = CollisionManager::RayHit(vStart, vDir, fLength, pHitObject, &vNormal, &pIgnore, 1,
                                                    mCheckStaticCollision ? 0x13BA : 0x132A, true);
        if (fHit < 1.0f)
        {
            const float fAlong = fHit * fLength;
            PblVector3 vHit(vDir.x * fAlong + vStart.x, vDir.y * fAlong + vStart.y,
                            vDir.z * fAlong + vStart.z);
            return !Collide(pHitObject, vHit, vNormal, CollisionManager::GetLastRayHitBodyId());
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00600520
bool OrdnanceGrenade::Collide(CollisionObject* pObject, const PblVector3& vPos,
                              const PblVector3& vNormal, int iBodyId)
{
    if (mBeginTime > GameLoop::GetMissionTime())
    {
        mBeginTime = GameLoop::GetMissionTime();
    }

    if (mNumBounces >= 4)
    {
        return false;
    }

    OrdnanceGrenadeClass* pClass = (OrdnanceGrenadeClass*)mClass;
    if (pObject->GetCollisionType() == COLL_WATER)
    {
        mVelocity = PblVector3(0.0f, -0.2f, 0.0f);
        mNumBounces = 4;
        pClass->mSoundCollisionWater.Play(&vPos, 0, false, true);
        RedWater::AddWaterRing(vPos, 1.5f, 0.01f);
        WaterEffect::AddWaterEffect(pClass->mImpactEffectWater, vPos);
        mDecrementLifeTime = true;
    }
    else if (pObject->GetCollisionType() == COLL_LEAFPATCH)
    {
        pClass->mSoundCollisionLeaf.Play(&vPos, 0, false, true);
    }
    else
    {
        if (pObject->GetCollisionType() != COLL_TERRAIN && mNumBounces == 0)
        {
            GameObject* pVictim = pObject->GetGameObject();
            if (pVictim)
            {
                PblVector3 vDir;
                D3DXVec3Normalize(&vDir, &mVelocity);
                AsDamageable(pVictim)->ApplyDamage(GetDamage(), vDir, vPos, iBodyId, 0xFFFFFFFF);
            }
        }

        const float fDot = mVelocity.x * vNormal.x + mVelocity.y * vNormal.y + mVelocity.z * vNormal.z;
        if (fDot < 0.0f)
        {
            const float fRebound = pClass->mRebound;
            const float fFriction = pClass->mFriction;
            mPosition = PblVector3(vPos.x + vNormal.x * 0.01f, vPos.y + vNormal.y * 0.01f,
                                   vPos.z + vNormal.z * 0.01f);

            mVelocity.x -= vNormal.x * fDot;
            mVelocity.y -= vNormal.y * fDot;
            mVelocity.z -= vNormal.z * fDot;
            float fSlow = -(fFriction * fDot * (fRebound + 1.0f) /
                            sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                  mVelocity.z * mVelocity.z));
            fSlow = fSlow < 1.0f ? fSlow : 1.0f;
            mVelocity.x -= fSlow * mVelocity.x;
            mVelocity.y -= mVelocity.y * fSlow;
            mVelocity.z -= mVelocity.z * fSlow;

            const float fBounce = fRebound * fDot;
            mVelocity.x -= vNormal.x * fBounce;
            mVelocity.y -= vNormal.y * fBounce;
            mVelocity.z -= vNormal.z * fBounce;
            if (mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z < 1.0f)
            {
                ++mNumBounces;
            }
        }
        else
        {
            mNumBounces = 4;
        }
        pClass->mSoundCollisionOther.Play(&vPos, 0, false, true);
        mDecrementLifeTime = true;
    }

    if (mNumBounces >= 4)
    {
        mGravity = 0.0f;
        mVelocity.z = 0.0f;
        mVelocity.y = 0.0f;
        mVelocity.x = 0.0f;
    }
    return false;
}

// FUNCTION: SWBF2 0x00600980
void OrdnanceGrenade::SoundEvent()
{
    AIUtil::GrenadeAlert(mPosition, GetOwner(), &mBeginTime, &mLastEventTime);
}
