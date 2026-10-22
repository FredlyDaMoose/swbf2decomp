#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "EntityMine.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Damageable.h"
#include "Explosion.h"
#include "NetGame.h"
#include "StickInfo.h"
#include "CollisionModel.h"
#include "Controllable.h"
#include "EntitySoldier.h"
#include "GameSound.h"
#include "FLEffect.h"
#include "ItemTimerDisplay.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "SoundFL/source/sndvoicevirtual.h"

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

extern PblVector3 gVectorZero;

PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos);

// FUNCTION: SWBF2 0x004d08b0
EntityMineClass::EntityMineClass(unsigned int uHash)
    : EntityItemClass(uHash)
{
    mIsNeutralWhenEmpty = false;
    mExplosionTrigger = 0;
    mExplosionExpire = 0;
    mExplosionDeath = 0;
    mTriggerContact = false;
    mTriggerRadius = 0.0f;
    mSuppressRadius = 0.0f;
    mTickPitch = 1.0f;
}

// GLOBAL: SWBF2 0x01ebbf90
// SYNTHETIC: SWBF2 0x00402c70
// `dynamic initializer for 'EntityMineClass::rttiHashEntityMineClass''
PblHash EntityMineClass::rttiHashEntityMineClass("EntityMineClass");
char EntityMineClass::rttiNameEntityMineClass[] = "EntityMineClass";

// FUNCTION: SWBF2 0x004cf580
bool EntityMineClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityMineClass || EntityItemClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004cf5c0
unsigned int EntityMineClass::GetDerivedRtti()
{
    return rttiHashEntityMineClass;
}

// FUNCTION: SWBF2 0x004cf5d0
const char* EntityMineClass::GetDerivedRttiName()
{
    return rttiNameEntityMineClass;
}

// FUNCTION: SWBF2 0x004d0aa0
Entity* EntityMineClass::Build(const EntityDesc& desc)
{
    if (EntityMine::sMemoryPool.mUsed >= EntityMine::sMemoryPool.mCount)
    {
        return 0;
    }

    EntityMine* pMine = new EntityMine(this, desc);
    if (pMine)
    {
        CollisionManager::AddItemBody(static_cast<CollisionObject*>(pMine));

        pMine->ActivatePhysics();

        pMine->BuildEffects();
    }
    return pMine;
}

// SYNTHETIC: SWBF2 0x004d0930
// EntityMineClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004d0960
EntityMineClass::EntityMineClass(EntityMineClass* pBase, unsigned int uHash)
    : EntityItemClass(pBase, uHash)
{
    mExplosionTrigger = pBase->mExplosionTrigger;
    mExplosionExpire = pBase->mExplosionExpire;
    mExplosionDeath = pBase->mExplosionDeath;
    mTriggerContact = pBase->mTriggerContact;
    mTriggerRadius = pBase->mTriggerRadius;
    mSuppressRadius = pBase->mSuppressRadius;
    mTick = pBase->mTick;
    mTickPitch = pBase->mTickPitch;
}

// FUNCTION: SWBF2 0x004d0a20
EntityMineClass::~EntityMineClass()
{
}

// FUNCTION: SWBF2 0x004d0a30
EntityClass* EntityMineClass::Derive(unsigned int uHash)
{
    return new EntityMineClass(this, uHash);
}

// FUNCTION: SWBF2 0x004d0b50
void EntityMineClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x02BB1FE0 ||
        uProperty == 0x6A5E371C ||
        uProperty == 0x9B30E763)
    {
        mExplosionTrigger = ExplosionClass::Find(PblHash(pszValue));
    }
    else if (uProperty == 0x2E51081F)
    {
        mExplosionExpire = ExplosionClass::Find(PblHash(pszValue));
    }
    else if (uProperty == 0xA2723442)
    {
        mExplosionDeath = ExplosionClass::Find(PblHash(pszValue));
    }
    else if (uProperty == 0xACFE0D9B)
    {
        mTick.SetID(pszValue, false, false);
    }
    else if (uProperty == 0x18953967)
    {
        sscanf("%f", pszValue, &mTickPitch);
    }
    else if (uProperty == 0xBFB22E35)
    {
        mTriggerContact = atoi(pszValue) != 0;
    }
    else if (uProperty == 0xC461D811)
    {
        sscanf(pszValue, "%f", &mTriggerRadius);
    }
    else if (uProperty == 0xCE754894)
    {
        sscanf(pszValue, "%f", &mSuppressRadius);
    }
    else
    {
        EntityItemClass::SetProperty(uProperty, pszValue);
    }
}

