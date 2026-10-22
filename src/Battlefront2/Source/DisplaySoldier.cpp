#include <string.h>

#include <d3dx9math.h>

#include "DisplaySoldier.h"
#include "AnimatedAddon.h"
#include "Cloth.h"
#include "Entity.h"
#include "EntitySoldier.h"
#include "GameModel.h"
#include "SoldierAnimationBank.h"
#include "SoldierAnimator.h"
#include "SoldierAnimatorClass.h"
#include "TentacleSimulator.h"
#include "Weapon.h"
#include "WeaponMelee.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Graphics/RedShadingState.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

namespace GameLoop { extern float sDeltaTime; }

extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x0048d740
bool SoldierElement::Update(float fElapsed)
{
    return true;
}

// FUNCTION: SWBF2 0x0048d750
PblVector3 SoldierElement::GetPos()
{
    return mPos;
}

// FUNCTION: SWBF2 0x0048d780
SoldierElement::SoldierElement()
{
    Enable(false);
    mAnimationHash = 0;
    mModel = 0;
    for (int i = 0; i < 8; ++i)
    {
        mWeaponClass[i] = 0;
    }
    mSoldierAnimator = 0;
    mWeaponIndex = 0;
    mClass = 0;
    mMatrix.Reset();
    mIsSetup = false;
    mID = reinterpret_cast<EntitySoldier*>(this);
    mGroundNormal = PblVector3(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 8; ++i)
    {
        mClothPiece[i] = 0;
    }
    mTentacles = 0;
    Reset();
}

// FUNCTION: SWBF2 0x0048d9e0
void SoldierElement::Reset()
{
    SoldierElement* pElement = (SoldierElement*)this;
    pElement->mSelected = false;
    pElement->mSelectedTimer = 1000.0f;
    pElement->mRandomActionTimer = 0.0f;
    pElement->mState = 0;
}

// FUNCTION: SWBF2 0x0048da10
bool SoldierElement::Setup()
{
    if (!mClass)
    {
        return false;
    }

    if (mClass->m_numTentacles && !TentacleSimulator::sMemoryPool.IsFull())
    {
        if (mTentacles)
        {
            delete mTentacles;
        }
        mTentacles = new TentacleSimulator(mClass->m_numTentacles, mClass->m_bonesPerTentacle,
                                           mClass->m_tentacleCollType);
    }
    else
    {
        mTentacles = 0;
    }

    if (mSoldierAnimator && mSoldierAnimator->GetOwner() == mID)
    {
        return false;
    }

    mSoldierAnimator = SoldierAnimatorClass::sInstance->GetAnimator(0.0f);
    if (mSoldierAnimator)
    {
        mSoldierAnimator->SetNewOwner((unsigned int)mID, 0, mClass->mSpecialSkeleton,
                                      *(const PblBitVector<32>*)mClass->mUpperBodyAnimMask,
                                      *(const PblBitVector<32>*)mClass->mLowerBodyAnimMask, 1.0f);
        mSoldierAnimator->SetAction(SoldierState::STAND, mMatrix, mAnimationHash);
    }

    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        mWeaponClass[i] = (WeaponClass*)mClass->GetWeaponClass(i);
    }

    EntityDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.mMatrix = mMatrix;
    for (unsigned int i = 0; i < mModel->mClothCount; ++i)
    {
        EntityClass* pClass = EntityClass::Find(mModel->mClothList[i]);
        if (mClothPiece[i])
        {
            delete mClothPiece[i];
        }
        mClothPiece[i] = pClass ? (EntityCloth*)pClass->Build(desc) : 0;
    }

    mIsSetup = true;
    return true;
}

