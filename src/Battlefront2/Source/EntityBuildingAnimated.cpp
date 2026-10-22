#include "EntityBuildingAnimated.h"
#include "PebbleFL/Common/PblVector.h"

#include <stdio.h>
#include <float.h>
#include <d3dx9math.h>
#include "CameraManager.h"
#include "FLEffect.h"
#include "GameLoop.h"
#include "NetGame.h"

#include "CollisionObject.h"
#include "Damageable.h"
#include "Explosion.h"
#include "GameModel.h"
#include "Thread.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "Zephyr/Source/ZephyrAnim.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"

// FUNCTION: SWBF2 0x00494b30
EntityBuildingAnimatedClass::EntityBuildingAnimatedClass(unsigned int uHash)
    : EntityBuildingClass(uHash)
{
    mAnimObj = 0;
    mAnimations[0] = 0;
    mAnimations[1] = 0;
    for (int i = 0; i < 5; i++)
    {
        mDeathEffectHash[i] = 0;
    }
    for (int i = 0; i < 5; i++)
    {
        mDeathEffectNodeHash[i] = 0;
    }
    mDeathExplosions = false;
}

// FUNCTION: SWBF2 0x00494c50
EntityBuildingAnimatedClass::~EntityBuildingAnimatedClass()
{
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x01ebb87c
// SYNTHETIC: SWBF2 0x004028b0
// `dynamic initializer for 'EntityBuildingAnimatedClass::rttiHashEntityBuildingAnimatedClass''
PblHash EntityBuildingAnimatedClass::rttiHashEntityBuildingAnimatedClass("EntityBuildingAnimatedClass");
char EntityBuildingAnimatedClass::rttiNameEntityBuildingAnimatedClass[] = "EntityBuildingAnimatedClass";

// GLOBAL: SWBF2 0x01ebb878
// SYNTHETIC: SWBF2 0x004028a0
// `dynamic initializer for 'EntityBuildingAnimated::rttiHashEntityBuildingAnimated''
PblHash EntityBuildingAnimated::rttiHashEntityBuildingAnimated("EntityBuildingAnimated");
char EntityBuildingAnimated::rttiNameEntityBuildingAnimated[] = "EntityBuildingAnimated";

// FUNCTION: SWBF2 0x00475fb0
bool EntityBuildingAnimatedClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityBuildingAnimatedClass || EntityBuildingClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00494140
unsigned int EntityBuildingAnimatedClass::GetDerivedRtti()
{
    return rttiHashEntityBuildingAnimatedClass;
}

// FUNCTION: SWBF2 0x00494150
const char* EntityBuildingAnimatedClass::GetDerivedRttiName()
{
    return rttiNameEntityBuildingAnimatedClass;
}

// SYNTHETIC: SWBF2 0x00494b90
// EntityBuildingAnimatedClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00494bc0
EntityBuildingAnimatedClass::EntityBuildingAnimatedClass(EntityBuildingAnimatedClass* pBase, unsigned int uHash)
    : EntityBuildingClass(pBase, uHash)
{
    mAnimObj = pBase->mAnimObj;
    mAnimations[0] = pBase->mAnimations[0];
    mAnimations[1] = pBase->mAnimations[1];
    for (int i = 0; i < 5; i++)
    {
        mDeathEffectHash[i] = pBase->mDeathEffectHash[i];
    }
    for (int i = 0; i < 5; i++)
    {
        mDeathEffectNodeHash[i] = pBase->mDeathEffectNodeHash[i];
    }
    mDeathExplosions = pBase->mDeathExplosions;
}

// FUNCTION: SWBF2 0x00494c60
bool EntityBuildingAnimatedClass::InitAnimations(const char* pszName)
{
    mAnimObj = RedAnimation::Find(PblHash(pszName));
    return mAnimObj != 0;
}

// FUNCTION: SWBF2 0x00494ca0
void EntityBuildingAnimatedClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x98557a5e:
        InitAnimations(pszValue);
        break;

    case 0x297bc6eb:
        if (mAnimObj)
        {
            mAnimations[0] = (ZephyrAnim*)mAnimObj->FindAnimation(pszValue);
        }
        break;

    case 0x5fac58b5:
        if (mAnimObj)
        {
            mAnimations[1] = (ZephyrAnim*)mAnimObj->FindAnimation(pszValue);
        }
        break;

    case 0x3e4e9ba0:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        mDeathExplosions = iValue != 0;
        break;
    }

    case 0xf138015c:
    {
        int i = 0;
        while (mDeathEffectHash[i])
        {
            if (++i >= 5)
            {
                return;
            }
        }
        char acEffect[32];
        char acNode[32];
        sscanf(pszValue, "%s %s", acEffect, acNode);
        mDeathEffectHash[i] = PblHash(acEffect);
        mDeathEffectNodeHash[i] = PblTEMPHash(acNode);
        break;
    }

    default:
        EntityBuildingClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00494e00