// GLOBAL: SWBF2 0x01ebbf8c
// SYNTHETIC: SWBF2 0x00402c60
// `dynamic initializer for 'EntityMine::rttiHashEntityMine''
PblHash EntityMine::rttiHashEntityMine("EntityMine");
char EntityMine::rttiNameEntityMine[] = "EntityMine";

// GLOBAL: SWBF2 0x01ebbf98
// SYNTHETIC: SWBF2 0x00402c40
// `dynamic initializer for 'EntityMine::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f80
// `dynamic atexit destructor for 'EntityMine::sMemoryPool''
MemoryPool EntityMine::sMemoryPool("EntityMine", 0x2a0);

// FUNCTION: SWBF2 0x004cf5e0
EntityMine::EntityMine(EntityMineClass* pClass, const EntityDesc& desc)
    : EntityItem(pClass, desc)
{
    mNetDead = false;
    mPrevTimerVal = 0;
    mStickInfo = 0;
    mHitSurface = 0;
    mTrigger = false;
    NetGame::Created(this);
}

// FUNCTION: SWBF2 0x004cf690
EntityMine::~EntityMine()
{
    ItemTimerDisplay::RemoveItemTimer(this);
    NetGame::Destroyed(this);
    if (mStickInfo)
    {
        StickInfo::sMemoryPool.Free(mStickInfo);
    }
}

// SYNTHETIC: SWBF2 0x004cf550
// EntityMine::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004d0cc9
// [thunk]:EntityMine::`vector deleting destructor'`adjustor{148}'
// SYNTHETIC: SWBF2 0x004d0cd4
// [thunk]:EntityMine::`vector deleting destructor'`adjustor{320}'
// SYNTHETIC: SWBF2 0x004d0cdf
// [thunk]:EntityMine::`vector deleting destructor'`adjustor{12}'
// SYNTHETIC: SWBF2 0x004d0ce7
// [thunk]:EntityMine::`vector deleting destructor'`adjustor{512}'
// SYNTHETIC: SWBF2 0x004d0cf2
// [thunk]:EntityMine::`vector deleting destructor'`adjustor{576}'

// FUNCTION: SWBF2 0x004cf4c0
bool EntityMine::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityMine ||
           EntityItem::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004cf510
unsigned int EntityMine::GetDerivedRtti() const
{
    return rttiHashEntityMine;
}

// FUNCTION: SWBF2 0x004cf520
const char* EntityMine::GetDerivedRttiName() const
{
    return rttiNameEntityMine;
}

// FUNCTION: SWBF2 0x004cf540
EntityType EntityMine::GetEntityType() const
{
    return static_cast<EntityType>(8);
}

// FUNCTION: SWBF2 0x004cf730
void EntityMine::Kill()
{
    mTrigger = true;
}

// FUNCTION: SWBF2 0x004cf740
void EntityMine::ClientKill()
{
    const CollisionObject* pCollision = static_cast<const CollisionObject*>(this);
    EntityMineClass* pClass = static_cast<EntityMineClass*>(mClass);

    if (mLifeTimer <= 0.1f)
    {
        if (pClass->mExplosionExpire)
        {
            pClass->mExplosionExpire->CreateExplosion(
                pCollision->GetPosition(),
                DamageOwner(mOwner, reinterpret_cast<GameObject*>(this), mTeam, mClass, 0, 1.0f),
                false, COLL_UNKNOWN);
        }
    }
    else if (mIsAlive)
    {
        if (pClass->mExplosionTrigger)
        {
            pClass->mExplosionTrigger->CreateExplosion(
                pCollision->GetPosition(),
                DamageOwner(mOwner, reinterpret_cast<GameObject*>(this), mTeam, mClass, 0, 1.0f),
                false, COLL_UNKNOWN);
        }
    }
    else
    {
        if (pClass->mExplosionDeath)
        {
            pClass->mExplosionDeath->CreateExplosion(
                pCollision->GetPosition(),
                DamageOwner(mOwner, reinterpret_cast<GameObject*>(this), mTeam, mClass, 0, 1.0f),
                false, COLL_UNKNOWN);
        }
    }
}

