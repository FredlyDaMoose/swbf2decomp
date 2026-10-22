#include "EntityPropAnimated.h"
#include "PebbleFL/Common/PblVector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CollisionObject.h"
#include "SoldierAnimationBank.h"
#include "Thread.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "Zephyr/Source/ZephyrAnim.h"
#include "CollisionManager.h"
#include "Damageable.h"
#include "EntitySoldier.h"
#include "FirstPerson.h"
#include "GameModel.h"
#include "Team.h"
#include "RedEngineFL/Graphics/RedModel.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

// TEMPLATE: SWBF2 0x004dc8e0
// PblHandle<GameObject>::operator()

// FUNCTION: SWBF2 0x004dc060
EntityPropAnimatedClass::EntityPropAnimatedClass(unsigned int uHash)
    : EntityPropClass(uHash)
{
    mAnimObj = 0;
    for (int i = 0; i < 5; i++)
    {
        mAnimations[i] = 0;
    }
    for (int i = 0; i < 3; i++)
    {
        mAnimationTriggers[i].mTriggerPoint = 0;
        mAnimationTriggers[i].mTriggerRadius = 0.0f;
        mAnimationTriggers[i].mAnimationID = 0;
    }
    memset(mAttachTriggers, 0, sizeof(mAttachTriggers));
    for (int i = 0; i < 3; i++)
    {
        mAttachTriggers[i].mSoldierAnimation = -1;
    }
    mDisableForCloneWars = false;
    mTrackDeathOnAttach = false;
    mIdleDelay = 0.0f;
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x01ebc044
// SYNTHETIC: SWBF2 0x00402cf0
// `dynamic initializer for 'EntityPropAnimatedClass::rttiHashEntityPropAnimatedClass''
PblHash EntityPropAnimatedClass::rttiHashEntityPropAnimatedClass("EntityPropAnimatedClass");
char EntityPropAnimatedClass::rttiNameEntityPropAnimatedClass[] = "EntityPropAnimatedClass";

// GLOBAL: SWBF2 0x01ebc048
// SYNTHETIC: SWBF2 0x00402ce0
// `dynamic initializer for 'EntityPropAnimated::rttiHashEntityPropAnimated''
PblHash EntityPropAnimated::rttiHashEntityPropAnimated("EntityPropAnimated");
char EntityPropAnimated::rttiNameEntityPropAnimated[] = "EntityPropAnimated";

// FUNCTION: SWBF2 0x004db3c0
EntityPropAnimated::EntityPropAnimated(EntityPropAnimatedClass* pClass, const EntityDesc& desc)
    : EntityProp(pClass, desc), Thread()
{
    mAnimObj = pClass->mAnimObj;
    mAnimatedClass = pClass;
    mBlendFactor = 1.0f;
    mAnimationId = -1;
    if (!pClass->mAnimations[0] || pClass->mDisableForCloneWars)
    {
        mAnimationTime = 0.0f;
    }
    else
    {
        mAnimationTime = pClass->mAnimations[0]->m_u16NumFrames *
                         PblRandom::GetGlobalFloat() / 30.0f;
    }
    if (mShadowProxy)
    {
        mShadowProxy->Deactivate();
    }
    mZephyrSkeleton.Open(*mAnimObj->GetZephyrSkeletonShared());
    mZephyrPoseStatic.Open(mZephyrSkeleton);
    mZephyrPoseDyn.Open(mZephyrSkeleton);
    mAttachedSoldier.mObject = 0;
    mAttachedSoldier.mSavedHandleId = 0;
    mAttachedSoldierAnimation = -1;
    mAttachMatrix = 0;
    mIdleDelayTimer = 0.0f;
}

// FUNCTION: SWBF2 0x004db5d0
EntityPropAnimated::~EntityPropAnimated()
{
}

// FUNCTION: SWBF2 0x004db670
void EntityPropAnimated::Render(unsigned char ucLod, float fAlpha, unsigned int uFlags)
{
    if (mAnimatedClass->m_bDynamicLighting)
    {
        uFlags &= ~0x80u;
    }
    else
    {
        uFlags |= 0x80u;
    }

    static PblHash sPlayerShadow("player_shadow");
    SetupShadow(0, mModel, ucLod, uFlags, sPlayerShadow);
    const GameModel* pModel = GetModel();
    const RedColor kWhite = RedColor::WHITE;
    pModel->Render(ucLod, fAlpha, &mMatrix, &mPose, kWhite,
                   uFlags | 0x01000000, 0);
}

// SYNTHETIC: SWBF2 0x004db340
// EntityPropAnimated::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004992da
// [thunk]:EntityGeometry::GetGameObject`adjustor{12}'
// SYNTHETIC: SWBF2 0x004dc901
// [thunk]:EntityPropAnimated::`vector deleting destructor'`adjustor{384}'
// SYNTHETIC: SWBF2 0x004dc90c
// [thunk]:EntityPropAnimated::`vector deleting destructor'`adjustor{12}'
// SYNTHETIC: SWBF2 0x004dc914
// [thunk]:EntityPropAnimated::`vector deleting destructor'`adjustor{148}'

// FUNCTION: SWBF2 0x004db370
bool EntityPropAnimatedClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityPropAnimatedClass || EntityPropClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004db2a0
bool EntityPropAnimated::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityPropAnimated ||
           EntityProp::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004db3a0
unsigned int EntityPropAnimatedClass::GetDerivedRtti()
{
    return rttiHashEntityPropAnimatedClass;
}

// FUNCTION: SWBF2 0x004db3b0
const char* EntityPropAnimatedClass::GetDerivedRttiName()
{
    return rttiNameEntityPropAnimatedClass;
}

// SYNTHETIC: SWBF2 0x004dc150
// EntityPropAnimatedClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004dc180
EntityPropAnimatedClass::EntityPropAnimatedClass(EntityPropAnimatedClass* pBase, unsigned int uHash)
    : EntityPropClass(pBase, uHash)
{
    mAnimObj = pBase->mAnimObj;
    for (int i = 0; i < 5; i++)
    {
        mAnimations[i] = pBase->mAnimations[i];
    }
    for (int i = 0; i < 3; i++)
    {
        mAnimationTriggers[i] = pBase->mAnimationTriggers[i];
    }
    for (int i = 0; i < 3; i++)
    {
        mAttachTriggers[i] = pBase->mAttachTriggers[i];
    }
    for (int i = 0; i < 3; i++)
    {
        mAttachTriggers[i].mSoldierAnimation = -1;
    }
    mDisableForCloneWars = pBase->mDisableForCloneWars;
    mTrackDeathOnAttach = pBase->mTrackDeathOnAttach;
    mIdleDelay = pBase->mIdleDelay;
}

// FUNCTION: SWBF2 0x004dc2e0
EntityPropAnimatedClass::~EntityPropAnimatedClass()
{
}

// FUNCTION: SWBF2 0x004dc340
bool EntityPropAnimatedClass::InitAnimations(const char* pszName)
{
    mAnimObj = RedAnimation::Find(PblHash(pszName));
    return mAnimObj != 0;
}

// FUNCTION: SWBF2 0x004dc380
bool EntityPropAnimatedClass::SetupAnimation(ZephyrPoseDyn<32>& kPose, int iAnim, float fTime, bool bLoop)
{
    if (iAnim < 0)
    {
        return false;
    }
    const ZephyrAnim* pAnim = mAnimations[iAnim];
    if (!pAnim)
    {
        return false;
    }

    kPose.SetLoop(bLoop);
    float fLength = pAnim->m_u16NumFrames / 30.0f;
    if (fTime > fLength)
    {
        fTime -= (int)(fTime / fLength) * fLength;
    }
    kPose.SetAnimation(pAnim, 30.0f);
    kPose.SetCurTime(fTime);
    kPose.Update(0.0f);
    return true;
}

// FUNCTION: SWBF2 0x004dc430
void EntityPropAnimatedClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x98557a5e:
        InitAnimations(pszValue);
        break;

    case 0xe145ee5d:
        if (mAnimObj)
        {
            for (int i = 0; i < 5; i++)
            {
                if (!mAnimations[i])
                {
                    mAnimations[i] = (ZephyrAnim*)mAnimObj->FindAnimation(pszValue);
                    return;
                }
            }
        }
        break;

    case 0x762ecc6a:
    {
        char acAnimation[256];
        char acSound[256];
        sscanf("%255s %255s\n", acAnimation, acSound);
        unsigned int uAnimation = PblTEMPHash(acAnimation);
        for (int i = 0; i < 5; i++)
        {
            if (mAnimations[i] && uAnimation == mAnimations[i]->GetNameCRC())
            {
                mAnimationSound[i].SetID(acSound, false, false);
                return;
            }
        }
        break;
    }

    case 0x72306628:
    {
        char acAttachPoint[256];
        char acAnimation[256];
        char acSoldierAnimation[256];
        char acUnused[256];
        char acEvent[256];
        float fRadius;
        float fTime;
        sscanf(pszValue, "%s %f %s %s %f %s %s", acAttachPoint, &fRadius, acAnimation,
               acSoldierAnimation, &fTime, acUnused, acEvent);
        for (int i = 0; i < 3; i++)
        {
            AttachTrigger& kTrigger = mAttachTriggers[i];
            if (!kTrigger.mAttachPoint)
            {
                for (int j = 0; j < 5; j++)
                {
                    if (mAnimations[j] && mAnimations[j]->GetNameCRC() == PblTEMPHash(acAnimation))
                    {
                        kTrigger.mAnimationID = j;
                        break;
                    }
                }
                kTrigger.mSoldierAnimation = SoldierAnimationBank::AddAnimation(acSoldierAnimation);
                kTrigger.mAttachPoint = PblTEMPHash(acAttachPoint);
                kTrigger.mAttachRadius = fRadius;
                kTrigger.mAnimationTime = fTime;
                PblTEMPHash(acEvent);
                kTrigger.mPostAttachEvent = 0;
                return;
            }
        }
    }

    case 0x1d56b73b:
    {
        char acAnimation[256];
        char acTriggerPoint[256];
        float fRadius;
        sscanf(pszValue, "%s %s %f", acAnimation, acTriggerPoint, &fRadius);
        for (int i = 0; i < 3; i++)
        {
            AnimationTrigger& kTrigger = mAnimationTriggers[i];
            if (!kTrigger.mTriggerPoint)
            {
                for (int j = 0; j < 5; j++)
                {
                    if (mAnimations[j] && mAnimations[j]->GetNameCRC() == PblTEMPHash(acAnimation))
                    {
                        kTrigger.mAnimationID = j;
                        break;
                    }
                }
                kTrigger.mTriggerPoint = PblTEMPHash(acTriggerPoint);
                kTrigger.mTriggerRadius = fRadius;
                return;
            }
        }
        break;
    }

    case 0x6b014234:
        mDisableForCloneWars = atoi(pszValue) != 0;
        break;

    case 0x0550a3b2:
        mTrackDeathOnAttach = atoi(pszValue) != 0;
        break;

    case 0xf949d898:
        mKillSoldierSound.SetID(pszValue, false, false);
        break;

    case 0xff701082:
        sscanf(pszValue, "%f", &mIdleDelay);
        break;

    default:
        EntityPropClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004dc7e0
EntityClass* EntityPropAnimatedClass::Derive(unsigned int uHash)
{
    return new EntityPropAnimatedClass(this, uHash);
}

// FUNCTION: SWBF2 0x004dc850
Entity* EntityPropAnimatedClass::Build(const EntityDesc& desc)
{
    EntityPropAnimated* pProp = new EntityPropAnimated(this, desc);
    static_cast<CollisionObject*>(pProp)->AddStaticToCollisionManager();
    static_cast<Thread*>(pProp)->ActivateThread(0, PER_LOCAL_TURN);
    pProp->BuildEffects();
    return pProp;
}

// FUNCTION: SWBF2 0x004db2e0
unsigned int EntityPropAnimated::GetRtti()
{
    return rttiHashEntityPropAnimated;
}

// FUNCTION: SWBF2 0x004db2f0
unsigned int EntityPropAnimated::GetDerivedRtti() const
{
    return rttiHashEntityPropAnimated;
}

// FUNCTION: SWBF2 0x004db300
const char* EntityPropAnimated::GetDerivedRttiName() const
{
    return rttiNameEntityPropAnimated;
}

// FUNCTION: SWBF2 0x004db310
EntityClass* EntityPropAnimated::GetEntityClass()
{
    return (EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x004db320
const EntityClass* EntityPropAnimated::GetEntityClass() const
{
    return (const EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x004db330
const RedPose* EntityPropAnimated::GetPose() const
{
    return &mPose;
}

// FUNCTION: SWBF2 0x004db760
void EntityPropAnimated::SetAnimation(unsigned int uAnimationId, bool bLoop)
{
    if (uAnimationId == mAnimationId)
    {
        return;
    }

    mLooping = bLoop;
    mAnimatedClass->SetupAnimation(mZephyrPoseDyn, uAnimationId, mAnimationTime, bLoop);
    if (mBlendFactor >= 1.0f)
    {
        mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
    }
    else
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDyn, mBlendFactor);
    }
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    mPose.ConvertFromZephyrPose(mZephyrPoseStatic);
    mAnimationId = uAnimationId;
}

// FUNCTION: SWBF2 0x004db800
bool EntityPropAnimated::Update(float fDeltaTime)
{
    if (mAnimationId == (unsigned int)-1)
    {
        SetAnimation(0, true);
        mZephyrPoseDyn.Update(0.0f);
        mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
        mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
        mPose.ConvertFromZephyrPose(mZephyrPoseStatic);
    }

    if (mAnimatedClass->mDisableForCloneWars)
    {
        Team* pTeam = Team::GetPtr(1);
        if (pTeam && (pTeam->GetSide() == Team::side_CIS || pTeam->GetSide() == Team::side_Republic))
        {
            return true;
        }
    }

    if (!mLooping)
    {
        const ZephyrAnim* pAnim = mAnimatedClass->mAnimations[mAnimationId];
        if (pAnim && mAnimationTime > pAnim->m_u16NumFrames / 30.0f)
        {
            mAnimationTime = 0.0f;
            mBlendFactor = 0.0f;
            if (mAttachedSoldier)
            {
                GameObject* pSoldier = mAttachedSoldier();
                pSoldier->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
                mAnimatedClass->mKillSoldierSound.Play((const PblVector3*)GetMatrix().m[3], 0, false, true);
            }
            mAttachedSoldier.mObject = 0;
            mAttachedSoldier.mSavedHandleId = 0;
            mAttachedSoldierAnimation = -1;
            mAttachMatrix = 0;
            SetAnimation(0, true);
        }
    }
    else if (mAttachedSoldier)
    {
        mAttachedSoldier->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
        mAnimatedClass->mKillSoldierSound.Play((const PblVector3*)GetMatrix().m[3], 0, false, true);
    }

    for (int i = 0; i < 3; ++i)
    {
        if (!mLooping || mAttachedSoldier)
        {
            continue;
        }
        const EntityPropAnimatedClass::AnimationTrigger& trigger = mAnimatedClass->mAnimationTriggers[i];
        if (!trigger.mTriggerPoint || mAnimationId == trigger.mAnimationID || mBlendFactor < 0.5f)
        {
            continue;
        }
        PblMatrix mBone;
        if (!GetModel()->GetRedModel(0)->GetBoneMatrix(trigger.mTriggerPoint, mBone))
        {
            continue;
        }
        PblMatrix mWorld;
        D3DXMatrixMultiply(&mWorld, &mBone, &mMatrix);
        GameObject* pFound;
        if (CollisionManager::GetObjectsInRange(*(const PblVector3*)mWorld.m[3], trigger.mTriggerRadius,
                                                &pFound, 1, 0, 2) > 0)
        {
            mAnimationTime = 0.0f;
            mBlendFactor = 0.0f;
            SetAnimation(trigger.mAnimationID, false);
            mAnimatedClass->mAnimationSound[i].Play((const PblVector3*)mWorld.m[3], 0, false, true);
        }
    }

    if (!mAttachedSoldier)
    {
        for (int i = 0; i < 3; ++i)
        {
            const EntityPropAnimatedClass::AttachTrigger& attach = mAnimatedClass->mAttachTriggers[i];
            const unsigned int uAttachPoint = attach.mAttachPoint;
            if (!uAttachPoint || mAnimationId != attach.mAnimationID || attach.mAnimationTime < mAnimationTime)
            {
                continue;
            }
            mAttachMatrix = const_cast<PblMatrix*>(GetPose()->Find(uAttachPoint));
            if (!mAttachMatrix)
            {
                continue;
            }
            PblMatrix mWorld;
            D3DXMatrixMultiply(&mWorld, mAttachMatrix, &mMatrix);
            GameObject* pFound;
            if (CollisionManager::GetObjectsInRange(*(const PblVector3*)mWorld.m[3], attach.mAttachRadius,
                                                    &pFound, 1, 0, 2) <= 0)
            {
                continue;
            }
            if (!pFound->IsRtti(EntitySoldier::rttiHashEntitySoldier) || !pFound->IsAlive())
            {
                continue;
            }

            EntitySoldier* pSoldier = static_cast<EntitySoldier*>(pFound);
            mAttachedSoldier = pFound;
            mAttachEvent = mAnimatedClass->mAttachTriggers[i].mPostAttachEvent;
            mAttachedSoldierAnimation = mAnimatedClass->mAttachTriggers[i].mSoldierAnimation;
            pSoldier->RemoveFromCollisionManager();
            pSoldier->AddAttachedToCollisionManager();
            pSoldier->mAttachedToProp = true;
            const int iJoystick = pSoldier->GetJoystickIndex();
            if (pSoldier->mPlayerId >= 0 && iJoystick > -1)
            {
                FirstPerson::Activate(iJoystick, false);
            }
            if (mAnimatedClass->mTrackDeathOnAttach)
            {
                if (pSoldier->mInitialPlayerId >= 0)
                {
                    pSoldier->TrackDeath(pSoldier->mInitialPlayerId, 0);
                }
                pSoldier->mMarkedForDeath = true;
            }
            break;
        }
    }

    float fStep = fDeltaTime;
    if (mLooping && mAnimationTime + fDeltaTime >= mAnimatedClass->mAnimations[0]->m_u16NumFrames * (1.0f / 30.0f))
    {
        mIdleDelayTimer += fDeltaTime;
        if (mIdleDelayTimer > mAnimatedClass->mIdleDelay)
        {
            mIdleDelayTimer = 0.0f;
        }
        else
        {
            fStep = 0.0f;
        }
    }
    mAnimationTime += fStep;
    if (mLooping)
    {
        const float fLength = mAnimatedClass->mAnimations[0]->m_u16NumFrames * (1.0f / 30.0f);
        if (mAnimationTime > fLength)
        {
            mAnimationTime -= fLength;
        }
    }

    mZephyrPoseDyn.Update(fStep);
    if (mBlendFactor >= 1.0f)
    {
        mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
    }
    else
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDyn, mBlendFactor);
    }
    mBlendFactor += mAnimationId == 0 ? fStep * 0.2f : fStep;
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    mPose.ConvertFromZephyrPose(mZephyrPoseStatic);

    if (mAttachedSoldier && mAttachMatrix)
    {
        PblMatrix mWorld;
        D3DXMatrixMultiply(&mWorld, mAttachMatrix, &mMatrix);
        RedPose kPose;
        mAttachedSoldier->SetupPose(kPose);
        const PblMatrix* pRoot = kPose.Find(0x16E27226u);
        if (pRoot)
        {
            const PblMatrix mInverse = pRoot->GetInverseFast();
            PblMatrix mLocal;
            D3DXMatrixMultiply(&mLocal, &mInverse, &mWorld);
            mWorld = mLocal;
        }
        static_cast<EntitySoldier*>((GameObject*)mAttachedSoldier)
            ->SetPilotInfo(mWorld, PblVector3(0.0f, 0.0f, 0.0f),
                           (SoldierAnimationBank::ANIMATION)mAttachedSoldierAnimation, true, -1.0f, -1.0f, -1.0f,
                           fDeltaTime);
    }

    UpdateSound();
    return true;
}