// FUNCTION: SWBF2 0x0048e520
void SoldierElement::SetSelected(bool bSelected)
{
    float fDeltaTime = GameLoop::sDeltaTime;
    if (bSelected)
    {
        if (!mSelected)
        {
            mSelectedTimer = 0.0f;
        }
        mSelectedTimer += fDeltaTime;

        if (mWeaponClass[mWeaponIndex] &&
            mWeaponClass[mWeaponIndex]->IsRtti(WeaponMeleeClass::rttiHashWeaponMeleeClass))
        {
            const WeaponMeleeClass* pMelee = (const WeaponMeleeClass*)mWeaponClass[mWeaponIndex];
            if (pMelee->IsLightSaber() && pMelee && !mSelected)
            {
                pMelee->PlayTurnOnSound();
            }
        }

        SoldierAnimationBank::MAP eMap = (SoldierAnimationBank::MAP)0;
        if (mWeaponClass[mWeaponIndex])
        {
            eMap = SoldierAnimationBank::FindMap(
                mClass->mAnimationBank,
                (SoldierAnimationBank::WEAPON)mWeaponClass[mWeaponIndex]->mSoldierAnimationWeapon);
            if (eMap == SoldierAnimationBank::INVALID_MAP)
            {
                eMap = (SoldierAnimationBank::MAP)0;
            }
        }
        if (mSoldierAnimator)
        {
            mSoldierAnimator->SetWeaponAnimationMap(eMap);
        }

        mRandomActionTimer -= fDeltaTime;
        if (mRandomActionTimer < 0.0f)
        {
            bool bRoot4 = mWeaponClass[mWeaponIndex] &&
                          SoldierAnimationBank::GetWeaponRoot((SoldierAnimationBank::WEAPON)
                              mWeaponClass[mWeaponIndex]->mSoldierAnimationWeapon) == 4;
            if (!bRoot4 && mSoldierAnimator)
            {
                mSoldierAnimator->SetWeaponState(4, 0.0f, false, false);
            }
            mRandomActionTimer = PblRandom::_Global.GetFloat() * 5.0f + 3.0f;
        }
        mSelected = true;
    }
    else
    {
        if (mSelected)
        {
            mSelectedTimer = 0.0f;
        }
        mSelectedTimer += fDeltaTime;

        if (mWeaponClass[mWeaponIndex] &&
            mWeaponClass[mWeaponIndex]->IsRtti(WeaponMeleeClass::rttiHashWeaponMeleeClass))
        {
            const WeaponMeleeClass* pMelee = (const WeaponMeleeClass*)mWeaponClass[mWeaponIndex];
            if (pMelee->IsLightSaber() && pMelee && mSelected)
            {
                pMelee->PlayTurnOffSound();
            }
        }
        mSelected = false;
        mRandomActionTimer = 0.0f;
    }
}