bool EntityBuildingAnimatedClass::SetupAnimation(ZephyrPoseDyn<32>& kPose, int iAnim, float fTime, bool bLoop)
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

// FUNCTION: SWBF2 0x00494eb0
EntityClass* EntityBuildingAnimatedClass::Derive(unsigned int uHash)
{
    return new EntityBuildingAnimatedClass(this, uHash);
}

// FUNCTION: SWBF2 0x00494f20
Entity* EntityBuildingAnimatedClass::Build(const EntityDesc& desc)
{
    typedef void (__thiscall* ActivatePhysicsFn)(void*);

    EntityBuildingAnimated* pBuilding = new EntityBuildingAnimated(this, desc);
    if (pBuilding)
    {
        static_cast<CollisionObject*>(pBuilding)->AddStaticToCollisionManager();
        pBuilding->ActivatePhysics();
    }
    return (Entity*)pBuilding;
}

// FUNCTION: SWBF2 0x00475f30
bool EntityBuildingAnimated::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityBuildingAnimated || EntityBuilding::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00475fa0
const RedPose* EntityBuildingAnimated::GetPose() const
{
    return &mPose;
}

// FUNCTION: SWBF2 0x004940f0
unsigned int EntityBuildingAnimated::GetDerivedRtti() const
{
    return rttiHashEntityBuildingAnimated;
}

// FUNCTION: SWBF2 0x00494100
const char* EntityBuildingAnimated::GetDerivedRttiName() const
{
    return rttiNameEntityBuildingAnimated;
}

