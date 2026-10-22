#include "OrdnanceEmitter.h"
#include "Explosion.h"
#include "FLEffect.h"
#include "AIUtil.h"
#include "Aimer.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "EntityDroideka.h"
#include "EntitySoldier.h"
#include "GameObject.h"
#include "LightningBoltEffect.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern PblVector3 gVectorZero;

static inline const PblMatrix operator*(const PblMatrix& a, const PblMatrix& b)
{
    PblMatrix result;
    D3DXMatrixMultiply(&result, &a, &b);
    return result;
}

// GLOBAL: SWBF2 0x01ef2624
// SYNTHETIC: SWBF2 0x00405a70
// `dynamic initializer for 'OrdnanceEmitterClass::rttiHashOrdnanceEmitterClass''
PblHash OrdnanceEmitterClass::rttiHashOrdnanceEmitterClass("OrdnanceEmitterClass");
char OrdnanceEmitterClass::rttiNameOrdnanceEmitterClass[] = "OrdnanceEmitterClass";

// FUNCTION: SWBF2 0x005fc410
OrdnanceEmitterClass::OrdnanceEmitterClass(unsigned int uHash)
    : OrdnanceClass(uHash)
{
    mFireEffect = 0;
    mLightningEffectClass = 0;
    mLightningEffectScale = 1.0f;
    mNoTargetLightningEffectCount = 1;
    mNoTargetLightningEffectRandomSpread = 0.0f;
    mStunAnimation = 0;
    mStunTime = 0.0f;
    mMinDamage = 0.0f;
    mMaxDamage = 0.0f;
    mDamageThreshold = 0.0f;
    mMaxJumpDistance = 10.0f;
    mJumpDeduction = 0.0f;
    mPush = 0.0f;
    mConeLength = 10.0f;
    mConeWidth = 0.866f;
    mFirstRadius = 1.5f;
    mMaxTargets = 5;
    mNoChaining = false;
    mRadius = 0.0f;
    mBuffOffenseTimer = 0.0f;
    mBuffOffenseMult = 0.0f;
    mBuffDefenseTimer = 0.0f;
    mBuffDefenseMult = 0.0f;
    mBuffHealthTimer = 0.0f;
    mBuffHealthRate = 0.0f;
    mDebuffDamageTimer = 0.0f;
    mDebuffDamageRate = 0.0f;
    mPlayEffectOnOwner = 0;
    mPlayEffectOnOwnerAimer = 0;
    mSmolderEffectTimer = 0.0f;
    mSmolderVanishDeath = false;
    for (int i = 0; i < 5; ++i)
    {
        mSmolderEffectBone[i] = 0;
        mSmolderEffect[i] = 0;
    }
    mSmolderDamageRate = 0.0f;
    mAffectFriends = true;
    mAffectEnemies = true;
    mApplyOnOwnerIfOnOthers = false;
    mExplosion = 0;
}