// FUNCTION: SWBF2 0x0048dc90
void SoldierElement::RenderUsingContext(const PblMatrix& mContext, const RedColor& color)
{
    if (!m_bEnabled)
    {
        return;
    }

    mPos = PblVector3(mContext.m[3][0], mContext.m[3][1], mContext.m[3][2]);

    PblMatrix mScreen = mContext;
    mScreen.m[1][0] = mContext.m[1][0] * -1.0f;
    mScreen.m[1][1] = mContext.m[1][1] * -1.0f;
    mScreen.m[1][2] = mContext.m[1][2] * -1.0f;
    mScreen.m[1][3] = 0.0f;

    float fScale = 0.75f / RedRenderer::GetScreenAspectRatio() * mDepth;
    mScreen.m[3][0] *= fScale;
    mScreen.m[3][1] *= fScale;
    mScreen.m[3][2] *= fScale;
    mScreen.m[3][3] = 1.0f;

    PblMatrix mWorld;
    PblQuaternion qUnit;
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mWorld,
                                 D3DXQuaternionNormalize((D3DXQUATERNION*)&qUnit,
                                                         (const D3DXQUATERNION*)&mRotation));
    mWorld.m[3][0] = mTranslation.x;
    mWorld.m[3][1] = mTranslation.y;
    mWorld.m[3][2] = mTranslation.z;
    mWorld.m[3][3] = 1.0f;

    PblMatrix mScaling;
    D3DXMatrixMultiply((D3DXMATRIX*)&mWorld, (const D3DXMATRIX*)&mWorld,
                       D3DXMatrixScaling((D3DXMATRIX*)&mScaling, mScale, mScale, mScale));
    D3DXMatrixMultiply((D3DXMATRIX*)&mScaling, (const D3DXMATRIX*)&mWorld,
                       (const D3DXMATRIX*)&mScreen);
    mScreen = mScaling;

    RedPose pose;

    if (!mSoldierAnimator || mSoldierAnimator->GetOwner() != mID ||
        mSoldierAnimator->mSoldierAction != mState)
    {

        mSoldierAnimator = SoldierAnimatorClass::sInstance->GetAnimator(0.0f);
        if (mSoldierAnimator)
        {
            mSoldierAnimator->SetNewOwner((unsigned int)mID, 0, mClass->mSpecialSkeleton,
                                          *(const PblBitVector<32>*)mClass->mUpperBodyAnimMask,
                                          *(const PblBitVector<32>*)mClass->mLowerBodyAnimMask, 1.0f);
            mSoldierAnimator->SetAction((SoldierState::SoldierState)mState, mMatrix, mAnimationHash);
        }
        for (int i = 0; i < mClass->mWeaponCount; ++i)
        {
            mWeaponClass[i] = (WeaponClass*)mClass->GetWeaponClass(i);
        }
        mIsSetup = true;
    }

    SoldierAnimationBank::MAP eMap = (SoldierAnimationBank::MAP)0;
    if (mWeaponClass[mWeaponIndex])
    {
        eMap = SoldierAnimationBank::FindMap(
            mClass->mAnimationBank,
            (SoldierAnimationBank::WEAPON)mWeaponClass[mWeaponIndex]->mSoldierAnimationWeapon);
        if (eMap == SoldierAnimationBank::INVALID_MAP)
        {
            eMap = (SoldierAnimationBank::MAP)0;
        }
    }
    if (mSoldierAnimator)
    {
        mSoldierAnimator->SetWeaponAnimationMap(eMap);
        mSoldierAnimator->SetAction((SoldierState::SoldierState)mState, mMatrix, mAnimationHash);
        mSoldierAnimator->SetAimDirection(mAim);
        mSoldierAnimator->SetAlert(false);
        mSoldierAnimator->SetupPose(pose);
    }

    PblMatrix mRender = mScreen;
    if (mState == SoldierState::PRONE)
    {
        D3DXVECTOR3 vBack(-mScreen.m[2][0], -mScreen.m[2][1], -mScreen.m[2][2]);
        D3DXVECTOR3 vCross;
        D3DXVec3Cross(&vCross, &vBack, (const D3DXVECTOR3*)&mGroundNormal);
        D3DXVec3Normalize((D3DXVECTOR3*)&mRender.m[0][0], &vCross);
        mRender.m[0][3] = 0.0f;

        mRender.m[1][0] = mGroundNormal.x;
        mRender.m[1][1] = mGroundNormal.y;
        mRender.m[1][2] = mGroundNormal.z;
        mRender.m[1][3] = 0.0f;

        D3DXVec3Cross(&vCross, (const D3DXVECTOR3*)&mScreen.m[0][0], (const D3DXVECTOR3*)&mGroundNormal);
        D3DXVec3Normalize((D3DXVECTOR3*)&mRender.m[2][0], &vCross);
        mRender.m[2][3] = 0.0f;
    }

    if (mTentacles)
    {
        mTentacles->UpdateTimer(GameLoop::sDeltaTime);
    }

    if (mClass->mAddon)
    {
        mClass->mAddon->Render(mTentacles, gVectorZero, &pose, mRender, color, 0x4000000);
    }
    else if (mTentacles)
    {
        PblMatrix aTentacleMatrices[20];
        mTentacles->DoTentacles(&pose, mRender, gVectorZero, aTentacleMatrices, -1.0f);
    }

    if (mState != SoldierState::PILOT_ANIMATION && mWeaponClass[mWeaponIndex])
    {

        if (mWeaponClass[mWeaponIndex]->IsRtti(WeaponMeleeClass::rttiHashWeaponMeleeClass))
        {
            const WeaponMeleeClass* pMelee = (const WeaponMeleeClass*)mWeaponClass[mWeaponIndex];
            float fTurnOn = pMelee->m_fTurnOnDuration > mSelectedTimer
                                ? mSelectedTimer / pMelee->m_fTurnOnDuration
                                : 1.0f;
            if (!mSelected)
            {
                fTurnOn = 1.0f - fTurnOn;
            }
            pMelee->Render(fTurnOn, mRender, &pose, color, 0x4000000, false);
        }
        else
        {
            mWeaponClass[mWeaponIndex]->Render(mRender, &pose, color, 0x4000000, false);
        }
    }

    RedModel* pRedModel = (RedModel*)mModel->GetRedModel(0);
    if (mClass->mOverrideTextureHash)
    {

        RedShadingPose shadingPose;
        shadingPose.m_pFirstEntry = 0;
        shadingPose.m_pModel = 0;
        shadingPose.BindToModel(pRedModel);
        if (RedShadingState* pState = shadingPose.CreateShadingState(0xdc5803e9))
        {
            pState->GetIntParam(0x891891e9) = mClass->mOverrideTextureHash;
        }
        if (mClass->mOverrideTextureHash2)
        {
            if (RedShadingState* pState = shadingPose.CreateShadingState(0xb98e11c1))
            {
                pState->GetIntParam(0x891891e9) = mClass->mOverrideTextureHash2;
            }
        }
        pRedModel->Render(&mRender, &pose, color, 0x4000000, &shadingPose);
        shadingPose.Reset();
    }
    else
    {
        pRedModel->Render(&mRender, &pose, color, 0x4000000, 0);
    }

    for (unsigned int i = 0; i < mModel->mClothCount; ++i)
    {
        if (mClothPiece[i])
        {
            mClothPiece[i]->Render(mRender, &pose, 0, color, 0x4000000, 1);
        }
    }
}

// SYNTHETIC: SWBF2 0x0048d910
// SoldierElement::`scalar deleting destructor'
// FUNCTION: SWBF2 0x0048d940
SoldierElement::~SoldierElement()
{
    for (int i = 0; i < 8; ++i)
    {
        delete mClothPiece[i];
    }
    for (int i = 0; i < 8; ++i)
    {
        mClothPiece[i] = 0;
    }
    if (mTentacles)
    {
        delete mTentacles;
    }
}
