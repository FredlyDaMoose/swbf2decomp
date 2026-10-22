#include <math.h>

#include "OrdnanceGrapplingHook.h"
#include "Aimer.h"
#include "CollisionManager.h"
#include "CollisionModel.h"
#include "Controllable.h"
#include "EntitySoldier.h"
#include "WeaponGrapplingHook.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

extern PblVector3 gVectorZero;

// GLOBAL: SWBF2 0x01ef2630
// SYNTHETIC: SWBF2 0x00405aa0
// `dynamic initializer for 'OrdnanceGrapplingHook::rttiHashOrdnanceGrapplingHook''
PblHash OrdnanceGrapplingHook::rttiHashOrdnanceGrapplingHook("OrdnanceGrapplingHook");
char OrdnanceGrapplingHook::rttiNameOrdnanceGrapplingHook[] = "OrdnanceGrapplingHook";

// FUNCTION: SWBF2 0x005ff090
bool OrdnanceGrapplingHook::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceGrapplingHook || OrdnanceBullet::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005ff0c0
unsigned int OrdnanceGrapplingHook::GetDerivedRtti() const
{
    return rttiHashOrdnanceGrapplingHook;
}

// FUNCTION: SWBF2 0x005ff0d0
const char* OrdnanceGrapplingHook::GetDerivedRttiName() const
{
    return rttiNameOrdnanceGrapplingHook;
}