// FUNCTION: SWBF2 0x005fc620
OrdnanceEmitterClass::OrdnanceEmitterClass(OrdnanceEmitterClass* pBase, unsigned int uHash)
    : OrdnanceClass(pBase, uHash)
{
    mFireEffect = pBase->mFireEffect;
    mLightningEffectClass = pBase->mLightningEffectClass;
    mLightningEffectScale = pBase->mLightningEffectScale;
    mNoTargetLightningEffectCount = pBase->mNoTargetLightningEffectCount;
    mNoTargetLightningEffectRandomSpread = pBase->mNoTargetLightningEffectRandomSpread;
    mStunAnimation = pBase->mStunAnimation;
    mStunTime = pBase->mStunTime;
    mMinDamage = pBase->mMinDamage;
    mPush = pBase->mPush;
    mConeLength = pBase->mConeLength;
    mConeWidth = pBase->mConeWidth;
    mMaxTargets = pBase->mMaxTargets;
    mMaxDamage = pBase->mMaxDamage;
    mDamageThreshold = pBase->mDamageThreshold;
    mMaxJumpDistance = pBase->mMaxJumpDistance;
    mJumpDeduction = pBase->mJumpDeduction;
    mFirstRadius = pBase->mFirstRadius;
    mNoChaining = pBase->mNoChaining;
    mRadius = pBase->mRadius;
    mBuffOffenseTimer = pBase->mBuffOffenseTimer;
    mBuffOffenseMult = pBase->mBuffOffenseMult;
    mBuffDefenseTimer = pBase->mBuffDefenseTimer;
    mBuffDefenseMult = pBase->mBuffDefenseMult;
    mBuffHealthTimer = pBase->mBuffHealthTimer;
    mBuffHealthRate = pBase->mBuffHealthRate;
    mDebuffDamageTimer = pBase->mDebuffDamageTimer;
    mDebuffDamageRate = pBase->mDebuffDamageRate;
    mPlayEffectOnOwner = pBase->mPlayEffectOnOwner;
    mPlayEffectOnOwnerAimer = pBase->mPlayEffectOnOwnerAimer;
    mSmolderEffectTimer = pBase->mSmolderEffectTimer;
    mSmolderVanishDeath = pBase->mSmolderVanishDeath;
    for (int i = 0; i < 5; ++i)
    {
        mSmolderEffectBone[i] = pBase->mSmolderEffectBone[i];
        mSmolderEffect[i] = pBase->mSmolderEffect[i];
    }
    mSmolderDamageRate = pBase->mSmolderDamageRate;
    mAffectFriends = pBase->mAffectFriends;
    mAffectEnemies = pBase->mAffectEnemies;
    mApplyOnOwnerIfOnOthers = pBase->mApplyOnOwnerIfOnOthers;
    mExplosion = pBase->mExplosion;
}

// FUNCTION: SWBF2 0x005fc3b0
bool OrdnanceEmitterClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceEmitterClass || OrdnanceClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005fc3e0
unsigned int OrdnanceEmitterClass::GetDerivedRtti()
{
    return rttiHashOrdnanceEmitterClass;
}

// FUNCTION: SWBF2 0x005fc3f0
const char* OrdnanceEmitterClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceEmitterClass;
}

// FUNCTION: SWBF2 0x005fc400
float OrdnanceEmitterClass::GetRange()
{
    return mConeLength;
}