// FUNCTION: SWBF2 0x004cf890
bool EntityMine::Update(float fDeltaTime)
{
    const bool bClient = NetGame::IsEnabled() && NetGame::IsClient();

    if (mStickInfo)
    {
        GameObject* pStuckTo = mStickInfo->mCollHandle.mObject;
        if (pStuckTo && pStuckTo->mHandleId != mStickInfo->mCollHandle.mSavedHandleId)
        {
            mParent = 0;
            delete mStickInfo;
            mStickInfo = 0;
            mStationary = false;
        }
        else
        {
            PblQuaternion qRot;
            PblMatrix mStick;
            D3DXMatrixRotationQuaternion(&mStick, D3DXQuaternionNormalize(&qRot, &mStickInfo->mLocalRotation));
            mStick.m[3][0] = mStickInfo->mLocalPosition.x;
            mStick.m[3][1] = mStickInfo->mLocalPosition.y;
            mStick.m[3][2] = mStickInfo->mLocalPosition.z;
            mStick.m[3][3] = 1.0f;
            CollisionObject* pObject = mStickInfo->mCollObject;
            if (pObject->mCollisionModel)
            {
                PblMatrix mBody = pObject->mCollisionModel->GetWorldMatrix(mStickInfo->mCollBodyID,
                                                                           pObject->GetMatrix(), pObject->GetPose());
                D3DXMatrixMultiply(&mStick, &mStick, &mBody);
            }
            SetMatrix(mStick);
            if (fDeltaTime > 0.0f)
            {
                const float fInv = 1.0f / fDeltaTime;
                mVelocity.x = (mMatrix.m[3][0] - mOldPosition.x) * fInv;
                mVelocity.y = (mMatrix.m[3][1] - mOldPosition.y) * fInv;
                mVelocity.z = (mMatrix.m[3][2] - mOldPosition.z) * fInv;
                mOldPosition = *(const PblVector3*)mMatrix.m[3];
            }
            mStationary = true;
        }
    }

    if (mNetDead)
    {
        return true;
    }

    EntityMineClass* pClass = static_cast<EntityMineClass*>(mClass);
    const int iTimer = (int)mLifeTimer;
    if (iTimer != mPrevTimerVal)
    {
        const GameSound* pTick = &pClass->mTick;
        const float fTickPitch = pClass->mTickPitch;
        GameSoundControllable tick;
        tick.Play(pTick, (const PblVector3*)mMatrix.m[3], 0, true);
        Snd::VoiceVirtual* pVoice = tick.GetVoiceVirtual();
        if (pVoice && pClass->mLifeSpan > 0.0f)
        {
            const float fLife = mLifeTimer / pClass->mLifeSpan;
            pVoice->SetPitch(pTick->GetPitch() * ((1.0f - fLife) * fTickPitch + fLife));
        }
        mPrevTimerVal = iTimer;
    }

    if (mLifeTimer <= 1.0f && mTrailEmitterObj)
    {
        mTrailEmitterObj->DeactivateEffect();
    }

    if (mLifeTimer <= fDeltaTime)
    {
        if (!bClient && pClass->mExplosionExpire)
        {
            pClass->mExplosionExpire->CreateExplosion(
                GetPosition(), DamageOwner(mOwner, this, mTeam, mClass, 0, 1.0f), false, COLL_UNKNOWN);
        }
        mNetDead = true;
        return bClient;
    }

    if (!IsAlive())
    {
        if (!bClient && pClass->mExplosionDeath)
        {
            pClass->mExplosionDeath->CreateExplosion(
                GetPosition(), DamageOwner(mOwner, this, mTeam, mClass, 0, 1.0f), false, COLL_UNKNOWN);
        }
        mNetDead = true;
        return bClient;
    }

    if (mStationary && pClass->mExplosionTrigger && pClass->mTriggerRadius > 0.0f)
    {
        PblVector3 vPos = *(const PblVector3*)mMatrix.m[3];
        const float fRange =
            pClass->mTriggerRadius > pClass->mSuppressRadius ? pClass->mTriggerRadius : pClass->mSuppressRadius;
        GameObject* apObjects[64];
        const int iCount = CollisionManager::GetObjectsInRange(vPos, fRange, apObjects, 64, this, 42);
        for (int i = 0; i < iCount; ++i)
        {
            GameObject* pObject = apObjects[i];
            if (IsMyEnemy(pObject) && pObject->IsAlive())
            {
                if (pObject->GetEntityClass()->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass) &&
                    static_cast<EntitySoldierClass*>(
                        static_cast<GameObjectClass*>(pObject->GetEntityClass()))->mImmuneToMines)
                {
                    continue;
                }

                const PblVector3 vCenter = pObject->GetCollisionObject()->GetCollisionCenter();
                const float fRadius = pObject->GetCollisionObject()->GetCollisionRadius();
                const float fDX = vCenter.x - mMatrix.m[3][0];
                const float fDY = vCenter.y - mMatrix.m[3][1];
                const float fDZ = vCenter.z - mMatrix.m[3][2];
                const float fReach = pClass->mTriggerRadius + fRadius;
                if (fReach * fReach > fDY * fDY + fDX * fDX + fDZ * fDZ)
                {
                    const PblVector3& vVelocity = pObject->GetVelocity();
                    if (vVelocity.y * fDY + fDX * vVelocity.x + vVelocity.z * fDZ >= 0.0f)
                    {
                        mTrigger = true;
                    }
                }
            }
            else if (IsMyFriend(pObject) && pObject->IsAlive() && pClass->mSuppressRadius > 0.0f)
            {
                const PblVector3 vCenter = pObject->GetCollisionObject()->GetCollisionCenter();
                const float fRadius = pObject->GetCollisionObject()->GetCollisionRadius();
                const float fDX = vCenter.x - vPos.x;
                const float fDY = vCenter.y - vPos.y;
                const float fDZ = vCenter.z - vPos.z;
                const float fReach = pClass->mSuppressRadius + fRadius;
                if (fReach * fReach > fDY * fDY + fDX * fDX + fDZ * fDZ)
                {
                    mTrigger = false;
                    break;
                }
            }
        }
    }

    if (mTrigger)
    {
        if (mTrailEmitterObj)
        {
            mTrailEmitterObj->DeactivateEffect();
        }
        if (!bClient && pClass->mExplosionTrigger)
        {
            pClass->mExplosionTrigger->CreateExplosion(
                GetPosition(), DamageOwner(mOwner, this, mTeam, mClass, 0, 1.0f), false, COLL_UNKNOWN);
        }
        mNetDead = true;
        return bClient;
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mMatrix.m[3][1] < -500.0f)
    {
        Kill();
        return false;
    }
    return EntityItem::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x004d00e0