// FUNCTION: SWBF2 0x005ff160
OrdnanceGrapplingHook::OrdnanceGrapplingHook(OrdnanceGrapplingHookClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceBullet(pClass, desc),
      mWeaponOffset(gVectorZero)
{
    if (desc.mFireWeapon->IsRtti(WeaponGrapplingHook::rttiHashWeaponGrapplingHook))
    {
        mFireWeapon = (WeaponGrapplingHook*)desc.mFireWeapon;
        PblMatrix inverse = mOwner->GetMatrix().GetInverseFast();
        D3DXVec3TransformCoord(&mWeaponOffset, &mFireWeapon->mAimer->mFirePos, &inverse);
    }
    else
    {
        mFireWeapon = NULL;
    }

    if (mOwner && mOwner->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        ((EntitySoldier*)(GameObject*)mOwner)->SetGrapplingHook(this);
    }

    mState = HOOK_ATTACHING;
    mMaxLifetime = -1.0f;
    mWaitTime = 0.3f;
    mDeltaTime = 0.0f;
    mCollBodyID = -1;
    mCollOffset = gVectorZero;
}

// SYNTHETIC: SWBF2 0x005ff0e0
// OrdnanceGrapplingHook::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00600011
// [thunk]:OrdnanceGrapplingHook::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x00600019
// [thunk]:OrdnanceGrapplingHook::`scalar deleting destructor' adjustor{152}
// FUNCTION: SWBF2 0x005ff360
OrdnanceGrapplingHook::~OrdnanceGrapplingHook()
{
    if (mOwner && mFireWeapon)
    {
        if (mOwner->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            EntitySoldier* pSoldier = (EntitySoldier*)(GameObject*)mOwner;
            pSoldier->LockInput(-1.0f);
            pSoldier->SetGrapplingHook(NULL);
        }
        mFireWeapon->SetOrdnance(NULL);
        mFireWeapon = NULL;
    }
}

// FUNCTION: SWBF2 0x005ff480
void OrdnanceGrapplingHook::HookFailure()
{
    if (mState == HOOK_ABORTING)
    {
        return;
    }

    mState = HOOK_ABORTING;
    CollisionManager::RemoveOrdnance(this);
    mVelocity = PblVector3(-mVelocity.x, -mVelocity.y, -mVelocity.z);
    mFinalCollisionOnNextUpdate = false;
    mDecrementLifeTime = false;
    mLifeTime = 20.0f;
}

// FUNCTION: SWBF2 0x005ff500
bool OrdnanceGrapplingHook::Collide(CollisionObject* pObject, const PblVector3& vPos,
                                    const PblVector3& vNormal, int iBodyId)
{
    const PblVector3& vFirePos = mFireWeapon->mAimer->mFirePos;
    PblVector3 vDelta(vPos.x - vFirePos.x, vPos.y - vFirePos.y, vPos.z - vFirePos.z);
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vDelta);

    CollisionObjectType eType = pObject->GetCollisionType();
    if (eType != COLL_STATIC && eType != COLL_RIGID)
    {
        HookFailure();
        return false;
    }

    mState = HOOK_WAITING;
    mHitObject = pObject->GetGameObject();
    if (mHitObject)
    {
        PblMatrix world = pObject->mCollisionModel->GetWorldMatrix(iBodyId, mHitObject->GetMatrix(),
                                                                   pObject->GetPose());
        mCollBodyID = iBodyId;
        PblMatrix inverse = world.GetInverseFast();
        D3DXVec3TransformCoord(&mCollOffset, &vPos, &inverse);
    }

    CollisionManager::RemoveOrdnance(this);
    mFinalCollisionOnNextUpdate = false;
    mDecrementLifeTime = false;
    mLifeTime = 20.0f;
    mVelocity = PblVector3(mVelocity.x * 0.5f, mVelocity.y * 0.5f, mVelocity.z * 0.5f);

    if (mOwner->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        EntitySoldier* pSoldier = (EntitySoldier*)(GameObject*)mOwner;
        if (pSoldier->IsInCollisionList())
        {
            pSoldier->RemoveFromCollisionManager();
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005ff760
void OrdnanceGrapplingHook::MoveSoldier(EntitySoldier* pSoldier, float fDeltaTime)
{
    if (!pSoldier)
    {
        return;
    }

    PblMatrix matrix = pSoldier->GetMatrix();
    if (mState == HOOK_LIFTING)
    {
        matrix.m[3][0] += mVelocity.x * fDeltaTime;
        matrix.m[3][1] += mVelocity.y * fDeltaTime;
        matrix.m[3][2] += mVelocity.z * fDeltaTime;
        matrix.m[3][3] = 1.0f;
        pSoldier->SetPosition(matrix.GetTranslation());
        PblVector3 vOffset(0.0f, 0.0f, 0.0f);
        pSoldier->SetPilotInfo(matrix, vOffset,
                               ((OrdnanceGrapplingHookClass*)mClass)->mAttachSoldierAnimation, true,
                               -1.0f, -1.0f, -1.0f, fDeltaTime);
    }
    pSoldier->SetVelocity(gVectorZero);
}

// FUNCTION: SWBF2 0x005ff8b0
bool OrdnanceGrapplingHook::Update(float fDeltaTime)
{
    if (!mOwner)
    {
        return false;
    }

    mDeltaTime = fDeltaTime;
    if (mCollBodyID >= 0 && mHitObject)
    {
        PblMatrix world = mHitObject->GetCollisionObject()->mCollisionModel->GetWorldMatrix(
            mCollBodyID, mHitObject->GetMatrix(), mHitObject->GetCollisionObject()->GetPose());
        D3DXVec3TransformCoord(&mPosition, &mCollOffset, &world);

        float fSpeed = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                             mVelocity.z * mVelocity.z);
        const PblVector3& vFirePos = mFireWeapon->mAimer->mFirePos;
        PblVector3 vDelta(mPosition.x - vFirePos.x, mPosition.y - vFirePos.y, mPosition.z - vFirePos.z);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vDelta);
        mVelocity = PblVector3(vDir.x * fSpeed, vDir.y * fSpeed, vDir.z * fSpeed);
    }

    EntitySoldier* pSoldier = NULL;
    if (mOwner->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        pSoldier = (EntitySoldier*)(GameObject*)mOwner;
        pSoldier->LockInput(1.0f);
    }

    if (mState == HOOK_LIFTING)
    {
        if (!mFireWeapon->mTrigger->IsUp())
        {
            float fRadius = mOwner->GetCollisionObject()->GetCollisionRadius();
            const PblVector3& vCenter = mOwner->GetCollisionObject()->GetCollisionCenter();
            const PblVector3& vPos = GetPosition();
            float fDx = vCenter.x - vPos.x;
            float fDz = vCenter.z - vPos.z;
            if (fRadius * 2.0f <= sqrtf(fDz * fDz + fDx * fDx))
            {
                return true;
            }
        }

        if (!pSoldier)
        {
            return false;
        }
        if (pSoldier->IsInCollisionList())
        {
            pSoldier->RemoveFromCollisionManager();
        }
        pSoldier->AddSoftToCollisionManager();
        pSoldier->SetVelocity(gVectorZero);
        static_cast<Controllable*>(pSoldier)->Stand();
        return false;
    }

    if (mState == HOOK_ABORTING)
    {
        const PblVector3& vFirePos = mFireWeapon->mAimer->mFirePos;
        const PblVector3& vPos = GetPosition();
        float fDx = vFirePos.x - vPos.x;
        float fDy = vFirePos.y - vPos.y;
        float fDz = vFirePos.z - vPos.z;
        float fDist = sqrtf(fDy * fDy + fDx * fDx + fDz * fDz);
        float fStep = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                            mVelocity.z * mVelocity.z) * fDeltaTime;
        if (fStep > fDist)
        {
            return false;
        }
    }
    else if (mState == HOOK_WAITING)
    {
        mWaitTime -= fDeltaTime;
        if (mWaitTime < 0.0f)
        {
            mState = HOOK_LIFTING;
        }
        return true;
    }

    if (mCollBodyID >= 0 && !mHitObject)
    {
        return false;
    }
    if (mFireWeapon->mTrigger->IsUp())
    {
        return false;
    }

    if (!OrdnanceBullet::Update(fDeltaTime) && mState != HOOK_LIFTING && mState != HOOK_WAITING)
    {
        HookFailure();
    }
    if (mMaxLifetime < 0.0f)
    {
        mMaxLifetime = mLifeTime;
    }
    return true;
}