// FUNCTION: SWBF2 0x005fc8a0
void OrdnanceEmitterClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x6E6E8D54)
    {
        mFireEffect = FLEffect::s_EffectClasses.Find(PblHash(pszValue));
    }
    else if (uProperty == 0x6DD5A894)
    {
        mLightningEffectClass = FLEffect::s_EffectClasses.Find(PblHash(pszValue));
    }
    else if (uProperty == 0x38FFE98E)
    {
        sscanf(pszValue, "%f", &mLightningEffectScale);
    }
    else if (uProperty == 0x63384B21)
    {
        sscanf(pszValue, "%i", &mNoTargetLightningEffectCount);
    }
    else if (uProperty == 0xDD09F94E)
    {
        sscanf(pszValue, "%f", &mNoTargetLightningEffectRandomSpread);
    }
    else if (uProperty == 0x6F2732A9)
    {
        mStunAnimation = PblTEMPHash(pszValue);
    }
    else if (uProperty == 0x8DF1E8C9)
    {
        sscanf(pszValue, "%f", &mStunTime);
    }
    else if (uProperty == 0x59E94C40)
    {
        sscanf(pszValue, "%f", &mMinDamage);
    }
    else if (uProperty == 0x47D0E794)
    {
        sscanf(pszValue, "%f", &mMaxDamage);
        mDamage.mBaseValue = mMaxDamage;
    }
    else if (uProperty == 0xCCA74473)
    {
        sscanf(pszValue, "%f", &mDamageThreshold);
    }
    else if (uProperty == 0xAD37744E)
    {
        sscanf(pszValue, "%f", &mMaxJumpDistance);
    }
    else if (uProperty == 0xFC0D5E74)
    {
        sscanf(pszValue, "%f", &mJumpDeduction);
    }
    else if (uProperty == 0x876FFFDD)
    {
        sscanf(pszValue, "%f", &mPush);
    }
    else if (uProperty == 0x9342844E)
    {
        sscanf(pszValue, "%f", &mConeLength);
    }
    else if (uProperty == 0xA3F03B49)
    {
        sscanf(pszValue, "%f", &mConeWidth);
        mConeWidth = (float)cos(mConeWidth * 0.5f * 0.017453292f);
    }
    else if (uProperty == 0x9CC82B1F)
    {
        sscanf(pszValue, "%f", &mFirstRadius);
    }
    else if (uProperty == 0xA00A48CD)
    {
        sscanf(pszValue, "%i", &mMaxTargets);
    }
    else if (uProperty == 0x4C387CA1)
    {
        mNoChaining = atoi(pszValue) != 0;
    }
    else if (uProperty == 0x0DBA4CB3)
    {
        sscanf(pszValue, "%f", &mRadius);
    }
    else if (uProperty == 0x70B204AF)
    {
        sscanf(pszValue, "%f", &mBuffOffenseTimer);
    }
    else if (uProperty == 0xCE532E64)
    {
        sscanf(pszValue, "%f", &mBuffOffenseMult);
    }
    else if (uProperty == 0x7705FCC1)
    {
        sscanf(pszValue, "%f", &mBuffDefenseTimer);
    }
    else if (uProperty == 0xB25FB62E)
    {
        sscanf(pszValue, "%f", &mBuffDefenseMult);
    }
    else if (uProperty == 0x460B143B)
    {
        sscanf(pszValue, "%f", &mBuffHealthTimer);
    }
    else if (uProperty == 0xB6639618)
    {
        sscanf(pszValue, "%f", &mBuffHealthRate);
    }
    else if (uProperty == 0x805B51E7)
    {
        sscanf(pszValue, "%f", &mDebuffDamageTimer);
    }
    else if (uProperty == 0x9834A7B4)
    {
        sscanf(pszValue, "%f", &mDebuffDamageRate);
    }
    else if (uProperty == 0x76F3BEEA)
    {
        mPlayEffectOnOwner = PblHash(pszValue);
    }
    else if (uProperty == 0xC0E20834)
    {
        mPlayEffectOnOwnerAimer = PblHash(pszValue);
    }
    else if (uProperty == 0x6824F471)
    {
        sscanf(pszValue, "%f", &mSmolderEffectTimer);
    }
    else if (uProperty == 0x348506DC)
    {
        mSmolderVanishDeath = atoi(pszValue) != 0;
    }
    else if (uProperty == 0x6CF895FB)
    {
        int i;
        for (i = 0; i < 5; ++i)
        {
            if (!mSmolderEffectBone[i])
            {
                break;
            }
        }
        mSmolderEffectBone[i] = PblTEMPHash(pszValue);
    }
    else if (uProperty == 0x592FB160)
    {
        int i;
        for (i = 0; i < 5; ++i)
        {
            if (!mSmolderEffect[i])
            {
                break;
            }
        }
        mSmolderEffect[i] = PblHash(pszValue);
    }
    else if (uProperty == 0x1076DFD6)
    {
        sscanf(pszValue, "%f", &mSmolderDamageRate);
    }
    else if (uProperty == 0xAB1376D9)
    {
        mAffectFriends = atoi(pszValue) != 0;
    }
    else if (uProperty == 0xD41EC7E0)
    {
        mAffectEnemies = atoi(pszValue) != 0;
    }
    else if (uProperty == 0x39B3648C)
    {
        mApplyOnOwnerIfOnOthers = atoi(pszValue) != 0;
        OrdnanceClass::SetProperty(uProperty, pszValue);
    }
    else if (uProperty == 0x02BB1FE0)
    {
        mExplosion = ExplosionClass::Find(PblHash(pszValue));
    }
    else
    {
        OrdnanceClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x005fcdf0
OrdnanceClass* OrdnanceEmitterClass::Derive(unsigned int uHash)
{
    return new OrdnanceEmitterClass(this, uHash);
}

static const PblVector3 RotateY(const PblVector3& v, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    return PblVector3(fCos * v.x - fSin * v.z, v.y, fSin * v.x + fCos * v.z);
}

static const PblVector3 RotateZ(const PblVector3& v, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    return PblVector3(fCos * v.x + fSin * v.y, fCos * v.y - fSin * v.x, v.z);
}

static const int kEmitterTargetFilter = COLL_SOFT | COLL_RIGID | COLL_ATTACHED | COLL_SHIELD;
static const int kEmitterRayFilter = COLL_ASTEROID | COLL_RIGID | COLL_STATIC | COLL_TERRAIN;

// FUNCTION: SWBF2 0x005fce60
Ordnance* OrdnanceEmitterClass::Build(const OrdnanceDesc& desc)
{
    PblVector3 vPos = desc.mPosition;
    PblVector3 vDir = desc.mDirection;

    if (desc.mFireWeapon)
    {
        desc.mFireWeapon->SignalFire();
    }

    if (mPlayEffectOnOwner && desc.mOwner)
    {
        FLEffectClass* pEffectClass = FLEffect::s_EffectClasses.Find(mPlayEffectOnOwner);
        if (pEffectClass)
        {
            FLEffectObject* pEffect = pEffectClass->BuildEffect();
            if (pEffect)
            {
                pEffect->AttachEffectToObject(desc.mOwner, true);
            }
        }
    }

    if (mPlayEffectOnOwnerAimer && desc.mOwner)
    {
        FLEffectClass* pEffectClass = FLEffect::s_EffectClasses.Find(mPlayEffectOnOwnerAimer);
        if (pEffectClass)
        {
            FLEffectObject* pEffect = pEffectClass->BuildEffect();
            if (pEffect && desc.mOwner->GetControllable())
            {
                PblMatrix mOffset = desc.mOwner->GetControllable()->GetCurAimer()->GetFireMatrix() *
                                    desc.mOwner->GetMatrix().GetInverseFast();
                pEffect->AttachEffectToObject(desc.mOwner, mOffset, true);
            }
        }
    }

    GameObject* objects[128];
    bool bApplied = false;
    float fLastDamage = 0.0f;

    if (mConeLength > 0.0f && desc.mOwner)
    {
        int iCount = CollisionManager::GetObjectsInRange(vPos, mConeLength, objects, 96, desc.mOwner,
                                                         kEmitterTargetFilter);
        if (mFireEffect)
        {
            FLEffectObject* pEffect = mFireEffect->BuildEffect();
            if (pEffect)
            {
                pEffect->AttachEffectToVector(&vPos, vDir, false);
            }
        }

        float fConeLengthSq = mConeLength * mConeLength;
        float fNearConeWidth = 0.1f;
        if (mConeWidth <= 0.1f)
        {
            fNearConeWidth = mConeWidth;
        }

        PblHeap<GameObject*, float> heap(96, 1000000.0f, 0);
        for (int i = iCount - 1; i >= 0; --i)
        {
            CollisionObject* pCollision = objects[i]->GetCollisionObject();
            if (!pCollision || !pCollision->GetGameObject() || !pCollision->GetGameObject()->IsAlive())
            {
                continue;
            }
            if (desc.mOwner->GetAffiliationToMe(pCollision->GetGameObject()) < 0)
            {
                if (!mAffectEnemies)
                {
                    continue;
                }
            }
            else if (!mAffectFriends)
            {
                continue;
            }

            PblVector3 vTarget = objects[i]->GetCollisionCenter();
            PblVector3 vDelta(vTarget.x - vPos.x, vTarget.y - vPos.y, vTarget.z - vPos.z);
            float fDistSq = vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z;
            if (fConeLengthSq > fDistSq)
            {
                PblVector3 vToTarget;
                D3DXVec3Normalize(&vToTarget, &vDelta);
                float fWidth = fDistSq < 6.25f ? fNearConeWidth : mConeWidth;
                if (vDir.x * vToTarget.x + vToTarget.y * vDir.y + vToTarget.z * vDir.z > fWidth)
                {
                    heap.Push(objects[i], -(float)sqrt((double)fDistSq));
                }
            }
        }

        PblVector3 vSource = vPos;
        int iTargets = 0;
        float fChainDist = 0.0f;
        float fDamage = GetDamage(desc.mStrength);
        if (mNoChaining && heap.GetCount() > 0)
        {
            fDamage = fDamage / (float)heap.GetCount();
            if (mMinDamage > fDamage)
            {
                fDamage = mMinDamage;
            }
        }

        while (!heap.IsEmpty())
        {
            if (iTargets >= mMaxTargets || fDamage < mDamageThreshold)
            {
                break;
            }

            float fTargetDist = -heap.Value(0);
            GameObject* pTarget = heap.Get(0);
            heap.Pop();

            if (!mNoChaining)
            {
                float fMaxJump = iTargets ? mMaxJumpDistance : mConeLength;
                if (fTargetDist - fChainDist > fMaxJump)
                {
                    break;
                }
            }

            TreeGridObject* pTreeGrid = pTarget;
            PblVector3 vTarget = pTreeGrid->GetCollisionCenter();
            if (CollisionManager::RayTest(vSource, vTarget, 0, 0, kEmitterRayFilter))
            {
                continue;
            }

            PblVector3 vDelta(vTarget.x - vSource.x, vTarget.y - vSource.y, vTarget.z - vSource.z);
            PblVector3 vJumpDir;
            D3DXVec3Normalize(&vJumpDir, &vDelta);

            if (pTarget->Deflect(0, vTarget, vJumpDir))
            {
                if (mLightningEffectClass)
                {
                    LightningBoltEffectObject* pBolt =
                        static_cast<LightningBoltEffectObject*>(mLightningEffectClass->BuildEffect());
                    if (pBolt)
                    {
                        if (iTargets > 0 || !pBolt->SetOrdnanceOwner(desc.mOwner))
                        {
                            pBolt->AttachEffectToVector(&vSource, false);
                        }
                        vTarget.x -= vJumpDir.x * 0.75f;
                        vTarget.y -= vJumpDir.y * 0.75f;
                        vTarget.z -= vJumpDir.z * 0.75f;
                        pBolt->SetTargetPoint(vTarget);
                        pBolt->SetStrength(mLightningEffectScale * desc.mStrength);
                    }
                }
            }
            else
            {
                if (!mNoChaining)
                {
                    if (mFirstRadius > 0.0f && iTargets == 0)
                    {
                        float fAlong = (vTarget.y - vPos.y) * vDir.y + (vTarget.x - vPos.x) * vDir.x +
                                       (vTarget.z - vPos.z) * vDir.z;
                        float fDx = fAlong * vDir.x + vPos.x - vTarget.x;
                        float fDy = fAlong * vDir.y + vPos.y - vTarget.y;
                        float fDz = fAlong * vDir.z + vPos.z - vTarget.z;
                        if (fDx * fDx + fDy * fDy + fDz * fDz > mFirstRadius * mFirstRadius)
                        {
                            continue;
                        }
                    }
                    fChainDist = fTargetDist;
                }

                bool bExplode = true;
                if (fDamage > 0.0f)
                {
                    EntitySoldier* pSoldier = pTarget->GetDerivedRtti() == EntitySoldier::rttiHashEntitySoldier
                                                  ? static_cast<EntitySoldier*>(pTarget)
                                                  : 0;
                    if (pSoldier && mSmolderEffectTimer > 0.0f)
                    {
                        for (int i = 0; i < 5; ++i)
                        {
                            pSoldier->SetupSmolderEffect(i, mSmolderEffectBone[i], mSmolderEffect[i]);
                        }
                        pSoldier->ActivateSmolder(mSmolderEffectTimer, mSmolderVanishDeath, mSmolderDamageRate,
                                                  desc.mDamageOwner);
                    }
                    DamageDesc damage = mDamage;
                    damage.mDamageOwner = desc.mDamageOwner;
                    damage.mBaseValue = fDamage;
                    pTarget->ApplyDamage(damage, 0xFFFFFFFF);
                    bApplied = true;
                    fLastDamage = fDamage;
                    if (pSoldier && !pTarget->IsAlive() && ((CollisionObject*)pTarget)->GetCollisionType() == COLL_ATTACHED)
                    {
                        pSoldier->DirectionKill(gVectorZero);
                    }
                    bExplode = false;
                    if (mExplosion)
                    {
                        mExplosion->CreateExplosion(pTreeGrid->GetCollisionCenter(), desc.mDamageOwner, false,
                                                    COLL_UNKNOWN);
                    }
                }
                bApplied |= DoBuff(pTarget, desc, bExplode);

                float fPush = mPush;
                if (fPush > 0.0f)
                {
                    pTarget->ApplyRadiusPush(vPos, mConeLength * 0.5f, mConeLength * 2.0f, fPush, true);
                }
                else if (fPush < 0.0f)
                {
                    float fDx = vTarget.x - vPos.x;
                    float fDy = vTarget.y - vPos.y;
                    float fDz = vTarget.z - vPos.z;
                    float fLength = sqrtf(fDx * fDx + fDy * fDy + fDz * fDz);
                    if (fLength > 0.0f)
                    {
                        float fInvLength = 1.0f / fLength;
                        fDx *= fInvLength;
                        fDy *= fInvLength;
                        fDz *= fInvLength;
                    }
                    float fRadius = mConeLength * 1.1f;

                    PblVector3 vPushPos(fDx * fRadius + vPos.x + 0.0f, fDy * fRadius + vPos.y + 0.0f,
                                        fDz * fRadius + vPos.z + 0.0f);
                    pTarget->ApplyRadiusPush(vPushPos, 0.0f, fRadius, -fPush, true);
                }

                if (mLightningEffectClass)
                {
                    LightningBoltEffectObject* pBolt =
                        static_cast<LightningBoltEffectObject*>(mLightningEffectClass->BuildEffect());
                    if (pBolt)
                    {
                        if (iTargets > 0 || !pBolt->SetOrdnanceOwner(desc.mOwner))
                        {
                            pBolt->AttachEffectToVector(&vSource, false);
                        }
                        pBolt->SetTargetPoint(vTarget);
                        pBolt->SetStrength(mLightningEffectScale * desc.mStrength);
                        if (!mNoChaining && (!iTargets || PblRandom::GetGlobalFloat() > 0.3f))
                        {
                            vSource = vTarget;
                        }
                    }
                }
            }

            ++iTargets;
            if (!mNoChaining)
            {
                fDamage -= mJumpDeduction;
            }
        }

        if (mLightningEffectClass)
        {
            for (int iBolts = mNoTargetLightningEffectCount - iTargets; iBolts > 0; --iBolts)
            {
                LightningBoltEffectObject* pBolt =
                    static_cast<LightningBoltEffectObject*>(mLightningEffectClass->BuildEffect());
                if (!pBolt)
                {
                    continue;
                }

                PblVector3 vBoltDir;
                if (mNoTargetLightningEffectRandomSpread == 0.0f)
                {
                    vBoltDir = vDir;
                }
                else
                {
                    vBoltDir = RotateZ(RotateY(vDir, (1.0f - PblRandom::_Global.GetFloat() * 2.0f) *
                                                         mNoTargetLightningEffectRandomSpread),
                                       (1.0f - PblRandom::_Global.GetFloat() * 2.0f) *
                                           mNoTargetLightningEffectRandomSpread);
                }

                CollisionObject* pHit;
                float fFraction = CollisionManager::RayHit(vPos, vBoltDir, mConeLength, pHit, 0, 0, 0,
                                                           kEmitterRayFilter, true);
                if (!pBolt->SetOrdnanceOwner(desc.mOwner))
                {
                    pBolt->AttachEffectToVector(&vPos, false);
                }
                float fLength = mConeLength * fFraction;
                pBolt->SetTargetPoint(PblVector3(vPos.x + fLength * vBoltDir.x, fLength * vBoltDir.y + vPos.y,
                                                 fLength * vBoltDir.z + vPos.z));
                pBolt->SetStrength(mLightningEffectScale * desc.mStrength);
            }
        }
    }

    if (mRadius > 0.0f && desc.mOwner && desc.mOwner->GetControllable())
    {
        int iCount = CollisionManager::GetObjectsInRange(desc.mPosition, mRadius, objects, 128, desc.mOwner,
                                                         kEmitterTargetFilter);
        float fDamage = iCount ? GetDamage(desc.mStrength) / (float)iCount : 0.0f;
        if (mMinDamage > fDamage)
        {
            fDamage = mMinDamage;
        }

        for (int i = 0; i < iCount; ++i)
        {
            GameObject* pTarget = objects[i];
            if (!pTarget || !pTarget->IsAlive())
            {
                continue;
            }
            const PblVector3& vTarget = pTarget->GetCollisionCenter();
            if (!AIUtil::CanSeePosition(desc.mOwner->GetControllable()->GetEyePoint(),
                                        desc.mOwner->GetControllable(), vTarget, pTarget))
            {
                continue;
            }
            if (desc.mOwner->GetAffiliationToMe(pTarget) < 0)
            {
                if (!mAffectEnemies)
                {
                    continue;
                }
            }
            else if (!mAffectFriends)
            {
                continue;
            }

            EntitySoldier* pSoldier = pTarget->GetDerivedRtti() == EntitySoldier::rttiHashEntitySoldier
                                          ? static_cast<EntitySoldier*>(pTarget)
                                          : 0;
            if (pSoldier && mSmolderEffectTimer > 0.0f)
            {
                for (int j = 0; j < 5; ++j)
                {
                    pSoldier->SetupSmolderEffect(j, mSmolderEffectBone[j], mSmolderEffect[j]);
                }
                pSoldier->ActivateSmolder(mSmolderEffectTimer, mSmolderVanishDeath, mSmolderDamageRate,
                                          desc.mDamageOwner);
            }

            bool bExplode;
            if (fDamage > 0.0f)
            {
                DamageDesc damage = mDamage;
                damage.mDamageOwner = desc.mDamageOwner;
                damage.mBaseValue = fDamage;
                pTarget->ApplyDamage(damage, 0xFFFFFFFF);
                bApplied = true;
                fLastDamage = fDamage;
                if (pSoldier && !pTarget->IsAlive() && pTarget->GetObjectData(0) == COLL_ATTACHED)
                {
                    pSoldier->DirectionKill(gVectorZero);
                }
                bExplode = false;
                if (mExplosion)
                {
                    mExplosion->CreateExplosion(pTarget->GetCollisionCenter(), desc.mDamageOwner, false,
                                                COLL_UNKNOWN);
                }
            }
            else
            {
                bExplode = true;
            }
            bApplied |= DoBuff(pTarget, desc, bExplode);
        }
    }

    if (bApplied && mApplyOnOwnerIfOnOthers)
    {
        GameObject* pOwner = desc.mOwner;
        if (pOwner && pOwner->IsAlive() && mAffectFriends)
        {
            EntitySoldier* pSoldier = pOwner->GetDerivedRtti() == EntitySoldier::rttiHashEntitySoldier
                                          ? static_cast<EntitySoldier*>(pOwner)
                                          : 0;
            if (pSoldier && mSmolderEffectTimer > 0.0f)
            {
                for (int i = 0; i < 5; ++i)
                {
                    pSoldier->SetupSmolderEffect(i, mSmolderEffectBone[i], mSmolderEffect[i]);
                }
                pSoldier->ActivateSmolder(mSmolderEffectTimer, mSmolderVanishDeath, mSmolderDamageRate,
                                          desc.mDamageOwner);
            }

            bool bExplode;
            if (fLastDamage > 0.0f)
            {
                DamageDesc damage = mDamage;
                damage.mDamageOwner = desc.mDamageOwner;
                damage.mBaseValue = fLastDamage;
                pOwner->ApplyDamage(damage, 0xFFFFFFFF);
                if (pSoldier && !pOwner->IsAlive() && pOwner->GetObjectData(0) == COLL_ATTACHED)
                {
                    pSoldier->DirectionKill(gVectorZero);
                }
                bExplode = false;
                if (mExplosion)
                {
                    mExplosion->CreateExplosion(pOwner->GetCollisionCenter(), desc.mDamageOwner, false,
                                                COLL_UNKNOWN);
                }
            }
            else
            {
                bExplode = true;
            }
            DoBuff(pOwner, desc, bExplode);
        }
    }

    return 0;
}

// FUNCTION: SWBF2 0x005fe320
bool OrdnanceEmitterClass::DoBuff(GameObject* pObject, const OrdnanceDesc& desc, bool bExplode)
{
    bool bBuffed = false;
    if (pObject->GetDerivedRtti() == EntitySoldier::rttiHashEntitySoldier)
    {
        if (pObject->IsAlive())
        {
            EntitySoldier* pSoldier = static_cast<EntitySoldier*>(pObject);
            if (mDebuffDamageTimer > 0.0f)
            {
                pSoldier->DebuffDamage(mDebuffDamageTimer, mDebuffDamageRate, desc.mDamageOwner);
            }
            if (mBuffOffenseTimer > 0.0f)
            {
                pSoldier->BuffOffense(mBuffOffenseTimer, mBuffOffenseMult);
            }
            if (mBuffDefenseTimer > 0.0f)
            {
                pSoldier->BuffDefense(mBuffDefenseTimer, mBuffDefenseMult);
            }
            if (mBuffHealthTimer > 0.0f)
            {
                pSoldier->BuffHealth(mBuffHealthTimer, mBuffHealthRate);
            }
            if (mDebuffDamageTimer > 0.0f || mBuffOffenseTimer > 0.0f || mBuffDefenseTimer > 0.0f ||
                mBuffHealthTimer > 0.0f)
            {
                bBuffed = true;
                if (mExplosion && bExplode)
                {
                    mExplosion->CreateExplosion(pObject->GetCollisionCenter(), desc.mDamageOwner, false,
                                                COLL_UNKNOWN);
                }
            }
        }
    }
    else if (pObject->GetDerivedRtti() == EntityDroideka::rttiHashEntityDroideka)
    {
        if (pObject->IsAlive())
        {
            EntityDroideka* pDroideka = static_cast<EntityDroideka*>(pObject);
            if (mDebuffDamageTimer > 0.0f)
            {
                pDroideka->DebuffDamage(mDebuffDamageTimer, mDebuffDamageRate, desc.mDamageOwner);
            }
            if (mBuffOffenseTimer > 0.0f)
            {
                pDroideka->BuffOffense(mBuffOffenseTimer, mBuffOffenseMult);
            }
            if (mBuffDefenseTimer > 0.0f)
            {
                pDroideka->BuffDefense(mBuffDefenseTimer, mBuffDefenseMult);
            }
            if (mBuffHealthTimer > 0.0f)
            {
                pDroideka->BuffHealth(mBuffHealthTimer, mBuffHealthRate);
            }
            if (mDebuffDamageTimer > 0.0f || mBuffOffenseTimer > 0.0f || mBuffDefenseTimer > 0.0f ||
                mBuffHealthTimer > 0.0f)
            {
                bBuffed = true;
                if (mExplosion && bExplode)
                {
                    mExplosion->CreateExplosion(pObject->GetCollisionCenter(), desc.mDamageOwner, false,
                                                COLL_UNKNOWN);
                }
            }
        }
    }
    return bBuffed;
}

// FUNCTION: SWBF2 0x005fe580
float OrdnanceEmitterClass::GetDamage(float fStrength) const
{
    return (mMaxDamage - mMinDamage) * fStrength + mMinDamage;
}

// FUNCTION: SWBF2 0x005fe5a0
DamageDesc OrdnanceEmitterClass::GetDamage(const DamageOwner& owner, float fStrength, float,
                                           float) const
{
    DamageDesc damage = OrdnanceClass::GetDamage();
    damage.mDamageOwner = owner;
    damage.mBaseValue = GetDamage(fStrength);
    return damage;
}

// FUNCTION: SWBF2 0x005fc2e0
PblMatrix Aimer::GetFireMatrix() const
{
    PblVector3 vSide(mDirection.z, 0.0f, -mDirection.x);
    PblVector3 vRight;
    D3DXVec3Normalize(&vRight, &vSide);
    PblVector3 vUp(mDirection.y * vRight.z - mDirection.z * vRight.y,
                   mDirection.z * vRight.x - mDirection.x * vRight.z,
                   mDirection.x * vRight.y - mDirection.y * vRight.x);

    PblMatrix m;
    m.m[0][0] = vRight.x;
    m.m[0][1] = vRight.y;
    m.m[0][2] = vRight.z;
    m.m[0][3] = 0.0f;
    m.m[1][0] = vUp.x;
    m.m[1][1] = vUp.y;
    m.m[1][2] = vUp.z;
    m.m[1][3] = 0.0f;
    m.m[2][0] = mDirection.x;
    m.m[2][1] = mDirection.y;
    m.m[2][2] = mDirection.z;
    m.m[2][3] = 0.0f;
    m.m[3][0] = mFirePos.x;
    m.m[3][1] = mFirePos.y;
    m.m[3][2] = mFirePos.z;
    m.m[3][3] = 1.0f;
    return m;
}

// FUNCTION: SWBF2 0x005fe640
template void PblHeap<GameObject*, float>::Push(GameObject*& obj, float key);

// FUNCTION: SWBF2 0x005fe6a0
template void PblHeap<GameObject*, float>::Pop();

// SYNTHETIC: SWBF2 0x005fc5f0
// OrdnanceEmitterClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005fc890
OrdnanceEmitterClass::~OrdnanceEmitterClass()
{
}