bool EntityMine::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor)
{
    if (mStationary)
    {
        if (mStickInfo && mStickInfo->mCollObject == pOther)
        {
            return true;
        }

        EntityMineClass* pClass = static_cast<EntityMineClass*>(mClass);
        if (pClass->mTriggerContact && pClass->mExplosionTrigger)
        {
            GameObject* pObject = pOther->GetGameObject();
            if (pObject)
            {
                FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
                PlayImpactEffects((const PblVector3*)mMatrix.m[3], &mVelocity);
                if (IsMyEnemy(pObject) &&
                    !(pObject->GetEntityClass()->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass) &&
                      static_cast<EntitySoldierClass*>(
                          static_cast<GameObjectClass*>(pObject->GetEntityClass()))->mImmuneToMines))
                {
                    DamageOwner owner(mOwner, this, mTeam, mClass, 0, 1.0f);
                    static_cast<EntityMineClass*>(mClass)->mExplosionTrigger->CreateExplosion(
                        GetPosition(), owner, false, COLL_UNKNOWN);
                    InstantDeath(owner, true);
                    return true;
                }
            }
        }
        return false;
    }

    const bool bResult = CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    EntityMineClass* pClass = static_cast<EntityMineClass*>(mClass);
    const CollisionObjectType eType = pOther->GetCollisionType();
    if (eType == COLL_WATER)
    {
        if (!mHitSurface)
        {
            mHitSurface = 1;
            pClass->mCollisionWaterSound.Play((const PblVector3*)mMatrix.m[3], 0, false, true);
        }
        mVelocity = PblVector3(0.0f, -0.2f, 0.0f);
        return false;
    }
    if (eType == COLL_LEAFPATCH)
    {
        if (!mHitSurface)
        {
            mHitSurface = 1;
            pClass->mCollisionFoliageSound.Play((const PblVector3*)mMatrix.m[3], 0, false, true);
        }
        return false;
    }

    if (mLifeTimer > pClass->mLifeSpan - 0.5f && mOwner && pOther->GetGameObject() &&
        pOther->GetGameObject()->GetControllable() &&
        pOther->GetGameObject()->GetControllable()->mCharacter == mOwner)
    {
        return bResult;
    }

    if ((!pOther->GetGameObject() || pOther->GetGameObject()->mHealthType >= HEALTH_TYPE_VEHICLE) &&
        StickInfo::sMemoryPool.mUsed < StickInfo::sMemoryPool.mCount)
    {
        CollisionModel* pModel = pOther->mCollisionModel;
        PblVector3 vNormal = pResult->mSeparationNormal;
        PblVector3 vDown(-vNormal.x, -vNormal.y, -vNormal.z);
        PblVector3 vStart(mMatrix.m[3][0] + vNormal.x * 0.5f, mMatrix.m[3][1] + vNormal.y * 0.5f,
                          mMatrix.m[3][2] + vNormal.z * 0.5f);
        pOther->RayHit(vStart, vDown, 1.0f, &vNormal, 0, true, 0.0f);
        PblMatrix mStick = NormalMatrix(*(const PblVector3*)mMatrix.m[2], vNormal,
                                      *(const PblVector3*)mMatrix.m[3]);

        mStickInfo = new StickInfo;
        mStickInfo->mCollHandle = pOther->GetGameObject();
        mStickInfo->mCollObject = pOther;
        if (pModel)
        {
            PblMatrix mBody = pModel->GetWorldMatrix(pResult->mReceiverId, pOther->GetMatrix(), pOther->GetPose());
            const PblMatrix mInverse = mBody.GetInverseFast();
            D3DXMatrixMultiply(&mStick, &mStick, &mInverse);
            mStickInfo->mCollBodyID = pResult->mReceiverId;
        }
        else
        {
            mStickInfo->mCollBodyID = -1;
        }
        mStickInfo->mLocalPosition = PblVector3(mStick.m[3][0], mStick.m[3][1], mStick.m[3][2]);
        mStickInfo->mLocalRotation = PblQuaternion(mStick);

        mParent = pOther;
        mVelocity = gVectorZero;
        mOldPosition = *(const PblVector3*)mMatrix.m[3];
        if (!mHitSurface)
        {
            mHitSurface = 1;
            FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
            PlayImpactEffects((const PblVector3*)mMatrix.m[3], &mVelocity);
            pClass->mCollisionSound.Play((const PblVector3*)mMatrix.m[3], 0, false, true);
        }
        mStationary = true;
        ItemTimerDisplay::RemoveItemTimer(this);
        ItemTimerDisplay::AddItemTimer(this);
        return bResult;
    }

    const PblVector3& vNormal = pResult->mSeparationNormal;
    const float fDot = mVelocity.x * vNormal.x + vNormal.y * mVelocity.y + vNormal.z * mVelocity.z;
    if (fDot < 0.0f)
    {
        const float fRebound = pClass->mRebound;
        const float fFriction = pClass->mFriction * fDot * (fRebound + 1.0f);
        mVelocity.x -= vNormal.x * fDot;
        mVelocity.y -= vNormal.y * fDot;
        mVelocity.z -= vNormal.z * fDot;
        float fScale = -(fFriction / sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                           mVelocity.z * mVelocity.z));
        if (!(1.0f > fScale))
        {
            fScale = 1.0f;
        }
        mVelocity.x -= fScale * mVelocity.x;
        mVelocity.y -= mVelocity.y * fScale;
        mVelocity.z -= mVelocity.z * fScale;
        const float fBounce = fRebound * fDot;
        mVelocity.x -= vNormal.x * fBounce;
        mVelocity.y -= vNormal.y * fBounce;
        mVelocity.z -= vNormal.z * fBounce;
        if (!mHitSurface)
        {
            mHitSurface = 1;
            FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
            PlayImpactEffects((const PblVector3*)mMatrix.m[3], &mVelocity);
            pClass->mCollisionSound.Play((const PblVector3*)mMatrix.m[3], 0, false, true);
        }
    }
    return bResult;
}