// GLOBAL: SWBF2 0x01ef2634
// SYNTHETIC: SWBF2 0x00405ab0
// `dynamic initializer for 'OrdnanceGrapplingHookClass::rttiHashOrdnanceGrapplingHookClass''
PblHash OrdnanceGrapplingHookClass::rttiHashOrdnanceGrapplingHookClass("OrdnanceGrapplingHookClass");
char OrdnanceGrapplingHookClass::rttiNameOrdnanceGrapplingHookClass[] = "OrdnanceGrapplingHookClass";

// FUNCTION: SWBF2 0x005ffe30
OrdnanceGrapplingHookClass::OrdnanceGrapplingHookClass(unsigned int uHash)
    : OrdnanceBulletClass(uHash)
{
    mAttachSoldierAnimation = SoldierAnimationBank::INVALID_ANIMATION;
}

// FUNCTION: SWBF2 0x005ffe90
OrdnanceGrapplingHookClass::OrdnanceGrapplingHookClass(OrdnanceGrapplingHookClass* pBase, unsigned int uHash)
    : OrdnanceBulletClass(pBase, uHash)
{
    mAttachSoldierAnimation = pBase->mAttachSoldierAnimation;
}

// FUNCTION: SWBF2 0x005ff110
bool OrdnanceGrapplingHookClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceGrapplingHookClass || OrdnanceBulletClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005ff140
unsigned int OrdnanceGrapplingHookClass::GetDerivedRtti()
{
    return rttiHashOrdnanceGrapplingHookClass;
}

// FUNCTION: SWBF2 0x005ff150
const char* OrdnanceGrapplingHookClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceGrapplingHookClass;
}

// FUNCTION: SWBF2 0x005fffe0
void OrdnanceGrapplingHookClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x5F0CE10D)
    {
        mAttachSoldierAnimation = SoldierAnimationBank::AddAnimation(pszValue);
    }
    else
    {
        OrdnanceBulletClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x005ffed0
OrdnanceClass* OrdnanceGrapplingHookClass::Derive(unsigned int uHash)
{
    return new OrdnanceGrapplingHookClass(this, uHash);
}

// FUNCTION: SWBF2 0x005fff40
Ordnance* OrdnanceGrapplingHookClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceGrapplingHook* pHook = new OrdnanceGrapplingHook(this, desc);
    pHook->ActivateThread(-40, PER_LOCAL_TURN);
    pHook->AddOrdnanceToCollisionManager();
    return pHook;
}

// GLOBAL: SWBF2 0x01eaed5c
unsigned int OrdnanceGrapplingHook::sTextureHash;

// SYNTHETIC: SWBF2 0x005ffe60
// OrdnanceGrapplingHookClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005ffec0
OrdnanceGrapplingHookClass::~OrdnanceGrapplingHookClass()
{
}