// FUNCTION: SWBF2 0x00475f80
EntityClass* EntityBuildingAnimated::GetEntityClass()
{
    return (EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x00475f90
const EntityClass* EntityBuildingAnimated::GetEntityClass() const
{
    return (const EntityClass*)mAnimatedClass;
}

// FUNCTION: SWBF2 0x00494160
EntityBuildingAnimated::EntityBuildingAnimated(EntityBuildingAnimatedClass* pClass, const EntityDesc& desc)
    : EntityBuilding(pClass, desc)
{
    mAnimObj = pClass->mAnimObj;
    mAnimatedClass = pClass;
    mAnimationId = (unsigned int)-1;
    if (pClass->mAnimations[0])
    {
        mAnimationTime = PblRandom::GetGlobalFloat() * mAnimatedClass->mAnimations[0]->m_u16NumFrames * 30.0f;
    }
    else
    {
        mAnimationTime = 0.0f;
    }
    if (mAnimObj)
    {
        mZephyrSkeleton.Open(*mAnimatedClass->mAnimObj->GetZephyrSkeletonShared());
        mZephyrPoseDyn.Open(mZephyrSkeleton);
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }
    mDeathExplosionTime = 0.0f;
}

// SYNTHETIC: SWBF2 0x00494110
// EntityBuildingAnimated::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00494f98
// [thunk]: EntityBuildingAnimated::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x00494fa3
// [thunk]: EntityBuildingAnimated::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x00494fab
// [thunk]: EntityBuildingAnimated::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x00494fb6
// [thunk]: EntityBuildingAnimated::`scalar deleting destructor' adjustor{608}
// SYNTHETIC: SWBF2 0x00494fc1
// [thunk]: EntityBuildingAnimated::`scalar deleting destructor' adjustor{148}

// FUNCTION: SWBF2 0x00494320
EntityBuildingAnimated::~EntityBuildingAnimated()
{
}

// FUNCTION: SWBF2 0x004943d0
void EntityBuildingAnimated::Init()
{
    mModel->GetRedModel(0)->SetupPose(mPose);
    if (mModel->GetRedModel(0)->_pStencilShadow)
    {
        RedDirectionalLight* pLight = RedLight::GetGlobalDirectionalLight(0);
        if (!pLight->IsShadowCaster())
        {
            pLight = RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster()
                         ? RedLight::GetGlobalDirectionalLight(1)
                         : NULL;
        }
        if (pLight)
        {
            const PblVector3& vDir = pLight->m_vDirection;
            float fGrow = _Sphere.w;
            if (vDir.y < -0.5f)
            {
                fGrow /= vDir.y * -2.0f;
            }
            SetRenderPos(PblVector3(vDir.x * fGrow + _Sphere.x, vDir.y * fGrow + _Sphere.y,
                                    vDir.z * fGrow + _Sphere.z));
            SetRenderRadius(_Sphere.w + fGrow);
            SetTestBBox(false);
        }
    }
    if (mpShadow)
    {
        mpShadow->RemoveFromScene();
    }
}

// FUNCTION: SWBF2 0x00494b00
void EntityBuildingAnimated::ActivatePhysics()
{
    ActivateThread(0, PER_LOCAL_TURN);
}

// FUNCTION: SWBF2 0x00494b20
void EntityBuildingAnimated::DeactivatePhysics()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x00494a10
void EntityBuildingAnimated::SetAnimation(unsigned int uAnimationId, bool bLoop)
{
    if (!mAnimObj || uAnimationId == mAnimationId)
    {
        return;
    }

    mAnimatedClass->SetupAnimation(mZephyrPoseDyn, uAnimationId, mAnimationTime, bLoop);

    ZephyrPoseStatic<32> kPose;
    mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
    kPose.Open(mZephyrSkeleton);
    kPose.Set(mZephyrPoseDyn, true);
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    mAnimationId = uAnimationId;
}

// FUNCTION: SWBF2 0x00494940
void EntityBuildingAnimated::Kill()
{
    if (mAnimationId != 1 && mAnimatedClass->mAnimations[1])
    {
        mState = STATE_DYING;
        mAnimationTime = 0.0f;
        SetAnimation(1, false);
        if (mAnimatedClass->mExplosion)
        {
            DamageOwner owner(mDamageOwner.mCharacter, mDamageOwner.mGameObject, mDamageOwner.mTeam, mClass, 0,
                              1.0f);
            mAnimatedClass->mExplosion->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
        }
        return;
    }
    EntityBuilding::Kill();
}

// FUNCTION: SWBF2 0x004944f0
bool EntityBuildingAnimated::Update(float fDeltaTime)
{
    EntityBuilding::Update(fDeltaTime);

    if (mAnimObj)
    {
        if (mAnimationId == (unsigned int)-1 && mAnimatedClass->mAnimations[0])
        {
            SetAnimation(0, true);
        }

        if (mAnimationId == 1 && mAnimationTime > mAnimatedClass->mAnimations[1]->m_u16NumFrames / 30.0f)
        {
            mAnimationId = (unsigned int)-1;
            mAnimationTime = 0.0f;
            for (int i = 0; i < 5; ++i)
            {
                if (!mAnimatedClass->mDeathEffectHash[i])
                {
                    continue;
                }
                PblMatrix mEffect;
                GetModel()->GetRedModel(0)->GetBoneMatrix(mAnimatedClass->mDeathEffectNodeHash[i], mEffect);
                PblMatrix mWorld;
                D3DXMatrixMultiply((D3DXMATRIX*)&mWorld, (const D3DXMATRIX*)&mMatrix, (const D3DXMATRIX*)&mEffect);
                mEffect = mWorld;
                if (CameraManager::sInstance->IsInsideAnyCamera(mEffect.GetTranslation(), 60.0f, 70.0f))
                {
                    if (FLEffectClass* pEffectClass = FLEffect::Get(mAnimatedClass->mDeathEffectHash[i]))
                    {
                        if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                        {
                            pEffect->AttachEffectToMatrix(&mEffect, false);
                        }
                    }
                }
            }
            EntityBuilding::Kill();
        }

        ZephyrPoseStatic<32> kPose;
        mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
        kPose.Open(mZephyrSkeleton);
        if (mAnimationId != (unsigned int)-1)
        {
            mAnimationTime += fDeltaTime;
            mZephyrPoseDyn.Update(fDeltaTime);
            kPose.Set(mZephyrPoseDyn, true);
        }
        mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }

    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return true;
    }
    if (mState == STATE_DEAD && !IsInScene() && !IsInCollisionList() && mAddHealth < FLT_EPSILON)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x004947d0
void EntityBuildingAnimated::Render(unsigned char uFlags, float fDist, unsigned int uMask)
{
    if (mAnimationId == 1 && mAnimatedClass->mDeathExplosions)
    {
        mDeathExplosionTime += GameLoop::GetDeltaTime();
        if (mDeathExplosionTime > 0.4f)
        {
            mDeathExplosionTime = 0.0f;
            FLEffectClass* pEffectClass = 0;
            switch (PblRandom::GetGlobalInt() % 3)
            {
            case 0:
                pEffectClass = FLEffect::s_EffectClasses.Find(0x02bb1fe0);
                break;
            case 1:
                pEffectClass = FLEffect::s_EffectClasses.Find(0xe07f39b5);
                break;
            case 2:
                pEffectClass = FLEffect::s_EffectClasses.Find(0x9db69ee4);
                break;
            }
            if (pEffectClass)
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    int iBody = PblRandom::GetGlobalInt() % mCollisionModel->GetNumBodies();
                    PblMatrix mBody = mCollisionModel->GetWorldMatrix(iBody, mMatrix, GetPose());
                    pEffect->AttachEffectToMatrix(&mBody, false);
                }
            }
        }
    }
    SetupShadow(this, mModel, uFlags, uMask, 0);
    EntityBuilding::Render(uFlags, fDist, uMask);
}
