#include "SoldierAnimatorClass.h"
#include "PebbleFL/Common/PblVector.h"

#include <new>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// LIBRARY: SWBF2 0x005b3fe0
// strrchr

#include "SoldierAnimation.h"
#include "SoldierAnimationData.h"
#include "SoldierAnimator.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "RedEngineFL/Util/RedStack.h"
#include "Zephyr/Source/ZephyrAnimBank.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrSkeleton.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

static const float ANIM_FRAME_SPEED = 1.0f / 30.0f;

// GLOBAL: SWBF2 0x01eafb1c
SoldierAnimatorClass* SoldierAnimatorClass::sInstance;

namespace
{
inline int SoldierPlayerId(const EntitySoldier* pSoldier)
{
    return pSoldier->mPlayerId;
}

inline const SoldierAnimator* SoldierAnimatorOf(const EntitySoldier* pSoldier)
{
    return pSoldier->mSoldierAnimator;
}
}

// GLOBAL: SWBF2 0x007ae72c
static const unsigned int s_auiAnimatedJoints[] = { 0x16e27226, 0x2b960099, 0 };

// GLOBAL: SWBF2 0x007ae740
static const unsigned int s_auiAnimatedJoints_93e93819[] = { 0x16e27226, 0x8afdf5e4, 0x98b5d322, 0xa1296ed1,
                                                             0x3ce47edc, 0x0578c32f, 0 };

// GLOBAL: SWBF2 0x007ae75c
static const unsigned int s_auiAnimatedJoints_7a89c91f[] = { 0x16e27226, 0 };

// GLOBAL: SWBF2 0x007ae790
static const unsigned int s_auiLowerBodyJoints_b70e2184[] = {
    0x5e892db0, 0x1acfaf53, 0xcdce0a7a, 0x5a483007, 0x0488ff56, 0xc90f17cd, 0x3344aa92, 0xb3cf3c33,
    0x7e48d4a8, 0x3785b725, 0xb70e2184, 0x7a89c91f, 0x93e93819, 0x16e27226, 0xe6e47876,
};
// GLOBAL: SWBF2 0x007ae7d0
static const unsigned int s_auiLowerBodyJoints[] = {
    0x16e27226, 0xe6e47876, 0xc496176d, 0x87019234, 0x1af77461,
    0x2a8eade7, 0xf6eff696, 0xb2f01cd3, 0x2f06fa86, 0x5a5be874,
};

// FUNCTION: SWBF2 0x006436f0
void SoldierAnimatorClass::Create()
{
    if (!sInstance)
    {
        sInstance = new SoldierAnimatorClass;
    }
}

// FUNCTION: SWBF2 0x00643760
void SoldierAnimatorClass::Destroy()
{
    if (sInstance)
    {
        delete sInstance;
        sInstance = 0;
    }
    SoldierAnimationData::CleanupAnimationData();
}

// FUNCTION: SWBF2 0x00643790
void SoldierAnimatorClass::Read(PblFileChunk& chunk)
{
    SoldierAnimationData::Read(chunk);
}

// FUNCTION: SWBF2 0x006437a0
SoldierAnimatorClass::SoldierAnimatorClass()
{
    mZephyrSkeletonShared = 0;
    memset(mAnimBank, 0, sizeof(mAnimBank));
    memset(mMap, 0, sizeof(mMap));
    mAnimBankCount = 0;
    _AddBank("human");
    SetupJointTransformAnimationFlag(mZephyrSkeletonShared);
    SetupBodyMasks(mZephyrSkeletonShared, mDefaultUpperBodyAnimMask, mDefaultLowerBodyAnimMask);
    mSoldierAnimators = 0;
}

// FUNCTION: SWBF2 0x00643820
SoldierAnimatorClass::~SoldierAnimatorClass()
{
    if (mSoldierAnimators)
    {
        for (int i = 0; i < 50; ++i)
        {
            PblList<SoldierAnimator>::Node& node = mSoldierAnimators[i].GetLRUNode();
            if (node.IsInList())
            {
                mLRU_List.Remove(&node);
            }
        }
        delete[] mSoldierAnimators;
    }
}

// FUNCTION: SWBF2 0x006438a0
const SoldierAnimation* SoldierAnimatorClass::GetUpperBodyMovementAnimation(SoldierAnimType::MovementAnimation eAnim,
                                                                            SoldierAnimType::Posture ePosture,
                                                                            SoldierAnimationBank::MAP eMap,
                                                                            bool bAlert) const
{
    int iPosture = (unsigned int)ePosture % SoldierAnimType::NUM_STANDARD_POSTURES;
    if (bAlert)
    {
        iPosture += SoldierAnimType::NUM_STANDARD_POSTURES;
    }
    return mMap[eMap].mMovementAnimation[iPosture][eAnim][SoldierAnimType::UPPER];
}

// FUNCTION: SWBF2 0x006438f0
const SoldierAnimation* SoldierAnimatorClass::GetLowerBodyMovementAnimation(SoldierAnimType::MovementAnimation eAnim,
                                                                            SoldierAnimType::Posture ePosture,
                                                                            SoldierAnimationBank::MAP eMap,
                                                                            bool bAlert) const
{
    int iPosture = (unsigned int)ePosture % SoldierAnimType::NUM_STANDARD_POSTURES;
    if (bAlert)
    {
        iPosture += SoldierAnimType::NUM_STANDARD_POSTURES;
    }
    return mMap[eMap].mMovementAnimation[iPosture][eAnim][SoldierAnimType::LOWER];
}

// FUNCTION: SWBF2 0x00643940
const SoldierAnimation* SoldierAnimatorClass::GetUpperBodyActionAnimation(SoldierAnimType::ActionAnimation eAnim,
                                                                          SoldierAnimationBank::MAP eMap) const
{
    return mMap[eMap].mActionAnimation[eAnim][SoldierAnimType::UPPER];
}

// FUNCTION: SWBF2 0x00643960
const SoldierAnimation* SoldierAnimatorClass::GetLowerBodyActionAnimation(SoldierAnimType::ActionAnimation eAnim,
                                                                          SoldierAnimationBank::MAP eMap) const
{
    return mMap[eMap].mActionAnimation[eAnim][SoldierAnimType::LOWER];
}

// FUNCTION: SWBF2 0x00643980
const SoldierAnimation* SoldierAnimatorClass::GetUpperBodyWeaponAnimation(SoldierAnimType::Posture ePosture,
                                                                          SoldierAnimationBank::MAP eMap,
                                                                          SoldierAnimType::WeaponAnimation eAnim) const
{
    return mMap[eMap].mWeaponAnimation[ePosture][eAnim][SoldierAnimType::UPPER];
}

// FUNCTION: SWBF2 0x006439b0
const SoldierAnimation* SoldierAnimatorClass::GetLowerBodyWeaponAnimation(SoldierAnimType::Posture ePosture,
                                                                          SoldierAnimationBank::MAP eMap,
                                                                          SoldierAnimType::WeaponAnimation eAnim) const
{
    return mMap[eMap].mWeaponAnimation[ePosture][eAnim][SoldierAnimType::LOWER];
}

// FUNCTION: SWBF2 0x006439e0
const SoldierAnimation* SoldierAnimatorClass::GetUpperBodyAnimation(SoldierAnimationBank::MAP eMap,
                                                                    unsigned char uiAnimIndex) const
{
    int i = uiAnimIndex * SoldierAnimType::NUM_SCOPES + SoldierAnimType::UPPER;
    if (i < NUM_ACTION_ANIMATIONS_TOTAL + NUM_MOVEMENT_ANIMATIONS_TOTAL + NUM_WEAPON_ANIMATIONS_TOTAL)
    {
        return mMap[eMap].mAnimation[i];
    }
    return mMap[eMap].mAnimation[i - NUM_WEAPON_ANIMATIONS_TOTAL];
}

// FUNCTION: SWBF2 0x00643a10
const SoldierAnimation* SoldierAnimatorClass::GetLowerBodyAnimation(SoldierAnimationBank::MAP eMap,
                                                                    unsigned char uiAnimIndex) const
{
    int i = uiAnimIndex * SoldierAnimType::NUM_SCOPES + SoldierAnimType::LOWER;
    if (i < NUM_ACTION_ANIMATIONS_TOTAL + NUM_MOVEMENT_ANIMATIONS_TOTAL + NUM_WEAPON_ANIMATIONS_TOTAL)
    {
        return mMap[eMap].mAnimation[i];
    }
    return mMap[eMap].mAnimation[i - NUM_WEAPON_ANIMATIONS_TOTAL];
}

// FUNCTION: SWBF2 0x00643a50
SoldierAnimator* SoldierAnimatorClass::GetAnimator(float)
{
    for (PblList<SoldierAnimator>::Itor it = mLRU_List.Begin(); it != mLRU_List.End(); ++it)
    {
        SoldierAnimator* pAnimator = it;
        const EntitySoldier* pOwner =
            pAnimator->mOwner ? pAnimator->mOwner : (const EntitySoldier*)pAnimator->mOwnerId;
        if (!pOwner || SoldierPlayerId(pOwner) < 0 || SoldierAnimatorOf(pOwner) != pAnimator)
        {
            return pAnimator;
        }
    }
    return mLRU_List.GetFirst()->_pObject;
}

// FUNCTION: SWBF2 0x00643ab0
SoldierAnimType::WeaponAnimation SoldierAnimatorClass::WeaponStateToAnimation(int eState, bool bAlternate)
{
    switch (eState)
    {
    case 1:
        return bAlternate ? SoldierAnimType::SHOOT_SECONDARY : SoldierAnimType::SHOOT;
    case 2:
        return bAlternate ? SoldierAnimType::SHOOT_SECONDARY2 : SoldierAnimType::SHOOT2;
    case 3:
        return bAlternate ? SoldierAnimType::WEAPON_IDLE : SoldierAnimType::CHARGE;
    case 4:
        return bAlternate ? SoldierAnimType::WEAPON_IDLE : SoldierAnimType::RELOAD;
    default:
        return SoldierAnimType::WEAPON_IDLE;
    }
}

// FUNCTION: SWBF2 0x00643b10
bool SoldierAnimatorClass::GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap, SoldierAnimType::Posture ePosture,
                                                  SoldierAnimType::MovementAnimation eAnim, unsigned int uiJointCrc,
                                                  float fTime, PblMatrix& matrix) const
{
    bool bAlert = ePosture >= SoldierAnimType::STAND_ALERT;
    const SoldierAnimation* pUpper = GetUpperBodyMovementAnimation(eAnim, ePosture, eMap, bAlert);
    const SoldierAnimation* pLower = GetLowerBodyMovementAnimation(eAnim, ePosture, eMap, bAlert);
    if (!pUpper || !pLower)
    {
        return false;
    }
    return _GetAnimationBoneMatrix(_GetAnimationMapSkeleton(eMap), pUpper->m_pZephyrAnim, pLower->m_pZephyrAnim,
                                   fTime, fTime, uiJointCrc, matrix);
}

// FUNCTION: SWBF2 0x00643ba0
bool SoldierAnimatorClass::GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap,
                                                  SoldierAnimType::ActionAnimation eAnim, unsigned int uiJointCrc,
                                                  float fTime, PblMatrix& matrix) const
{
    const SoldierAnimation* pUpper = GetUpperBodyActionAnimation(eAnim, eMap);
    const SoldierAnimation* pLower = GetLowerBodyActionAnimation(eAnim, eMap);
    if (!pUpper || !pLower)
    {
        return false;
    }
    return _GetAnimationBoneMatrix(_GetAnimationMapSkeleton(eMap), pUpper->m_pZephyrAnim, pLower->m_pZephyrAnim,
                                   fTime, fTime, uiJointCrc, matrix);
}

// FUNCTION: SWBF2 0x00643c00
bool SoldierAnimatorClass::GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap, SoldierAnimType::Posture ePosture,
                                                  SoldierAnimType::WeaponAnimation eAnim, unsigned int uiJointCrc,
                                                  float fTime, PblMatrix& matrix) const
{
    const SoldierAnimation* pUpper = GetUpperBodyWeaponAnimation(ePosture, eMap, eAnim);
    const SoldierAnimation* pLower = GetLowerBodyWeaponAnimation(ePosture, eMap, eAnim);
    if (!pUpper)
    {
        return false;
    }
    float fLowerTime = fTime;
    if (!pLower)
    {
        pLower = GetLowerBodyMovementAnimation(SoldierAnimType::IDLE, SoldierAnimType::STAND, eMap, false);
        if (!pLower)
        {
            return false;
        }
        fLowerTime = 0.0f;
    }
    return _GetAnimationBoneMatrix(_GetAnimationMapSkeleton(eMap), pUpper->m_pZephyrAnim, pLower->m_pZephyrAnim,
                                   fTime, fLowerTime, uiJointCrc, matrix);
}

// FUNCTION: SWBF2 0x00643c80
bool SoldierAnimatorClass::GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap,
                                                  SoldierAnimationBank::ANIMATION eAnim, unsigned int uiJointCrc,
                                                  float fTime, PblMatrix& matrix) const
{
    unsigned int uiAnimHash = GetCustomAnimation(eMap, eAnim);
    if (!uiAnimHash)
    {
        return false;
    }
    const ZephyrAnim* pAnim = GetAnimation(uiAnimHash, 0);
    if (!pAnim)
    {
        return false;
    }
    return _GetAnimationBoneMatrix(_GetAnimationMapSkeleton(eMap), pAnim, pAnim, fTime, fTime, uiJointCrc, matrix);
}

// FUNCTION: SWBF2 0x00643cf0
bool SoldierAnimatorClass::GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap, unsigned char uiAnimIndex,
                                                  unsigned int uiJointCrc, float fTime, PblMatrix& matrix) const
{
    const SoldierAnimation* pUpper = GetUpperBodyAnimation(eMap, uiAnimIndex);
    const SoldierAnimation* pLower = GetLowerBodyAnimation(eMap, uiAnimIndex);
    if (!pUpper)
    {
        return false;
    }
    float fLowerTime = fTime;
    if (!pLower)
    {
        pLower = GetLowerBodyMovementAnimation(SoldierAnimType::IDLE, SoldierAnimType::STAND, eMap, false);
        if (!pLower)
        {
            return false;
        }
        fLowerTime = 0.0f;
    }
    return _GetAnimationBoneMatrix(_GetAnimationMapSkeleton(eMap), pUpper->m_pZephyrAnim, pLower->m_pZephyrAnim,
                                   fTime, fLowerTime, uiJointCrc, matrix);
}

// FUNCTION: SWBF2 0x00643d80
const ZephyrSkeletonShared* SoldierAnimatorClass::_GetAnimationMapSkeleton(SoldierAnimationBank::MAP eMap)
{
    unsigned int uiHash = SoldierAnimationBank::GetBankNameHash(SoldierAnimationBank::GetMapBank(eMap));
    const RedAnimation* pBank = RedAnimation::Find(uiHash);
    if (!pBank)
    {
        pBank = RedAnimation::Find(PblHash(uiHash, "_0"));
        if (!pBank)
        {
            return 0;
        }
    }
    return pBank->GetZephyrSkeletonShared();
}

// FUNCTION: SWBF2 0x00643de0
bool SoldierAnimatorClass::_GetAnimationBoneMatrix(const ZephyrSkeletonShared* pSkel, const ZephyrAnim* pUpperAnim,
                                                   const ZephyrAnim* pLowerAnim, float fUpperTime, float fLowerTime,
                                                   unsigned int uiJointCrc, PblMatrix& matrix)
{
    if (!pSkel)
    {
        return false;
    }
    int iJoint = (int)pSkel->m_kHashCRC2Joint.Find(uiJointCrc) - 1;
    if (iJoint < 0)
    {
        return false;
    }

    ZephyrSkeleton<32> kSkeleton;
    ZephyrPoseStatic<32> kPoseStatic;
    ZephyrPoseDyn<32> kPoseUpper;
    ZephyrPoseDyn<32> kPoseLower;
    kSkeleton.Open(*pSkel, kPoseStatic);
    kPoseStatic.Open(kSkeleton);
    kPoseUpper.Open(kSkeleton);
    kPoseLower.Open(kSkeleton);

    PblBitVector<32> upperBodyMask;
    PblBitVector<32> lowerBodyMask;
    SetupBodyMasks(pSkel, upperBodyMask, lowerBodyMask);
    upperBodyMask.SetBit(iJoint);
    lowerBodyMask.ClearBit(iJoint);

    kPoseUpper.SetAnimation(pUpperAnim, 30.0f);
    kPoseUpper.SetCurTime(0.0f);
    kPoseUpper.Update(fUpperTime);
    kPoseUpper.SetLoop(false);
    kPoseUpper.SetInterpolate(true);

    kPoseLower.SetAnimation(pLowerAnim, 30.0f);
    kPoseLower.SetLoop(false);
    kPoseLower.SetCurTime(0.0f);
    if (fLowerTime > 0.0f)
    {
        kPoseLower.SetInterpolate(true);
        kPoseLower.Update(fLowerTime);
    }
    else
    {
        kPoseLower.SetInterpolate(false);
    }

    kPoseStatic.Set(kPoseUpper, upperBodyMask, true);
    kPoseStatic.Set(kPoseLower, lowerBodyMask, true);
    kSkeleton.BuildWorldMatrices(_IdentityMatrix, iJoint);
    matrix = kSkeleton.m_pWorldMatrices[iJoint];
    return true;
}

// FUNCTION: SWBF2 0x006440f0
void SoldierAnimatorClass::SetupJointTransformAnimationFlag(ZephyrSkeletonShared* pSkel)
{
    for (int i = 1; i < (int)pSkel->m_uNumJoints; ++i)
    {
        pSkel->SetJointTransformAnimationFlag(i, false);
    }

    const unsigned int* auiJoints;
    if ((int)pSkel->m_kHashCRC2Joint.Find(0x7a89c91f) - 1 >= 0)
    {
        auiJoints = s_auiAnimatedJoints_7a89c91f;
    }
    else if ((int)pSkel->m_kHashCRC2Joint.Find(0x93e93819) - 1 >= 0)
    {
        auiJoints = s_auiAnimatedJoints_93e93819;
    }
    else
    {
        auiJoints = s_auiAnimatedJoints;
    }

    for (int i = 0; auiJoints[i]; ++i)
    {
        int iJoint = (int)pSkel->m_kHashCRC2Joint.Find(auiJoints[i]) - 1;
        pSkel->SetJointTransformAnimationFlag(iJoint, true);
    }
}

// FUNCTION: SWBF2 0x006441e0
void SoldierAnimatorClass::SetupBodyMasks(const ZephyrSkeletonShared* pSkel, PblBitVector<32>& upperBodyMask,
                                          PblBitVector<32>& lowerBodyMask)
{
    bool bHasJoint = (int)pSkel->m_kHashCRC2Joint.Find(0xb70e2184) - 1 >= 0;
    upperBodyMask.SetAll();
    lowerBodyMask.ClearAll();
    upperBodyMask.m_auiData[0] &= ~1u;

    const unsigned int* pJoint;
    const unsigned int* pEnd;
    if (bHasJoint)
    {
        pJoint = s_auiLowerBodyJoints_b70e2184;
        pEnd = s_auiLowerBodyJoints_b70e2184 + sizeof(s_auiLowerBodyJoints_b70e2184) / sizeof(unsigned int);
    }
    else
    {
        pJoint = s_auiLowerBodyJoints;
        pEnd = s_auiLowerBodyJoints + sizeof(s_auiLowerBodyJoints) / sizeof(unsigned int);
    }
    for (; pJoint < pEnd; ++pJoint)
    {
        int iJoint = (int)pSkel->m_kHashCRC2Joint.Find(*pJoint) - 1;
        unsigned int uiBit = 1u << (iJoint & 31);
        upperBodyMask.m_auiData[0] &= ~uiBit;
        lowerBodyMask.m_auiData[0] |= uiBit;
    }
}

// FUNCTION: SWBF2 0x006442a0
const ZephyrAnim* SoldierAnimatorClass::GetAnimation(unsigned int uiNameHash, const char*) const
{
    for (int i = 0; i < mAnimBankCount; ++i)
    {
        if (mAnimBank[i]->_pZephyrAnimBank)
        {
            const ZephyrAnim* pAnim = mAnimBank[i]->_pZephyrAnimBank->Find(uiNameHash);
            if (pAnim)
            {
                return pAnim;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006442f0
unsigned int SoldierAnimatorClass::GetCustomAnimation(SoldierAnimationBank::MAP eMap,
                                                      SoldierAnimationBank::ANIMATION eAnim) const
{
    return mMap[eMap].mCustomAnimation[eAnim];
}

// FUNCTION: SWBF2 0x00644310
void SoldierAnimatorClass::UpdateLRU(SoldierAnimator* pAnimator)
{
    mLRU_List.Remove(&pAnimator->GetLRUNode());
    mLRU_List.AddLast(&pAnimator->GetLRUNode(), pAnimator);
}

// FUNCTION: SWBF2 0x00644370
bool SoldierAnimatorClass::_AddBank(const char* pcName)
{
    const RedAnimation* pBank = RedAnimation::Find(PblHash(pcName));
    if (pBank)
    {
        return _AddBank(pBank);
    }

    char acName[257];
    int iLen = strlen(pcName);
    strlcpy(acName, pcName, sizeof(acName));
    char* pcSecond = strchr(acName, '_');
    if (pcSecond)
    {
        pcSecond = strchr(pcSecond + 1, '_');
    }

    strlcat(acName, "_0", sizeof(acName));
    pBank = RedAnimation::Find(PblHash(acName));
    while (!pBank)
    {
        acName[iLen] = 0;
        char* pcUnderscore = strrchr(acName, '_');
        if (!pcUnderscore)
        {
            return false;
        }
        if (pcSecond && pcUnderscore > pcSecond)
        {
            pcUnderscore = pcSecond;
        }
        iLen = pcUnderscore - acName;
        acName[iLen] = 0;
        pBank = RedAnimation::Find(PblHash(acName));
        if (pBank)
        {
            return _AddBank(pBank);
        }
        strlcat(acName, "_0", sizeof(acName));
        pBank = RedAnimation::Find(PblHash(acName));
    }

    bool bResult = _AddBank(pBank);
    for (int i = 1; bResult && i < MAX_ANIMATION_BANK_FILES; ++i)
    {
        sprintf(acName + iLen, "_%d", i);
        pBank = RedAnimation::Find(PblHash(acName));
        if (!pBank)
        {
            break;
        }
        bResult = _AddBank(pBank);
    }
    return bResult;
}

// FUNCTION: SWBF2 0x00644570
bool SoldierAnimatorClass::_AddBank(const RedAnimation* pBank)
{
    if (!pBank)
    {
        return false;
    }
    if (!mZephyrSkeletonShared)
    {
        mZephyrSkeletonShared = pBank->GetZephyrSkeletonShared();
    }
    if (!pBank->_pZephyrAnimBank)
    {
        return true;
    }
    for (int i = 0; i < mAnimBankCount; ++i)
    {
        if (mAnimBank[i] == pBank)
        {
            return true;
        }
    }
    if (mAnimBankCount >= MAX_ANIMATION_BANK_FILES)
    {
        return false;
    }
    mAnimBank[mAnimBankCount] = const_cast<RedAnimation*>(pBank);
    ++mAnimBankCount;
    return true;
}

// FUNCTION: SWBF2 0x006445e0
const PblMatrix* SoldierAnimatorClass::_GetBoneMatrix(unsigned int uiJointCrc, const SoldierAnimation* pAnim,
                                                      ZephyrSkeleton<32>& skeleton, ZephyrPoseDyn<32>& poseDyn,
                                                      ZephyrPoseStatic<32>& poseStatic) const
{
    RedStack stack;
    RedPose* pPose = new (stack.Alloc(sizeof(RedPose))) RedPose;
    pPose->BindToSkeleton(NULL);
    pAnim->SetupDynamicPose(poseDyn, (pAnim->GetAnimationNumFrames() - 1) * ANIM_FRAME_SPEED * 0.5f);
    poseStatic.Set(poseDyn, true);
    skeleton.BuildWorldMatrices(_IdentityMatrix);
    pPose->ConvertFromZephyrPose(poseStatic);
    return pPose->_Table.Find(uiJointCrc);
}

// FUNCTION: SWBF2 0x006446c0
static int _GetAnimationIndex(SoldierAnimType::ActionAnimation eAnim, SoldierAnimType::Scope eScope)
{
    return eAnim * SoldierAnimType::NUM_SCOPES + eScope;
}

// FUNCTION: SWBF2 0x006446d0
static int _GetAnimationIndex(SoldierAnimType::Posture ePosture, SoldierAnimType::MovementAnimation eAnim,
                              SoldierAnimType::Scope eScope)
{
    return SoldierAnimatorClass::NUM_ACTION_ANIMATIONS_TOTAL +
           (ePosture * SoldierAnimType::NUM_MOVEMENT_ANIMATIONS + eAnim) * SoldierAnimType::NUM_SCOPES + eScope;
}

// FUNCTION: SWBF2 0x006446f0
static int _GetAnimationIndex(SoldierAnimType::Posture ePosture, SoldierAnimType::WeaponAnimation eAnim,
                              SoldierAnimType::Scope eScope)
{
    return SoldierAnimatorClass::NUM_ACTION_ANIMATIONS_TOTAL + SoldierAnimatorClass::NUM_MOVEMENT_ANIMATIONS_TOTAL +
           (ePosture * SoldierAnimType::NUM_WEAPON_ANIMATIONS + eAnim) * SoldierAnimType::NUM_SCOPES + eScope;
}

// FUNCTION: SWBF2 0x00644710
static int _GetAnimationIndex(SoldierAnimationBank::COMBO_ANIMATION eAnim, SoldierAnimType::Scope eScope)
{
    return SoldierAnimatorClass::NUM_ACTION_ANIMATIONS_TOTAL + SoldierAnimatorClass::NUM_MOVEMENT_ANIMATIONS_TOTAL +
           eAnim * SoldierAnimType::NUM_SCOPES + eScope;
}

// FUNCTION: SWBF2 0x00644720
static bool _IsActionAnimationIndex(int i)
{
    return i >= 0 && i < SoldierAnimatorClass::NUM_ACTION_ANIMATIONS_TOTAL;
}

// FUNCTION: SWBF2 0x00644730
static bool _IsMovementAnimationIndex(int i)
{
    return i >= SoldierAnimatorClass::NUM_ACTION_ANIMATIONS_TOTAL &&
           i - SoldierAnimatorClass::NUM_ACTION_ANIMATIONS_TOTAL < SoldierAnimatorClass::NUM_MOVEMENT_ANIMATIONS_TOTAL;
}

// FUNCTION: SWBF2 0x00644750
static bool _IsWeaponMeleeAnimationIndex(int i)
{
    const int iBase = SoldierAnimatorClass::NUM_ACTION_ANIMATIONS_TOTAL + SoldierAnimatorClass::NUM_MOVEMENT_ANIMATIONS_TOTAL;
    return i >= iBase && i - iBase < SoldierAnimatorClass::NUM_WEAPON_MELEE_ANIMATIONS_TOTAL;
}

// FUNCTION: SWBF2 0x00644770
static void SetupVelocity(SoldierAnimation* pAnim, ZephyrPoseDyn<32>& tempPose)
{
    if (!pAnim)
    {
        return;
    }

    unsigned short uiRoot = 0;
    if (SoldierAnimatorClass::Instance())
    {
        uiRoot = (unsigned short)((int)SoldierAnimatorClass::Instance()->GetDefaultSkeleton()->m_kHashCRC2Joint.Find(
                                      0x4446e9b8) -
                                  1);
    }

    ZephyrTransform kTransform;
    tempPose.SetAnimation(NULL, 30.0f);
    pAnim->SetupDynamicPose(tempPose, 0.0f);
    tempPose.GetJointTransform(kTransform, uiRoot);
    PblVector3 vStart = kTransform.kTrans;

    float fTime = (pAnim->GetAnimationNumFrames() - 1) * ANIM_FRAME_SPEED * 0.5f;
    pAnim->SetupDynamicPose(tempPose, fTime);
    tempPose.GetJointTransform(kTransform, uiRoot);
    pAnim->SetVelocity((kTransform.kTrans.x - vStart.x) / fTime, (kTransform.kTrans.z - vStart.z) / fTime);
}

// FUNCTION: SWBF2 0x00644900
static void SetupVelocity(SoldierAnimation* pAnim, int iAnim, ZephyrPoseDyn<32>& tempPose)
{
    if (iAnim >= SoldierAnimatorClass::NUM_ACTION_ANIMATIONS_TOTAL + SoldierAnimatorClass::NUM_MOVEMENT_ANIMATIONS_TOTAL +
                    SoldierAnimatorClass::NUM_WEAPON_ANIMATIONS_TOTAL ||
        !pAnim || !pAnim->HasLowerBody())
    {
        return;
    }

    if (_IsActionAnimationIndex(iAnim))
    {
        int eAnim = SoldierAnimType::GetAnimFromAnimIndex((unsigned char)(iAnim / 2));
        if (eAnim != SoldierAnimType::SPRINT && eAnim < SoldierAnimType::DIE_FORWARD)
        {
            return;
        }
        SetupVelocity(pAnim, tempPose);
        if (eAnim != SoldierAnimType::SPRINT || pAnim->GetVelocityZ() > 0.0f)
        {
            return;
        }
        pAnim->SetVelocity(pAnim->GetVelocityX(), 11.5f);
        return;
    }

    if (!_IsMovementAnimationIndex(iAnim))
    {
        SetupVelocity(pAnim, tempPose);
        return;
    }

    int eAnim = SoldierAnimType::GetAnimFromAnimIndex((unsigned char)(iAnim / 2));
    if (eAnim < SoldierAnimType::WALK_FORWARD)
    {
        return;
    }
    SetupVelocity(pAnim, tempPose);

    float fVelocityX = pAnim->GetVelocityX();
    float fVelocityZ = pAnim->GetVelocityZ();
    switch (eAnim)
    {
    case SoldierAnimType::WALK_FORWARD:
        if (fVelocityZ <= 0.0f)
        {
            fVelocityZ = 0.9f;
        }
        break;
    case SoldierAnimType::RUN_FORWARD:
        if (fVelocityZ <= 0.0f)
        {
            fVelocityZ = 3.5f;
        }
        break;
    case SoldierAnimType::WALK_BACKWARD:
        if (fVelocityZ >= 0.0f)
        {
            fVelocityZ = -0.9f;
        }
        break;
    case SoldierAnimType::RUN_BACKWARD:
        if (fVelocityZ >= 0.0f)
        {
            fVelocityZ = -3.5f;
        }
        break;
    case SoldierAnimType::WALK_LEFT:
        if (fVelocityX >= 0.0f)
        {
            fVelocityX = -0.9f;
        }
        break;
    case SoldierAnimType::RUN_LEFT:
        if (fVelocityX >= 0.0f)
        {
            fVelocityX = -3.5f;
        }
        break;
    case SoldierAnimType::WALK_RIGHT:
        if (fVelocityX <= 0.0f)
        {
            fVelocityX = 0.9f;
        }
        break;
    case SoldierAnimType::RUN_RIGHT:
        if (fVelocityX <= 0.0f)
        {
            fVelocityX = 3.5f;
        }
        break;
    }
    pAnim->SetVelocity(fVelocityX, fVelocityZ);
}

template <class T, int N>
class MyStack
{
public:
    MyStack() : m_uiDepth(0) {}
    T* Push()
    {
        if (m_uiDepth < N)
        {
            return &m_aStack[m_uiDepth++];
        }
        return NULL;
    }
    void Pop()
    {
        if (m_uiDepth)
        {
            --m_uiDepth;
        }
    }
    unsigned int GetDepth() const { return m_uiDepth; }

    T* GetData(unsigned int uiDepth) { return uiDepth < m_uiDepth ? &m_aStack[m_uiDepth - uiDepth - 1] : NULL; }

    T m_aStack[N];
    unsigned int m_uiDepth;
};

class MapCache
{
public:
    struct CacheEntry
    {
        SoldierAnimationBank::BANK eBank;
        SoldierAnimationBank::WEAPON eWeapon;
        SoldierAnimatorClass::AnimationMap Map;
    };
    static const int CACHE_SIZE = 24;

    MapCache()
    {
        for (int i = 0; i < CACHE_SIZE; ++i)
        {
            m_aCache[i].eBank = SoldierAnimationBank::INVALID_BANK;
        }
    }
    void RemoveBank(SoldierAnimationBank::BANK eBank)
    {
        for (int i = 0; i < CACHE_SIZE; ++i)
        {
            if (m_aCache[i].eBank == eBank)
            {
                m_aCache[i].eBank = SoldierAnimationBank::INVALID_BANK;
            }
        }
    }
    SoldierAnimatorClass::AnimationMap* AddMap(SoldierAnimationBank::BANK eBank, SoldierAnimationBank::WEAPON eWeapon);
    static void ClearMap(SoldierAnimatorClass::AnimationMap* pMap) { memset(pMap, -1, sizeof(*pMap)); }

    static bool IsInvalid(const void* p) { return *(const int*)p == -1; }

    CacheEntry m_aCache[CACHE_SIZE];
};

class CachedBankStack
{
public:
    struct Entry
    {
        SoldierAnimationBank::BANK eBank;
        SoldierAnimatorClass::AnimationMap* apMap[SoldierAnimationBank::MAX_WEAPONS];
    };

    CachedBankStack() {}
    unsigned int GetDepth() { return m_Stack.GetDepth(); }
    SoldierAnimationBank::BANK GetBank(unsigned int uiDepth)
    {
        Entry* pEntry = m_Stack.GetData(uiDepth);
        return pEntry ? pEntry->eBank : SoldierAnimationBank::INVALID_BANK;
    }
    SoldierAnimatorClass::AnimationMap*& Map(SoldierAnimationBank::WEAPON eWeapon, unsigned int uiDepth)
    {
        return m_Stack.GetData(uiDepth)->apMap[eWeapon];
    }
    void Push(SoldierAnimationBank::BANK eBank)
    {
        Entry* pEntry = m_Stack.Push();
        if (pEntry)
        {
            pEntry->eBank = eBank;
            memset(pEntry->apMap, 0, sizeof(pEntry->apMap));
        }
    }

    void Pop()
    {
        if (GetDepth())
        {
            m_Cache.RemoveBank(GetBank(0));
            m_Stack.Pop();
        }
    }
    SoldierAnimatorClass::AnimationMap* AddWeapon(SoldierAnimationBank::WEAPON eWeapon,
                                                  SoldierAnimatorClass::AnimationMap* pMap);
    SoldierAnimatorClass::AnimationMap* AddWeaponParent(SoldierAnimationBank::WEAPON eWeapon, unsigned int uiDepth);
    SoldierAnimatorClass::AnimationMap* AddBankParent(SoldierAnimationBank::WEAPON eWeapon, unsigned int uiDepth);

    MyStack<Entry, 3> m_Stack;
    MapCache m_Cache;
};

class AnimationFinder
{
public:
    struct Entry
    {
        SoldierAnimationBank::BANK eBank;
        int iAnimBankStart_ByBank;
        int iAnimBankEnd_ByBank;
        int aiAnimBankStart_ByWeapon[SoldierAnimationBank::MAX_WEAPONS];
        int aiAnimBankEnd_ByWeapon[SoldierAnimationBank::MAX_WEAPONS];
    };

    AnimationFinder(const RedAnimation** const ppAnimBank, int& iAnimBankCount, int iMaxCount);
    unsigned int GetDepth() { return m_Stack.GetDepth(); }

    void Push(SoldierAnimationBank::BANK eBank)
    {
        Entry* pEntry = m_Stack.Push();
        if (pEntry)
        {
            pEntry->eBank = eBank;
            pEntry->iAnimBankStart_ByBank = mAnimBankCount;
            _AddBank(SoldierAnimationBank::GetBankName(eBank));
            pEntry->iAnimBankEnd_ByBank = mAnimBankCount;
            memset(pEntry->aiAnimBankStart_ByWeapon, 0, sizeof(pEntry->aiAnimBankStart_ByWeapon));
            memset(pEntry->aiAnimBankEnd_ByWeapon, 0, sizeof(pEntry->aiAnimBankEnd_ByWeapon));
        }
    }
    void Pop() { m_Stack.Pop(); }
    void SetFindData(CachedBankStack& cache, ZephyrPoseDyn<32>& tempPose)
    {
        m_pCache = &cache;
        m_pTempPose = &tempPose;
    }
    void SetAnimationData(const SoldierAnimationData* pAnimData, SoldierAnimation::SCOPE eScope)
    {
        m_pAnimData = const_cast<SoldierAnimationData*>(pAnimData);
        m_eDefaultScope = eScope;
    }
    void SetTargetAnimation(int iAnim) { m_iAnim = iAnim; }
    void SetTargetCustomAnimation(int iAnim) { m_iAnim = iAnim; }
    bool AddWeapon(SoldierAnimationBank::WEAPON eWeapon, unsigned int uiDepth);
    const ZephyrAnim* FindZephyrAnimation(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                          unsigned int uiDepth);
    SoldierAnimation* AssignAnimation(const char* pcName, SoldierAnimationBank::WEAPON eWeapon, unsigned int uiDepth);
    SoldierAnimation* AssignAnimationFromWeaponParent(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                                      unsigned int uiDepth, bool bRecurse);
    SoldierAnimation* AssignAnimationFromWeaponMeleeParent(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                                           unsigned int uiDepth, bool bRecurse);
    SoldierAnimation* AssignAnimationFromWeaponAlertParent(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                                           unsigned int uiDepth, bool bRecurse);
    SoldierAnimation* AssignAnimationFromBankParent(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                                    unsigned int uiDepth, bool bRecurse);
    unsigned int AssignCustomAnimation(const char* pcName, SoldierAnimationBank::WEAPON eWeapon, unsigned int uiDepth);
    unsigned int AssignCustomAnimationFromWeaponParent(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                                       unsigned int uiDepth, bool bRecurse);
    unsigned int AssignCustomAnimationFromBankParent(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                                     unsigned int uiDepth, bool bRecurse);

private:
    bool _AddBank(const RedAnimation* pBank);
    bool _AddBank(const char* pcName);

public:
    MyStack<Entry, 3> m_Stack;
    int mMaxCount;
    RedAnimation** mAnimBank;
    int& mAnimBankCount;
    int m_iAnim;
    SoldierAnimationData* m_pAnimData;
    SoldierAnimation::SCOPE m_eDefaultScope;
    CachedBankStack* m_pCache;
    ZephyrPoseDyn<32>* m_pTempPose;
};
 
 
 
 
 
 
 
 
 
 
 

void* SoldierAnimation::operator new(unsigned int uiSize)
{
    return sMemoryPool.Allocate(uiSize);
}

// GLOBAL: SWBF2 0x0078b4a8
static const char* sScopeName[4] = { "NONE", "upper", "lower", "full" };

// FUNCTION: SWBF2 0x00644b40
SoldierAnimatorClass::AnimationMap* MapCache::AddMap(SoldierAnimationBank::BANK eBank,
                                                     SoldierAnimationBank::WEAPON eWeapon)
{
    int i;
    for (i = 0; i < CACHE_SIZE; ++i)
    {
        if (m_aCache[i].eBank == eBank && m_aCache[i].eWeapon == eWeapon)
        {
            return &m_aCache[i].Map;
        }
    }
    for (i = 0; i < CACHE_SIZE; ++i)
    {
        if (IsInvalid(&m_aCache[i]))
        {
            break;
        }
    }
    if (i == CACHE_SIZE)
    {
        return NULL;
    }
    m_aCache[i].eBank = eBank;
    m_aCache[i].eWeapon = eWeapon;
    ClearMap(&m_aCache[i].Map);
    return &m_aCache[i].Map;
}

// FUNCTION: SWBF2 0x00644c20
SoldierAnimatorClass::AnimationMap* CachedBankStack::AddWeapon(SoldierAnimationBank::WEAPON eWeapon,
                                                               SoldierAnimatorClass::AnimationMap* pMap)
{
    if (!GetDepth())
    {
        return NULL;
    }
    if (Map(eWeapon, 0))
    {
        return Map(eWeapon, 0);
    }
    MapCache::ClearMap(pMap);
    Map(eWeapon, 0) = pMap;
    return pMap;
}

// FUNCTION: SWBF2 0x00644cc0
SoldierAnimatorClass::AnimationMap* CachedBankStack::AddWeaponParent(SoldierAnimationBank::WEAPON eWeapon,
                                                                     unsigned int uiDepth)
{
    if (uiDepth >= GetDepth())
    {
        return NULL;
    }
    SoldierAnimationBank::WEAPON eParent = SoldierAnimationBank::GetWeaponParent(eWeapon);
    if (eParent == eWeapon)
    {
        return NULL;
    }
    if (!Map(eParent, uiDepth))
    {
        AddWeaponParent(eParent, uiDepth);
        Map(eParent, uiDepth) = m_Cache.AddMap(GetBank(uiDepth), eParent);
    }
    return Map(eParent, uiDepth);
}

// FUNCTION: SWBF2 0x00644d90
SoldierAnimatorClass::AnimationMap* CachedBankStack::AddBankParent(SoldierAnimationBank::WEAPON eWeapon,
                                                                   unsigned int uiDepth)
{
    unsigned int uiParentDepth = uiDepth + 1;
    if (uiParentDepth >= GetDepth())
    {
        return NULL;
    }
    if (!Map(eWeapon, uiParentDepth))
    {
        AddWeaponParent(eWeapon, uiParentDepth);
        AddBankParent(eWeapon, uiParentDepth);
        Map(eWeapon, uiParentDepth) = m_Cache.AddMap(GetBank(uiParentDepth), eWeapon);
    }
    return Map(eWeapon, uiParentDepth);
}

// FUNCTION: SWBF2 0x00644e50
AnimationFinder::AnimationFinder(const RedAnimation** const ppAnimBank, int& iAnimBankCount, int iMaxCount)
    : mMaxCount(iMaxCount), mAnimBank((RedAnimation**)ppAnimBank), mAnimBankCount(iAnimBankCount)
{
    Entry* pEntry = m_Stack.Push();
    if (pEntry)
    {
        pEntry->eBank = SoldierAnimationBank::HUMAN;
        pEntry->iAnimBankStart_ByBank = 0;
        pEntry->iAnimBankEnd_ByBank = mAnimBankCount;
        memset(pEntry->aiAnimBankStart_ByWeapon, 0, sizeof(pEntry->aiAnimBankStart_ByWeapon));
        memset(pEntry->aiAnimBankEnd_ByWeapon, 0, sizeof(pEntry->aiAnimBankEnd_ByWeapon));
    }
}

// FUNCTION: SWBF2 0x00644ee0
bool AnimationFinder::AddWeapon(SoldierAnimationBank::WEAPON eWeapon, unsigned int uiDepth)
{
    Entry* pEntry = m_Stack.GetData(uiDepth);
    if (!pEntry || pEntry->aiAnimBankEnd_ByWeapon[eWeapon] > 0)
    {
        return false;
    }

    const char* pcWeaponName = SoldierAnimationBank::GetWeaponName(eWeapon);
    char acName[256];
    _snprintf(acName, 255, "%s_%s", SoldierAnimationBank::GetBankName(pEntry->eBank), pcWeaponName);
    acName[255] = 0;

    pEntry->aiAnimBankStart_ByWeapon[eWeapon] = mAnimBankCount;
    _AddBank(acName);
    if (pEntry->eBank != SoldierAnimationBank::HUMAN)
    {
        _AddBank(SoldierAnimationBank::GetBankName(pEntry->eBank));
    }
    pEntry->aiAnimBankEnd_ByWeapon[eWeapon] = mAnimBankCount;

    SoldierAnimationBank::WEAPON eParent = SoldierAnimationBank::GetWeaponParent(eWeapon);
    if (eParent != eWeapon)
    {
        AddWeapon(eParent, uiDepth);
    }
    if (uiDepth + 1 < GetDepth())
    {
        AddWeapon(eWeapon, uiDepth + 1);
    }
    return true;
}

// FUNCTION: SWBF2 0x00644fe0
const ZephyrAnim* AnimationFinder::FindZephyrAnimation(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                                       unsigned int uiDepth)
{
    Entry* pEntry = m_Stack.GetData(uiDepth);
    if (!pEntry)
    {
        return NULL;
    }

    char acName[256];
    unsigned int uiNameHash;
    if (eWeapon != SoldierAnimationBank::INVALID_WEAPON)
    {
        _snprintf(acName, 255, "%s_%s_%s", SoldierAnimationBank::GetBankName(pEntry->eBank),
                  SoldierAnimationBank::GetWeaponName(eWeapon), pcName);
        acName[255] = 0;
        uiNameHash = PblTEMPHash(acName);

        unsigned int uiWeaponHash = SoldierAnimationBank::GetWeaponNameHash(eWeapon);
        for (int i = 0; i < SoldierAnimationBank::GetNumWeapons(); ++i)
        {
            if (SoldierAnimationBank::GetWeaponNameHash((SoldierAnimationBank::WEAPON)i) != uiWeaponHash)
            {
                continue;
            }
            for (int j = pEntry->aiAnimBankStart_ByWeapon[i]; j < pEntry->aiAnimBankEnd_ByWeapon[i]; ++j)
            {
                const ZephyrAnimBank* pBank = mAnimBank[j]->_pZephyrAnimBank;
                if (pBank)
                {
                    const ZephyrAnim* pAnim = pBank->Find(uiNameHash);
                    if (pAnim)
                    {
                        return pAnim;
                    }
                }
            }
        }
    }
    else
    {
        _snprintf(acName, 255, "%s_%s", SoldierAnimationBank::GetBankName(pEntry->eBank), pcName);
        acName[255] = 0;
        uiNameHash = PblTEMPHash(acName);
    }

    for (int j = pEntry->iAnimBankStart_ByBank; j < pEntry->iAnimBankEnd_ByBank; ++j)
    {
        const ZephyrAnimBank* pBank = mAnimBank[j]->_pZephyrAnimBank;
        if (pBank)
        {
            const ZephyrAnim* pAnim = pBank->Find(uiNameHash);
            if (pAnim)
            {
                return pAnim;
            }
        }
    }
    return NULL;
}

// FUNCTION: SWBF2 0x00645170
SoldierAnimation* AnimationFinder::AssignAnimation(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                                   unsigned int uiDepth)
{
    SoldierAnimatorClass::AnimationMap* pMap = m_pCache->Map(eWeapon, uiDepth);
    if (!MapCache::IsInvalid(&pMap->mAnimation[m_iAnim]))
    {
        return pMap->mAnimation[m_iAnim];
    }

    SoldierAnimation::SCOPE eScope = (m_iAnim & 1) ? SoldierAnimation::LOWER_BODY : SoldierAnimation::UPPER_BODY;
    const ZephyrAnim* pZephyrAnim;
    SoldierAnimation* pAnim;
    char acName[256];

    if (m_eDefaultScope == eScope)
    {
        pZephyrAnim = FindZephyrAnimation(pcName, eWeapon, uiDepth);
        if (pZephyrAnim)
        {
            pAnim = new SoldierAnimation(pZephyrAnim, m_pAnimData, m_eDefaultScope);
            goto setup;
        }
    }

    _snprintf(acName, 255, "%s_%s", pcName, sScopeName[eScope]);
    acName[255] = 0;
    pZephyrAnim = FindZephyrAnimation(acName, eWeapon, uiDepth);
    if (pZephyrAnim)
    {
        pAnim = new SoldierAnimation(pZephyrAnim, m_pAnimData, SoldierAnimation::UPPER_BODY);
        goto setup;
    }

    if (m_eDefaultScope == SoldierAnimation::FULL_BODY)
    {
        pZephyrAnim = FindZephyrAnimation(pcName, eWeapon, uiDepth);
        if (pZephyrAnim)
        {
            pAnim = new SoldierAnimation(pZephyrAnim, m_pAnimData, m_eDefaultScope);
            goto setup;
        }
    }

    _snprintf(acName, 255, "%s_full", pcName);
    acName[255] = 0;
    pZephyrAnim = FindZephyrAnimation(acName, eWeapon, uiDepth);
    if (!pZephyrAnim)
    {
        return NULL;
    }
    pAnim = new SoldierAnimation(pZephyrAnim, m_pAnimData, SoldierAnimation::FULL_BODY);

setup:
    SetupVelocity(pAnim, m_iAnim, *m_pTempPose);
    pMap->mAnimation[m_iAnim] = pAnim;
    return pAnim;
}

// FUNCTION: SWBF2 0x00645350
SoldierAnimation* AnimationFinder::AssignAnimationFromWeaponParent(const char* pcName,
                                                                   SoldierAnimationBank::WEAPON eWeapon,
                                                                   unsigned int uiDepth, bool bRecurse)
{
    SoldierAnimatorClass::AnimationMap* pMap = m_pCache->Map(eWeapon, uiDepth);
    SoldierAnimation* pAnim = pMap->mAnimation[m_iAnim];
    if (!MapCache::IsInvalid(&pMap->mAnimation[m_iAnim]) && (pAnim || !bRecurse))
    {
        return pAnim;
    }

    SoldierAnimationBank::WEAPON eParent = SoldierAnimationBank::GetWeaponParent(eWeapon);
    if (eParent == eWeapon ||
        (bRecurse && uiDepth && SoldierAnimationBank::GetWeaponRoot(eWeapon) == SoldierAnimationBank::MELEE &&
         eWeapon == SoldierAnimationBank::MELEE))
    {
        pMap->mAnimation[m_iAnim] = NULL;
        return NULL;
    }

    m_pCache->AddWeaponParent(eWeapon, uiDepth);
    pAnim = AssignAnimation(pcName, eParent, uiDepth);
    if (!pAnim)
    {
        pAnim = AssignAnimationFromWeaponParent(pcName, eParent, uiDepth, bRecurse);
    }
    if (!pAnim && bRecurse)
    {
        pAnim = AssignAnimationFromBankParent(pcName, eParent, uiDepth, bRecurse);
    }
    pMap->mAnimation[m_iAnim] = pAnim;
    return pAnim;
}

// FUNCTION: SWBF2 0x00645450
SoldierAnimation* AnimationFinder::AssignAnimationFromWeaponMeleeParent(const char* pcName,
                                                                        SoldierAnimationBank::WEAPON eWeapon,
                                                                        unsigned int uiDepth, bool bRecurse)
{
    SoldierAnimatorClass::AnimationMap* pMap = m_pCache->Map(eWeapon, uiDepth);
    SoldierAnimation* pAnim = pMap->mAnimation[m_iAnim];
    if (!MapCache::IsInvalid(&pMap->mAnimation[m_iAnim]) && (pAnim || !bRecurse))
    {
        return pAnim;
    }

    SoldierAnimationBank::WEAPON eParent = SoldierAnimationBank::GetWeaponParent(eWeapon);
    if (eWeapon == SoldierAnimationBank::MELEE || eParent == eWeapon)
    {
        pMap->mAnimation[m_iAnim] = NULL;
        return NULL;
    }

    m_pCache->AddWeaponParent(eWeapon, uiDepth);
    pAnim = AssignAnimation(pcName, eParent, uiDepth);
    if (!pAnim)
    {
        pAnim = AssignAnimationFromWeaponMeleeParent(pcName, eParent, uiDepth, bRecurse);
    }
    if (!pAnim && bRecurse)
    {
        pAnim = AssignAnimationFromBankParent(pcName, eParent, uiDepth, bRecurse);
    }
    pMap->mAnimation[m_iAnim] = pAnim;
    return pAnim;
}

// FUNCTION: SWBF2 0x00645530
SoldierAnimation* AnimationFinder::AssignAnimationFromWeaponAlertParent(const char* pcName,
                                                                        SoldierAnimationBank::WEAPON eWeapon,
                                                                        unsigned int uiDepth, bool bRecurse)
{
    SoldierAnimatorClass::AnimationMap* pMap = m_pCache->Map(eWeapon, uiDepth);
    SoldierAnimation* pAnim = pMap->mAnimation[m_iAnim];
    if (!MapCache::IsInvalid(&pMap->mAnimation[m_iAnim]) && (pAnim || !bRecurse))
    {
        return pAnim;
    }

    SoldierAnimationBank::WEAPON eChild = eWeapon;
    SoldierAnimationBank::WEAPON eParent = SoldierAnimationBank::GetWeaponParent(eWeapon);
    while (eParent != eChild && !SoldierAnimationBank::HasAlertState(eParent))
    {
        eChild = eParent;
        eParent = SoldierAnimationBank::GetWeaponParent(eParent);
    }
    if (eParent == eChild)
    {
        pMap->mAnimation[m_iAnim] = NULL;
        return NULL;
    }

    m_pCache->AddWeaponParent(eChild, uiDepth);
    pAnim = AssignAnimation(pcName, eParent, uiDepth);
    if (!pAnim)
    {
        pAnim = AssignAnimationFromWeaponAlertParent(pcName, eParent, uiDepth, bRecurse);
    }
    if (!pAnim && bRecurse)
    {
        pAnim = AssignAnimationFromBankParent(pcName, eParent, uiDepth, bRecurse);
    }
    pMap->mAnimation[m_iAnim] = pAnim;
    return pAnim;
}

// FUNCTION: SWBF2 0x00645620
SoldierAnimation* AnimationFinder::AssignAnimationFromBankParent(const char* pcName,
                                                                 SoldierAnimationBank::WEAPON eWeapon,
                                                                 unsigned int uiDepth, bool bRecurse)
{
    SoldierAnimatorClass::AnimationMap* pMap = m_pCache->Map(eWeapon, uiDepth);
    SoldierAnimation* pAnim = pMap->mAnimation[m_iAnim];
    if (!MapCache::IsInvalid(&pMap->mAnimation[m_iAnim]) && (pAnim || !bRecurse))
    {
        return pAnim;
    }

    unsigned int uiParentDepth = uiDepth + 1;
    if (uiParentDepth >= m_pCache->GetDepth())
    {
        pMap->mAnimation[m_iAnim] = NULL;
        return NULL;
    }

    m_pCache->AddBankParent(eWeapon, uiDepth);
    pAnim = AssignAnimation(pcName, eWeapon, uiParentDepth);
    if (!pAnim)
    {
        pAnim = AssignAnimationFromBankParent(pcName, eWeapon, uiParentDepth, bRecurse);
    }
    if (!pAnim && bRecurse)
    {
        if (SoldierAnimationBank::GetWeaponRoot(eWeapon) == SoldierAnimationBank::MELEE &&
            _IsWeaponMeleeAnimationIndex(m_iAnim))
        {
            pAnim = AssignAnimationFromWeaponMeleeParent(pcName, eWeapon, uiParentDepth, bRecurse);
        }
        else
        {
            pAnim = AssignAnimationFromWeaponParent(pcName, eWeapon, uiParentDepth, bRecurse);
        }
    }
    pMap->mAnimation[m_iAnim] = pAnim;
    return pAnim;
}

// FUNCTION: SWBF2 0x00645730
unsigned int AnimationFinder::AssignCustomAnimation(const char* pcName, SoldierAnimationBank::WEAPON eWeapon,
                                                    unsigned int uiDepth)
{
    SoldierAnimatorClass::AnimationMap* pMap = m_pCache->Map(eWeapon, uiDepth);
    if (!MapCache::IsInvalid(&pMap->mCustomAnimation[m_iAnim]))
    {
        return pMap->mCustomAnimation[m_iAnim];
    }

    const ZephyrAnim* pZephyrAnim = FindZephyrAnimation(pcName, eWeapon, uiDepth);
    if (!pZephyrAnim)
    {
        return 0;
    }
    return pMap->mCustomAnimation[m_iAnim] = pZephyrAnim->GetNameCRC();
}

// FUNCTION: SWBF2 0x006457a0
unsigned int AnimationFinder::AssignCustomAnimationFromWeaponParent(const char* pcName,
                                                                    SoldierAnimationBank::WEAPON eWeapon,
                                                                    unsigned int uiDepth, bool bRecurse)
{
    SoldierAnimatorClass::AnimationMap* pMap = m_pCache->Map(eWeapon, uiDepth);
    unsigned int uiAnim = pMap->mCustomAnimation[m_iAnim];
    if (!MapCache::IsInvalid(&pMap->mCustomAnimation[m_iAnim]) && (uiAnim || !bRecurse))
    {
        return uiAnim;
    }

    SoldierAnimationBank::WEAPON eParent = SoldierAnimationBank::GetWeaponParent(eWeapon);
    if (eParent == eWeapon)
    {
        const ZephyrAnim* pZephyrAnim = FindZephyrAnimation(pcName, SoldierAnimationBank::INVALID_WEAPON, uiDepth);
        if (pZephyrAnim)
        {
            return pMap->mCustomAnimation[m_iAnim] = pZephyrAnim->GetNameCRC();
        }
        pMap->mCustomAnimation[m_iAnim] = 0;
        return 0;
    }

    m_pCache->AddWeaponParent(eWeapon, uiDepth);
    uiAnim = AssignCustomAnimation(pcName, eParent, uiDepth);
    if (!uiAnim)
    {
        uiAnim = AssignCustomAnimationFromWeaponParent(pcName, eParent, uiDepth, bRecurse);
    }
    if (!uiAnim && bRecurse)
    {
        uiAnim = AssignCustomAnimationFromBankParent(pcName, eParent, uiDepth, bRecurse);
    }
    pMap->mCustomAnimation[m_iAnim] = uiAnim;
    return uiAnim;
}

// FUNCTION: SWBF2 0x006458b0
unsigned int AnimationFinder::AssignCustomAnimationFromBankParent(const char* pcName,
                                                                  SoldierAnimationBank::WEAPON eWeapon,
                                                                  unsigned int uiDepth, bool bRecurse)
{
    SoldierAnimatorClass::AnimationMap* pMap = m_pCache->Map(eWeapon, uiDepth);
    unsigned int uiAnim = pMap->mCustomAnimation[m_iAnim];
    if (!MapCache::IsInvalid(&pMap->mCustomAnimation[m_iAnim]) && (uiAnim || !bRecurse))
    {
        return uiAnim;
    }

    unsigned int uiParentDepth = uiDepth + 1;
    if (uiParentDepth >= m_pCache->GetDepth())
    {
        pMap->mCustomAnimation[m_iAnim] = 0;
        return 0;
    }

    m_pCache->AddBankParent(eWeapon, uiDepth);
    uiAnim = AssignCustomAnimation(pcName, eWeapon, uiParentDepth);
    if (!uiAnim)
    {
        uiAnim = AssignCustomAnimationFromBankParent(pcName, eWeapon, uiParentDepth, bRecurse);
    }
    if (!uiAnim && bRecurse)
    {
        uiAnim = AssignCustomAnimationFromWeaponParent(pcName, eWeapon, uiParentDepth, bRecurse);
    }
    pMap->mCustomAnimation[m_iAnim] = uiAnim;
    return uiAnim;
}

// FUNCTION: SWBF2 0x00645990
bool AnimationFinder::_AddBank(const RedAnimation* pBank)
{
    if (!pBank || !pBank->_pZephyrAnimBank)
    {
        return false;
    }
    for (int i = 0; i < mAnimBankCount; ++i)
    {
        if (mAnimBank[i] == pBank)
        {
            return false;
        }
    }
    if (mAnimBankCount >= mMaxCount)
    {
        return false;
    }
    mAnimBank[mAnimBankCount] = const_cast<RedAnimation*>(pBank);
    ++mAnimBankCount;
    return true;
}

// FUNCTION: SWBF2 0x00645a00
bool AnimationFinder::_AddBank(const char* pcName)
{
    unsigned int uiLen = strlen(pcName);
    const char* pcUnderscore = strchr(pcName, '_');
    if (pcUnderscore)
    {
        pcUnderscore = strchr(pcUnderscore + 1, '_');
        if (pcUnderscore)
        {
            uiLen = pcUnderscore - pcName;
        }
    }
    if (uiLen > 256)
    {
        uiLen = 256;
    }

    char acName[257];
    strlcpy(acName, pcName, sizeof(acName));
    char* pcEnd = &acName[uiLen];
    *pcEnd = 0;

    const RedAnimation* pBank = RedAnimation::Find(PblHash(acName));
    if (pBank)
    {
        return _AddBank(pBank);
    }

    sprintf(pcEnd, "_0");
    pBank = RedAnimation::Find(PblHash(acName));
    if (!pBank)
    {
        return true;
    }

    bool bResult = _AddBank(pBank);
    for (int i = 1; i < mMaxCount && mAnimBankCount < mMaxCount; ++i)
    {
        sprintf(pcEnd, "_%d", i);
        pBank = RedAnimation::Find(PblHash(acName));
        if (!pBank)
        {
            break;
        }
        bResult = _AddBank(pBank);
    }
    return bResult;
}

extern PblVector3 gVectorZero;
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

// FUNCTION: SWBF2 0x00645b70
void SoldierAnimatorClass::_PostLoad()
{
    SoldierAnimationData::InitLoadTimeDataPostLoad();
    SoldierAnimationData::InitAnimationData(true);

    ZephyrSkeleton<32> tempZephyrSkeleton;
    ZephyrPoseDyn<32> tempZephyrPoseDyn;
    ZephyrPoseStatic<32> tempZephyrPoseStatic;
    tempZephyrSkeleton.Open(*mZephyrSkeletonShared, tempZephyrPoseStatic);
    tempZephyrPoseStatic.Open(tempZephyrSkeleton);
    tempZephyrPoseDyn.Open(tempZephyrSkeleton);

    const int iNumBanks = SoldierAnimationBank::GetNumBanks();
    const int iNumWeapons = SoldierAnimationBank::GetNumWeapons();
    const int iNumCustomAnimations = SoldierAnimationBank::GetNumAnimations();

    int n;
    SoldierAnimationBank::BANK aeBankOrder[SoldierAnimationBank::MAX_BANKS];
    for (n = 0, aeBankOrder[0] = SoldierAnimationBank::GetFirstBank(); aeBankOrder[n] != SoldierAnimationBank::INVALID_BANK;
         ++n)
    {
        aeBankOrder[n + 1] = SoldierAnimationBank::GetNextBank(aeBankOrder[n]);
    }

    SoldierAnimationBank::WEAPON aeWeaponOrder[SoldierAnimationBank::MAX_WEAPONS];
    for (n = 0, aeWeaponOrder[0] = SoldierAnimationBank::GetFirstWeapon();
         aeWeaponOrder[n] != SoldierAnimationBank::INVALID_WEAPON; ++n)
    {
        aeWeaponOrder[n + 1] = SoldierAnimationBank::GetNextWeapon(aeWeaponOrder[n]);
    }

    SoldierAnimationBank::MAP aeFindMap[SoldierAnimationBank::MAX_BANKS][SoldierAnimationBank::MAX_WEAPONS];
    for (int bank = 0; bank < iNumBanks; ++bank)
    {
        for (int weapon = 0; weapon < iNumWeapons; ++weapon)
        {
            aeFindMap[bank][weapon] =
                SoldierAnimationBank::FindMap((SoldierAnimationBank::BANK)bank, (SoldierAnimationBank::WEAPON)weapon);
        }
    }

    CachedBankStack Cache;
    AnimationFinder AnimFinder((const RedAnimation**)mAnimBank, mAnimBankCount, MAX_ANIMATION_BANK_FILES);
    Cache.Push(aeBankOrder[0]);
    AnimFinder.SetFindData(Cache, tempZephyrPoseDyn);

    for (int bank = 0; bank < iNumBanks; ++bank)
    {
        const SoldierAnimationBank::BANK eBank = aeBankOrder[bank];
        const SoldierAnimationBank::BANK eBankParent = SoldierAnimationBank::GetBankParent(eBank);
        if (bank)
        {

            if (eBankParent != aeBankOrder[bank - 1])
            {
                while (Cache.GetDepth())
                {
                    if (Cache.GetBank(0) == eBankParent)
                    {
                        break;
                    }
                    Cache.Pop();
                    AnimFinder.Pop();
                }
            }
            Cache.Push(eBank);
            AnimFinder.Push(eBank);
        }

        for (int weapon = 0; weapon < iNumWeapons; ++weapon)
        {
            const SoldierAnimationBank::WEAPON eWeapon = aeWeaponOrder[weapon];
            if (aeFindMap[eBank][eWeapon] == SoldierAnimationBank::INVALID_MAP)
            {
                continue;
            }

            bool bIsRootMap = SoldierAnimationBank::GetBankParent(eBank) == eBank;
            for (SoldierAnimationBank::WEAPON eWeaponIter = eWeapon; bIsRootMap;)
            {
                const SoldierAnimationBank::WEAPON eWeaponParent = SoldierAnimationBank::GetWeaponParent(eWeaponIter);
                if (eWeaponIter == eWeaponParent)
                {
                    break;
                }
                eWeaponIter = eWeaponParent;
                bIsRootMap = aeFindMap[eBank][eWeaponIter] == SoldierAnimationBank::INVALID_MAP;
            }
            const SoldierAnimation::SCOPE eDefaultScope =
                bIsRootMap ? SoldierAnimation::FULL_BODY : SoldierAnimation::UPPER_BODY;
            const bool bHasAlertState = SoldierAnimationBank::HasAlertState(eWeapon);
            const SoldierAnimationBank::WEAPON eRoot = SoldierAnimationBank::GetWeaponRoot(eWeapon);
            const SoldierAnimationBank::COMBO eCombo = SoldierAnimationBank::GetWeaponCombo(eWeapon);
            AnimationMap* pMap = Cache.AddWeapon(eWeapon, &mMap[aeFindMap[eBank][eWeapon]]);
            AnimFinder.AddWeapon(eWeapon, 0);

            if (eRoot != SoldierAnimationBank::MELEE)
            {
                for (int posture = 0; posture < SoldierAnimType::NUM_STANDARD_POSTURES; ++posture)
                {
                    for (int weaponAnim = 0; weaponAnim < SoldierAnimType::NUM_WEAPON_ANIMATIONS; ++weaponAnim)
                    {
                        const unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex(
                            (SoldierAnimType::Posture)posture, (SoldierAnimType::WeaponAnimation)weaponAnim);
                        for (int scope = 0; scope < SoldierAnimType::NUM_SCOPES; ++scope)
                        {
                            const char* szAnimName = SoldierAnimType::AnimIndexToName(uiAnimIndex, SoldierAnimType::NUM_SCOPES);
                            AnimFinder.SetAnimationData(SoldierAnimationData::Get(uiAnimIndex, (SoldierAnimType::Scope)scope),
                                                        SoldierAnimation::UPPER_BODY);
                            AnimFinder.SetTargetAnimation(_GetAnimationIndex((SoldierAnimType::Posture)posture,
                                                                             (SoldierAnimType::WeaponAnimation)weaponAnim,
                                                                             (SoldierAnimType::Scope)scope));
                            if (AnimFinder.AssignAnimation(szAnimName, eWeapon, 0))
                            {
                                continue;
                            }
                            AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                            if (!MapCache::IsInvalid(&pCacheMap->mAnimation[AnimFinder.m_iAnim]))
                            {
                                continue;
                            }
                            if (Cache.GetDepth() > 1)
                            {
                                Cache.AddBankParent(eWeapon, 0);
                                SoldierAnimation* pAnim = AnimFinder.AssignAnimation(szAnimName, eWeapon, 1);
                                if (!pAnim)
                                {
                                    pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeapon, 1, false);
                                }
                                pCacheMap->mAnimation[AnimFinder.m_iAnim] = pAnim;
                            }
                            else
                            {
                                pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                            }
                        }
                    }
                }
            }
            else if (eCombo != SoldierAnimationBank::INVALID_COMBO)
            {
                for (int weaponAnim = 0; weaponAnim < SoldierAnimationBank::GetNumComboAnimations(); ++weaponAnim)
                {
                    const unsigned int uiComboIndex =
                        SoldierAnimationBank::GetComboAnimationIndex(eCombo, (SoldierAnimationBank::COMBO_ANIMATION)weaponAnim);
                    if (uiComboIndex >= SoldierAnimationBank::GetComboNumAnimations(eCombo))
                    {
                        continue;
                    }
                    const unsigned char uiAnimIndex =
                        SoldierAnimType::GetAnimIndex((SoldierAnimationBank::COMBO_ANIMATION)weaponAnim);
                    for (int scope = 0; scope < SoldierAnimType::NUM_SCOPES; ++scope)
                    {
                        const char* szAnimName = SoldierAnimType::AnimIndexToName(uiAnimIndex, SoldierAnimType::NUM_SCOPES);
                        AnimFinder.SetAnimationData(SoldierAnimationData::Get(uiAnimIndex, (SoldierAnimType::Scope)scope),
                                                    SoldierAnimation::UPPER_BODY);
                        AnimFinder.SetTargetAnimation(_GetAnimationIndex((SoldierAnimationBank::COMBO_ANIMATION)weaponAnim,
                                                                         (SoldierAnimType::Scope)scope));
                        if (AnimFinder.AssignAnimation(szAnimName, eWeapon, 0))
                        {
                            continue;
                        }
                        AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                        if (!MapCache::IsInvalid(&pCacheMap->mAnimation[AnimFinder.m_iAnim]))
                        {
                            continue;
                        }
                        if (Cache.GetDepth() > 1)
                        {
                            Cache.AddBankParent(eWeapon, 0);
                            SoldierAnimation* pAnim = AnimFinder.AssignAnimation(szAnimName, eWeapon, 1);
                            if (!pAnim)
                            {
                                pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeapon, 1, false);
                            }
                            pCacheMap->mAnimation[AnimFinder.m_iAnim] = pAnim;
                        }
                        else
                        {
                            pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                        }
                    }
                }
            }

            for (int posture = 0; posture < SoldierAnimType::NUM_POSTURES; ++posture)
            {
                if (posture >= SoldierAnimType::NUM_STANDARD_POSTURES && !bHasAlertState)
                {
                    for (int movement = 0; movement < SoldierAnimType::NUM_MOVEMENT_ANIMATIONS; ++movement)
                    {
                        pMap->mMovementAnimation[posture][movement][SoldierAnimType::UPPER] =
                            pMap->mMovementAnimation[posture - SoldierAnimType::NUM_ALERT_POSTURES][movement][SoldierAnimType::UPPER];
                        pMap->mMovementAnimation[posture][movement][SoldierAnimType::LOWER] =
                            pMap->mMovementAnimation[posture - SoldierAnimType::NUM_ALERT_POSTURES][movement][SoldierAnimType::LOWER];
                    }
                    continue;
                }
                for (int movement = 0; movement < SoldierAnimType::NUM_MOVEMENT_ANIMATIONS; ++movement)
                {
                    const unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex(
                        (SoldierAnimType::Posture)posture, (SoldierAnimType::MovementAnimation)movement);
                    const bool bTurn = movement == SoldierAnimType::TURN_LEFT || movement == SoldierAnimType::TURN_RIGHT;
                    for (int scope = 0; scope < SoldierAnimType::NUM_SCOPES; ++scope)
                    {
                        const char* szAnimName = SoldierAnimType::AnimIndexToName(uiAnimIndex, SoldierAnimType::NUM_SCOPES);
                        AnimFinder.SetAnimationData(SoldierAnimationData::Get(uiAnimIndex, (SoldierAnimType::Scope)scope),
                                                    bTurn ? SoldierAnimation::LOWER_BODY : eDefaultScope);
                        AnimFinder.SetTargetAnimation(_GetAnimationIndex((SoldierAnimType::Posture)posture,
                                                                         (SoldierAnimType::MovementAnimation)movement,
                                                                         (SoldierAnimType::Scope)scope));
                        if (AnimFinder.AssignAnimation(szAnimName, eWeapon, 0))
                        {
                            continue;
                        }
                        if (scope)
                        {
                            if (posture >= SoldierAnimType::NUM_STANDARD_POSTURES)
                            {
                                AnimFinder.AssignAnimationFromWeaponAlertParent(szAnimName, eWeapon, 0, false);
                                continue;
                            }
                            AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                            if (!MapCache::IsInvalid(&pCacheMap->mAnimation[AnimFinder.m_iAnim]))
                            {
                                continue;
                            }
                            const SoldierAnimationBank::WEAPON eWeaponParent = SoldierAnimationBank::GetWeaponParent(eWeapon);
                            if (eWeaponParent == eWeapon)
                            {
                                pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                                continue;
                            }
                            Cache.AddWeaponParent(eWeapon, 0);
                            SoldierAnimation* pAnim = AnimFinder.AssignAnimation(szAnimName, eWeaponParent, 0);
                            if (!pAnim)
                            {
                                pAnim = AnimFinder.AssignAnimationFromWeaponParent(szAnimName, eWeaponParent, 0, false);
                            }
                            pCacheMap->mAnimation[AnimFinder.m_iAnim] = pAnim;
                        }
                        else
                        {

                            if (bTurn)
                            {
                                pMap->mMovementAnimation[posture][movement][SoldierAnimType::UPPER] = 0;
                                continue;
                            }
                            if (movement == SoldierAnimType::IDLE_TAKEKNEE &&
                                (posture == SoldierAnimType::CROUCH || posture == SoldierAnimType::CROUCH_ALERT))
                            {
                                pMap->mMovementAnimation[posture][SoldierAnimType::IDLE_TAKEKNEE][SoldierAnimType::UPPER] = 0;
                                continue;
                            }
                            AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                            if (!MapCache::IsInvalid(&pCacheMap->mAnimation[AnimFinder.m_iAnim]))
                            {
                                continue;
                            }
                            if (Cache.GetDepth() <= 1)
                            {
                                pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                                continue;
                            }
                            Cache.AddBankParent(eWeapon, 0);
                            SoldierAnimation* pAnim = AnimFinder.AssignAnimation(szAnimName, eWeapon, 1);
                            if (!pAnim)
                            {
                                pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeapon, 1, false);
                            }
                            pCacheMap->mAnimation[AnimFinder.m_iAnim] = pAnim;
                        }
                    }
                }
            }

            for (int action = 0; action < SoldierAnimType::NUM_ACTION_ANIMATIONS; ++action)
            {
                const unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex((SoldierAnimType::ActionAnimation)action);
                for (int scope = 0; scope < SoldierAnimType::NUM_SCOPES; ++scope)
                {
                    const char* szAnimName = SoldierAnimType::AnimIndexToName(uiAnimIndex, SoldierAnimType::NUM_SCOPES);
                    AnimFinder.SetAnimationData(SoldierAnimationData::Get(uiAnimIndex, (SoldierAnimType::Scope)scope),
                                                eDefaultScope);
                    AnimFinder.SetTargetAnimation(
                        _GetAnimationIndex((SoldierAnimType::ActionAnimation)action, (SoldierAnimType::Scope)scope));
                    if (AnimFinder.AssignAnimation(szAnimName, eWeapon, 0))
                    {
                        continue;
                    }
                    AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                    if (!MapCache::IsInvalid(&pCacheMap->mAnimation[AnimFinder.m_iAnim]))
                    {
                        continue;
                    }
                    SoldierAnimation* pAnim;
                    if (scope)
                    {
                        const SoldierAnimationBank::WEAPON eWeaponParent = SoldierAnimationBank::GetWeaponParent(eWeapon);
                        if (eWeaponParent == eWeapon)
                        {
                            pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                            continue;
                        }
                        Cache.AddWeaponParent(eWeapon, 0);
                        pAnim = AnimFinder.AssignAnimation(szAnimName, eWeaponParent, 0);
                        if (!pAnim)
                        {
                            pAnim = AnimFinder.AssignAnimationFromWeaponParent(szAnimName, eWeaponParent, 0, false);
                        }
                    }
                    else
                    {
                        if (Cache.GetDepth() <= 1)
                        {
                            pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                            continue;
                        }
                        Cache.AddBankParent(eWeapon, 0);
                        pAnim = AnimFinder.AssignAnimation(szAnimName, eWeapon, 1);
                        if (!pAnim)
                        {
                            pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeapon, 1, false);
                        }
                    }
                    pCacheMap->mAnimation[AnimFinder.m_iAnim] = pAnim;
                }
            }

            for (int customAnim = 0; customAnim < iNumCustomAnimations; ++customAnim)
            {
                const char* szAnimName = SoldierAnimationBank::GetAnimationName((SoldierAnimationBank::ANIMATION)customAnim);
                AnimFinder.SetTargetCustomAnimation(customAnim);
                AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                unsigned int uiAnim = pCacheMap->mCustomAnimation[customAnim];
                if (MapCache::IsInvalid(&uiAnim))
                {
                    const ZephyrAnim* pAnim = AnimFinder.FindZephyrAnimation(szAnimName, eWeapon, 0);
                    if (!pAnim)
                    {
                        AnimFinder.AssignCustomAnimationFromWeaponParent(szAnimName, eWeapon, 0, false);
                        continue;
                    }
                    uiAnim = pAnim->GetNameCRC();
                    pCacheMap->mCustomAnimation[AnimFinder.m_iAnim] = uiAnim;
                }
                if (!uiAnim)
                {
                    AnimFinder.AssignCustomAnimationFromWeaponParent(szAnimName, eWeapon, 0, false);
                }
            }
        }

        for (int weapon = 0; weapon < iNumWeapons; ++weapon)
        {
            const SoldierAnimationBank::WEAPON eWeapon = aeWeaponOrder[weapon];
            if (aeFindMap[eBank][eWeapon] == SoldierAnimationBank::INVALID_MAP)
            {
                continue;
            }

            const bool bRootBank = SoldierAnimationBank::GetBankParent(eBank) == eBank;
            bool bIsRootMap = bRootBank;
            for (SoldierAnimationBank::WEAPON eWeaponIter = eWeapon; bIsRootMap;)
            {
                const SoldierAnimationBank::WEAPON eWeaponParent = SoldierAnimationBank::GetWeaponParent(eWeaponIter);
                if (eWeaponIter == eWeaponParent)
                {
                    break;
                }
                eWeaponIter = eWeaponParent;
                bIsRootMap = aeFindMap[eBank][eWeaponIter] == SoldierAnimationBank::INVALID_MAP;
            }
            const SoldierAnimation::SCOPE eDefaultScope =
                bIsRootMap ? SoldierAnimation::FULL_BODY : SoldierAnimation::UPPER_BODY;
            const bool bHasAlertState = SoldierAnimationBank::HasAlertState(eWeapon);
            const SoldierAnimationBank::WEAPON eRoot = SoldierAnimationBank::GetWeaponRoot(eWeapon);
            const SoldierAnimationBank::COMBO eCombo = SoldierAnimationBank::GetWeaponCombo(eWeapon);
            AnimationMap* pMap = Cache.Map(eWeapon, 0);

            if (eRoot != SoldierAnimationBank::MELEE)
            {

                bool abMapCrouchToStand[4];
                abMapCrouchToStand[0] = abMapCrouchToStand[1] =
                    !pMap->mWeaponAnimation[SoldierAnimType::CROUCH][SoldierAnimType::SHOOT][SoldierAnimType::UPPER] &&
                    !pMap->mWeaponAnimation[SoldierAnimType::CROUCH][SoldierAnimType::SHOOT][SoldierAnimType::LOWER] &&
                    !pMap->mWeaponAnimation[SoldierAnimType::CROUCH][SoldierAnimType::SHOOT2][SoldierAnimType::UPPER] &&
                    !pMap->mWeaponAnimation[SoldierAnimType::CROUCH][SoldierAnimType::SHOOT2][SoldierAnimType::LOWER] &&
                    (pMap->mWeaponAnimation[SoldierAnimType::STAND][SoldierAnimType::SHOOT][SoldierAnimType::UPPER] ||
                     pMap->mWeaponAnimation[SoldierAnimType::STAND][SoldierAnimType::SHOOT][SoldierAnimType::LOWER] ||
                     pMap->mWeaponAnimation[SoldierAnimType::STAND][SoldierAnimType::SHOOT2][SoldierAnimType::UPPER] ||
                     pMap->mWeaponAnimation[SoldierAnimType::STAND][SoldierAnimType::SHOOT2][SoldierAnimType::LOWER]);
                abMapCrouchToStand[2] =
                    !pMap->mWeaponAnimation[SoldierAnimType::CROUCH][SoldierAnimType::SHOOT_SECONDARY][SoldierAnimType::UPPER] &&
                    !pMap->mWeaponAnimation[SoldierAnimType::CROUCH][SoldierAnimType::SHOOT_SECONDARY][SoldierAnimType::LOWER] &&
                    (pMap->mWeaponAnimation[SoldierAnimType::STAND][SoldierAnimType::SHOOT_SECONDARY][SoldierAnimType::UPPER] ||
                     pMap->mWeaponAnimation[SoldierAnimType::STAND][SoldierAnimType::SHOOT_SECONDARY][SoldierAnimType::LOWER]);
                abMapCrouchToStand[3] =
                    !pMap->mWeaponAnimation[SoldierAnimType::CROUCH][SoldierAnimType::SHOOT_SECONDARY2][SoldierAnimType::UPPER] &&
                    !pMap->mWeaponAnimation[SoldierAnimType::CROUCH][SoldierAnimType::SHOOT_SECONDARY2][SoldierAnimType::LOWER] &&
                    (pMap->mWeaponAnimation[SoldierAnimType::STAND][SoldierAnimType::SHOOT_SECONDARY2][SoldierAnimType::UPPER] ||
                     pMap->mWeaponAnimation[SoldierAnimType::STAND][SoldierAnimType::SHOOT_SECONDARY2][SoldierAnimType::LOWER]);

                for (int posture = 0; posture < SoldierAnimType::NUM_STANDARD_POSTURES; ++posture)
                {
                    for (int weaponAnim = 0; weaponAnim < SoldierAnimType::NUM_WEAPON_ANIMATIONS; ++weaponAnim)
                    {
                        SoldierAnimation** ppAnim = pMap->mWeaponAnimation[posture][weaponAnim];
                        if (ppAnim[SoldierAnimType::UPPER] && ppAnim[SoldierAnimType::LOWER])
                        {
                            continue;
                        }
                        const unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex(
                            (SoldierAnimType::Posture)posture, (SoldierAnimType::WeaponAnimation)weaponAnim);
                        for (int scope = 0; scope < SoldierAnimType::NUM_SCOPES; ++scope)
                        {
                            if (ppAnim[scope])
                            {
                                continue;
                            }
                            const char* szAnimName = SoldierAnimType::AnimIndexToName(uiAnimIndex, SoldierAnimType::NUM_SCOPES);
                            AnimFinder.SetAnimationData(SoldierAnimationData::Get(uiAnimIndex, (SoldierAnimType::Scope)scope),
                                                        SoldierAnimation::UPPER_BODY);
                            AnimFinder.SetTargetAnimation(_GetAnimationIndex((SoldierAnimType::Posture)posture,
                                                                             (SoldierAnimType::WeaponAnimation)weaponAnim,
                                                                             (SoldierAnimType::Scope)scope));
                            SoldierAnimation* pAnim = 0;
                            if (weaponAnim == SoldierAnimType::SHOOT)
                            {
                                pAnim = pMap->mWeaponAnimation[posture][SoldierAnimType::SHOOT2][scope];
                            }
                            else if (weaponAnim == SoldierAnimType::SHOOT2)
                            {
                                pAnim = pMap->mWeaponAnimation[posture][SoldierAnimType::SHOOT][scope];
                            }
                            if (posture == SoldierAnimType::CROUCH && weaponAnim <= SoldierAnimType::SHOOT_SECONDARY2 &&
                                abMapCrouchToStand[weaponAnim])
                            {
                                pAnim = pMap->mWeaponAnimation[SoldierAnimType::STAND][weaponAnim][scope];
                            }
                            if (!pAnim)
                            {
                                AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                                pAnim = pCacheMap->mAnimation[AnimFinder.m_iAnim];
                                if (MapCache::IsInvalid(&pAnim) || !pAnim)
                                {
                                    const SoldierAnimationBank::WEAPON eWeaponParent =
                                        SoldierAnimationBank::GetWeaponParent(eWeapon);
                                    if (eWeaponParent == eWeapon)
                                    {
                                        pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                                        pAnim = 0;
                                    }
                                    else
                                    {
                                        Cache.AddWeaponParent(eWeapon, 0);
                                        pAnim = AnimFinder.AssignAnimation(szAnimName, eWeaponParent, 0);
                                        if (!pAnim)
                                        {
                                            pAnim = AnimFinder.AssignAnimationFromWeaponParent(szAnimName, eWeaponParent, 0, true);
                                            if (!pAnim)
                                            {
                                                pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeaponParent, 0, true);
                                            }
                                        }
                                        pCacheMap->mAnimation[AnimFinder.m_iAnim] = pAnim;
                                    }
                                }

                                if (!pAnim)
                                {
                                    if (weaponAnim == SoldierAnimType::SHOOT_SECONDARY)
                                    {
                                        pAnim = pMap->mWeaponAnimation[posture][SoldierAnimType::SHOOT_SECONDARY2][scope];
                                    }
                                    else if (weaponAnim == SoldierAnimType::SHOOT_SECONDARY2)
                                    {
                                        pAnim = pMap->mWeaponAnimation[posture][SoldierAnimType::SHOOT_SECONDARY][scope];
                                    }
                                }
                            }
                            ppAnim[scope] = pAnim;

                            _stricmp(szAnimName, "prone");
                        }
                    }
                }
            }
            else if (eCombo != SoldierAnimationBank::INVALID_COMBO)
            {
                for (int weaponAnim = 0; weaponAnim < SoldierAnimationBank::GetNumComboAnimations(); ++weaponAnim)
                {
                    SoldierAnimation** ppAnim = pMap->mWeaponMeleeAnimation[weaponAnim];
                    const unsigned int uiComboIndex =
                        SoldierAnimationBank::GetComboAnimationIndex(eCombo, (SoldierAnimationBank::COMBO_ANIMATION)weaponAnim);
                    if (uiComboIndex >= SoldierAnimationBank::GetComboNumAnimations(eCombo) ||
                        (ppAnim[SoldierAnimType::UPPER] && ppAnim[SoldierAnimType::LOWER]))
                    {
                        continue;
                    }
                    const unsigned char uiAnimIndex =
                        SoldierAnimType::GetAnimIndex((SoldierAnimationBank::COMBO_ANIMATION)weaponAnim);
                    for (int scope = 0; scope < SoldierAnimType::NUM_SCOPES; ++scope)
                    {
                        if (ppAnim[scope])
                        {
                            continue;
                        }
                        const char* szAnimName = SoldierAnimType::AnimIndexToName(uiAnimIndex, SoldierAnimType::NUM_SCOPES);
                        AnimFinder.SetAnimationData(SoldierAnimationData::Get(uiAnimIndex, (SoldierAnimType::Scope)scope),
                                                    SoldierAnimation::UPPER_BODY);
                        AnimFinder.SetTargetAnimation(_GetAnimationIndex((SoldierAnimationBank::COMBO_ANIMATION)weaponAnim,
                                                                         (SoldierAnimType::Scope)scope));
                        AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                        SoldierAnimation* pAnim = pCacheMap->mAnimation[AnimFinder.m_iAnim];
                        if (MapCache::IsInvalid(&pAnim) || !pAnim)
                        {
                            const SoldierAnimationBank::WEAPON eWeaponParent = SoldierAnimationBank::GetWeaponParent(eWeapon);
                            if (eWeapon == SoldierAnimationBank::MELEE || eWeaponParent == eWeapon)
                            {
                                pAnim = 0;
                                pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                            }
                            else
                            {
                                Cache.AddWeaponParent(eWeapon, 0);
                                pAnim = AnimFinder.AssignAnimation(szAnimName, eWeaponParent, 0);
                                if (!pAnim)
                                {
                                    pAnim = AnimFinder.AssignAnimationFromWeaponMeleeParent(szAnimName, eWeaponParent, 0, true);
                                    if (!pAnim)
                                    {
                                        pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeaponParent, 0, true);
                                    }
                                }
                                pCacheMap->mAnimation[AnimFinder.m_iAnim] = pAnim;
                            }
                        }
                        ppAnim[scope] = pAnim;
                    }
                }
            }

            for (int posture = 0; posture < SoldierAnimType::NUM_POSTURES; ++posture)
            {
                if (posture >= SoldierAnimType::NUM_STANDARD_POSTURES && !bHasAlertState)
                {
                    for (int movement = 0; movement < SoldierAnimType::NUM_MOVEMENT_ANIMATIONS; ++movement)
                    {
                        SoldierAnimation** ppAnim = pMap->mMovementAnimation[posture][movement];
                        SoldierAnimation** ppStandard =
                            pMap->mMovementAnimation[posture - SoldierAnimType::NUM_ALERT_POSTURES][movement];
                        if (!ppAnim[SoldierAnimType::UPPER])
                        {
                            ppAnim[SoldierAnimType::UPPER] = ppStandard[SoldierAnimType::UPPER];
                        }
                        if (!ppAnim[SoldierAnimType::LOWER])
                        {
                            ppAnim[SoldierAnimType::LOWER] = ppStandard[SoldierAnimType::LOWER];
                        }
                    }
                    continue;
                }
                for (int movement = 0; movement < SoldierAnimType::NUM_MOVEMENT_ANIMATIONS; ++movement)
                {
                    SoldierAnimation** ppAnim = pMap->mMovementAnimation[posture][movement];
                    if (ppAnim[SoldierAnimType::UPPER] && ppAnim[SoldierAnimType::LOWER])
                    {
                        continue;
                    }
                    const unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex(
                        (SoldierAnimType::Posture)posture, (SoldierAnimType::MovementAnimation)movement);
                    const bool bTurn = movement == SoldierAnimType::TURN_LEFT || movement == SoldierAnimType::TURN_RIGHT;
                    for (int scope = 0; scope < SoldierAnimType::NUM_SCOPES; ++scope)
                    {
                        if (ppAnim[scope])
                        {
                            continue;
                        }
                        const char* szAnimName = SoldierAnimType::AnimIndexToName(uiAnimIndex, SoldierAnimType::NUM_SCOPES);
                        AnimFinder.SetAnimationData(SoldierAnimationData::Get(uiAnimIndex, (SoldierAnimType::Scope)scope),
                                                    bTurn ? SoldierAnimation::LOWER_BODY : eDefaultScope);
                        AnimFinder.SetTargetAnimation(_GetAnimationIndex((SoldierAnimType::Posture)posture,
                                                                         (SoldierAnimType::MovementAnimation)movement,
                                                                         (SoldierAnimType::Scope)scope));
                        SoldierAnimation* pAnim = 0;
                        if (scope)
                        {
                            pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeapon, 0, true);
                        }
                        else
                        {

                            if (bTurn)
                            {
                                pAnim = pMap->mMovementAnimation[posture][SoldierAnimType::IDLE][SoldierAnimType::UPPER];
                            }
                            else if (movement == SoldierAnimType::IDLE_TAKEKNEE &&
                                     (posture == SoldierAnimType::CROUCH || posture == SoldierAnimType::CROUCH_ALERT))
                            {
                                pAnim = pMap->mMovementAnimation[posture][SoldierAnimType::IDLE][SoldierAnimType::UPPER];
                                if (!pAnim && posture == SoldierAnimType::CROUCH_ALERT)
                                {
                                    pAnim = pMap->mMovementAnimation[SoldierAnimType::CROUCH][SoldierAnimType::IDLE]
                                                                    [SoldierAnimType::UPPER];
                                }
                            }
                            if (!pAnim)
                            {
                                if (posture >= SoldierAnimType::NUM_STANDARD_POSTURES)
                                {
                                    pAnim = AnimFinder.AssignAnimationFromWeaponAlertParent(szAnimName, eWeapon, 0, true);
                                }
                                else
                                {
                                    AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                                    pAnim = pCacheMap->mAnimation[AnimFinder.m_iAnim];
                                    if (MapCache::IsInvalid(&pAnim) || !pAnim)
                                    {
                                        const SoldierAnimationBank::WEAPON eWeaponParent =
                                            SoldierAnimationBank::GetWeaponParent(eWeapon);
                                        if (eWeaponParent == eWeapon)
                                        {
                                            pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                                            pAnim = 0;
                                        }
                                        else
                                        {
                                            Cache.AddWeaponParent(eWeapon, 0);
                                            pAnim = AnimFinder.AssignAnimation(szAnimName, eWeaponParent, 0);
                                            if (!pAnim)
                                            {
                                                pAnim = AnimFinder.AssignAnimationFromWeaponParent(szAnimName, eWeaponParent, 0,
                                                                                                    true);
                                                if (!pAnim)
                                                {
                                                    pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeaponParent,
                                                                                                      0, true);
                                                }
                                            }
                                            pCacheMap->mAnimation[AnimFinder.m_iAnim] = pAnim;
                                        }
                                    }
                                }
                            }
                        }

                        if (!pAnim)
                        {
                            if (posture >= SoldierAnimType::NUM_STANDARD_POSTURES)
                            {
                                pAnim = pMap->mMovementAnimation[posture - SoldierAnimType::NUM_ALERT_POSTURES][movement][scope];
                            }
                            if (!pAnim && bRootBank && movement <= SoldierAnimType::TURN_RIGHT)
                            {
                                pAnim = pMap->mMovementAnimation[posture][SoldierAnimType::IDLE][scope];
                                if (!pAnim)
                                {
                                    pAnim = pMap->mMovementAnimation[SoldierAnimType::STAND][SoldierAnimType::IDLE][scope];
                                }
                            }
                        }
                        ppAnim[scope] = pAnim;
                    }
                }
            }

            for (int action = 0; action < SoldierAnimType::NUM_ACTION_ANIMATIONS; ++action)
            {
                SoldierAnimation** ppAnim = pMap->mActionAnimation[action];
                if (ppAnim[SoldierAnimType::UPPER] && ppAnim[SoldierAnimType::LOWER])
                {
                    continue;
                }
                const unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex((SoldierAnimType::ActionAnimation)action);
                for (int scope = 0; scope < SoldierAnimType::NUM_SCOPES; ++scope)
                {
                    if (ppAnim[scope])
                    {
                        continue;
                    }
                    const char* szAnimName = SoldierAnimType::AnimIndexToName(uiAnimIndex, SoldierAnimType::NUM_SCOPES);
                    AnimFinder.SetAnimationData(SoldierAnimationData::Get(uiAnimIndex, (SoldierAnimType::Scope)scope),
                                                eDefaultScope);
                    AnimFinder.SetTargetAnimation(
                        _GetAnimationIndex((SoldierAnimType::ActionAnimation)action, (SoldierAnimType::Scope)scope));
                    SoldierAnimation* pAnim = 0;
                    if (scope == SoldierAnimType::LOWER)
                    {
                        pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeapon, 0, true);
                    }
                    if (!pAnim)
                    {

                        if ((action >= SoldierAnimType::JUMP_FORWARD && action <= SoldierAnimType::JUMP_RIGHT) ||
                            action == SoldierAnimType::JET)
                        {
                            pAnim = pMap->mActionAnimation[SoldierAnimType::JUMP_UP][scope];
                        }
                        else if (action == SoldierAnimType::SPRINT)
                        {
                            pAnim = pMap->mMovementAnimation[SoldierAnimType::STAND][SoldierAnimType::RUN_FORWARD][scope];
                        }
                        if (scope == SoldierAnimType::UPPER && !pAnim)
                        {
                            AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                            pAnim = pCacheMap->mAnimation[AnimFinder.m_iAnim];
                            if (MapCache::IsInvalid(&pAnim) || !pAnim)
                            {
                                const SoldierAnimationBank::WEAPON eWeaponParent =
                                    SoldierAnimationBank::GetWeaponParent(eWeapon);
                                if (eWeaponParent == eWeapon)
                                {
                                    pAnim = 0;
                                    pCacheMap->mAnimation[AnimFinder.m_iAnim] = 0;
                                }
                                else
                                {
                                    Cache.AddWeaponParent(eWeapon, 0);
                                    pAnim = AnimFinder.AssignAnimation(szAnimName, eWeaponParent, 0);
                                    if (!pAnim)
                                    {
                                        pAnim = AnimFinder.AssignAnimationFromWeaponParent(szAnimName, eWeaponParent, 0, true);
                                        if (!pAnim)
                                        {
                                            pAnim = AnimFinder.AssignAnimationFromBankParent(szAnimName, eWeaponParent, 0, true);
                                        }
                                    }
                                    pCacheMap->mAnimation[AnimFinder.m_iAnim] = pAnim;
                                }
                            }
                        }
                    }
                    ppAnim[scope] = pAnim;
                }
            }

            for (int customAnim = 0; customAnim < iNumCustomAnimations; ++customAnim)
            {
                if (pMap->mCustomAnimation[customAnim])
                {
                    continue;
                }
                const char* szAnimName = SoldierAnimationBank::GetAnimationName((SoldierAnimationBank::ANIMATION)customAnim);
                AnimFinder.SetTargetCustomAnimation(customAnim);
                AnimationMap* pCacheMap = Cache.Map(eWeapon, 0);
                unsigned int uiAnim = pCacheMap->mCustomAnimation[customAnim];
                if (MapCache::IsInvalid(&uiAnim) || !uiAnim)
                {
                    if (Cache.GetDepth() <= 1)
                    {
                        uiAnim = 0;
                        pCacheMap->mCustomAnimation[customAnim] = 0;
                    }
                    else
                    {
                        Cache.AddBankParent(eWeapon, 0);
                        AnimationMap* pParentMap = Cache.Map(eWeapon, 1);
                        uiAnim = pParentMap->mCustomAnimation[AnimFinder.m_iAnim];
                        if (MapCache::IsInvalid(&uiAnim))
                        {
                            const ZephyrAnim* pAnim = AnimFinder.FindZephyrAnimation(szAnimName, eWeapon, 1);
                            if (pAnim)
                            {
                                uiAnim = pAnim->GetNameCRC();
                                pParentMap->mCustomAnimation[AnimFinder.m_iAnim] = uiAnim;
                            }
                            else
                            {
                                uiAnim = 0;
                            }
                        }
                        if (!uiAnim)
                        {
                            uiAnim = AnimFinder.AssignCustomAnimationFromBankParent(szAnimName, eWeapon, 1, true);
                            if (!uiAnim)
                            {
                                uiAnim = AnimFinder.AssignCustomAnimationFromWeaponParent(szAnimName, eWeapon, 1, true);
                            }
                        }
                        pCacheMap->mCustomAnimation[AnimFinder.m_iAnim] = uiAnim;
                    }
                }
                pMap->mCustomAnimation[customAnim] = uiAnim;
            }
        }
    }

    for (int map = 0; map < SoldierAnimationBank::GetNumMaps(); ++map)
    {
        const SoldierAnimationBank::WEAPON eWeapon = SoldierAnimationBank::GetMapWeapon((SoldierAnimationBank::MAP)map);
        if (SoldierAnimationBank::GetWeaponRoot(eWeapon) != SoldierAnimationBank::MELEE ||
            SoldierAnimationBank::GetWeaponCombo(eWeapon) == SoldierAnimationBank::INVALID_COMBO)
        {
            continue;
        }
        for (int weaponAnim = 0; weaponAnim < SoldierAnimationBank::GetNumComboAnimations(); ++weaponAnim)
        {
            for (int scope = 0; scope < SoldierAnimType::NUM_SCOPES; ++scope)
            {
                if (MapCache::IsInvalid(&mMap[map].mWeaponMeleeAnimation[weaponAnim][scope]))
                {
                    mMap[map].mWeaponMeleeAnimation[weaponAnim][scope] = 0;
                }
            }
        }
    }

    SoldierAnimationData::DiscardLoadTimeData();

    mSoldierAnimators = new SoldierAnimator[50];
    if (mSoldierAnimators)
    {
        for (int i = 0; i < 50; ++i)
        {
            mSoldierAnimators[i].Initialize(mZephyrSkeletonShared);
            mLRU_List.Add(&mSoldierAnimators[i].GetLRUNode(), &mSoldierAnimators[i]);
        }
    }

    mJetPackMatrix = *_GetBoneMatrix(
        0xe5b51072, mMap[SoldierAnimationBank::HUMAN_RIFLE].mActionAnimation[SoldierAnimType::JET][SoldierAnimType::UPPER],
        tempZephyrSkeleton, tempZephyrPoseDyn, tempZephyrPoseStatic);

    {
        const unsigned int uiBoneHead = 0x8afdf5e4;
        const unsigned int uiBoneTorso = 0xe5b51072;
        const unsigned int uiBoneAbdomen = 0x8c1c9bba;
        const unsigned int uiBoneASpine = 0x93e93819;
        const unsigned int uiBoneAcklay = 0x7a89c91f;
        const unsigned int uiBoneRoot = 0x16e27226;
        const int NUM_SAMPLES = 4;

        // GLOBAL: SWBF2 0x01fac234
        static const float NUM_SAMPLES_INV = 1.0f / NUM_SAMPLES;
        // GLOBAL: SWBF2 0x01fac204
        static const PblVector3 s_vHeadOffset(0.035f, 0.03f, 0.0f);
        // GLOBAL: SWBF2 0x01fac1f8
        static const PblVector3 s_vTorsoOffset(-0.12f, 0.12f, 0.0f);
        // GLOBAL: SWBF2 0x01fac228
        static const PblVector3 s_vTorsoOffset_Jump(-0.12f, -0.05f, 0.0f);
        // GLOBAL: SWBF2 0x01fac21c
        static const PblVector3 s_vPivotAxisOffset(0.26f, -0.16f, 0.0f);
        // GLOBAL: SWBF2 0x01fac210
        static const PblVector3 s_vPivotAxisOffset_Crouch(0.1f, 0.05f, 0.0f);
        // GLOBAL: SWBF2 0x007ae770
        static const SoldierAnimType::Posture s_aPosture[4] = { SoldierAnimType::STAND, SoldierAnimType::STAND_ALERT,
                                                                SoldierAnimType::CROUCH, SoldierAnimType::CROUCH_ALERT };
        // GLOBAL: SWBF2 0x007ae780
        static const SoldierAnimType::MovementAnimation s_aMoveAnim[4] = {
            SoldierAnimType::IDLE, SoldierAnimType::WALK_FORWARD, SoldierAnimType::RUN_BACKWARD,
            SoldierAnimType::RUN_FORWARD
        };
        PblMatrix mBone;

        for (SoldierAnimationBank::MAP eMap = (SoldierAnimationBank::MAP)0; eMap < SoldierAnimationBank::GetNumMaps();
             eMap = (SoldierAnimationBank::MAP)(eMap + 1))
        {
            const ZephyrSkeletonShared* pSkeleton = _GetAnimationMapSkeleton(eMap);
            if (pSkeleton->GetJointByCrc(uiBoneAcklay))
            {
                break;
            }
            unsigned int uiBonePitch = uiBoneAbdomen;
            if (!pSkeleton->GetJointByCrc(uiBoneAbdomen))
            {
                uiBonePitch = uiBoneASpine;
            }

            SoldierOrdnanceCollisionTable& table = m_aOrdCollTable[eMap];
            float fAnimDuration;
            int iEntry = 0;
            for (int iPosture = 0; iPosture < 4; ++iPosture)
            {
                const PblVector3& vPitchAxisOffset = (iPosture & 1) ? s_vPivotAxisOffset_Crouch : s_vPivotAxisOffset;
                for (int iAnim = 0; iAnim < 4; ++iAnim, ++iEntry)
                {
                    SoldierAnimType::MovementAnimation eMoveAnim = s_aMoveAnim[iAnim];
                    const SoldierAnimation* pAnim =
                        GetUpperBodyMovementAnimation(eMoveAnim, s_aPosture[iPosture], eMap, false);
                    if (!pAnim && eMoveAnim >= SoldierAnimType::WALK_FORWARD)
                    {

                        eMoveAnim = (SoldierAnimType::MovementAnimation)(
                            ((eMoveAnim - SoldierAnimType::WALK_FORWARD) ^ 1) + SoldierAnimType::WALK_FORWARD);
                        pAnim = GetUpperBodyMovementAnimation(eMoveAnim, s_aPosture[iPosture], eMap, false);
                    }
                    fAnimDuration = (pAnim->GetAnimationNumFrames() - 1) * ANIM_FRAME_SPEED;
                    table.avHeadPos[iEntry] = table.avTorsoPos[iEntry] = table.avPitchAxis[iEntry] = gVectorZero;
                    for (int iStep = 0; iStep < NUM_SAMPLES; ++iStep)
                    {
                        PblVector3 vHead;
                        GetAnimationBoneMatrix(eMap, s_aPosture[iPosture], eMoveAnim, uiBoneHead,
                                               iStep * NUM_SAMPLES_INV * fAnimDuration, mBone);
                        D3DXVec3TransformCoord(&vHead, &s_vHeadOffset, &mBone);
                        table.avHeadPos[iEntry].x += vHead.x * NUM_SAMPLES_INV;
                        table.avHeadPos[iEntry].y += vHead.y * NUM_SAMPLES_INV;
                        table.avHeadPos[iEntry].z += vHead.z * NUM_SAMPLES_INV;

                        PblVector3 vTorso;
                        GetAnimationBoneMatrix(eMap, s_aPosture[iPosture], eMoveAnim, uiBoneTorso,
                                               iStep * NUM_SAMPLES_INV * fAnimDuration, mBone);
                        D3DXVec3TransformCoord(&vTorso, &s_vTorsoOffset, &mBone);
                        table.avTorsoPos[iEntry].x += vTorso.x * NUM_SAMPLES_INV;
                        table.avTorsoPos[iEntry].y += vTorso.y * NUM_SAMPLES_INV;
                        table.avTorsoPos[iEntry].z += vTorso.z * NUM_SAMPLES_INV;

                        PblVector3 vPitch;
                        GetAnimationBoneMatrix(eMap, s_aPosture[iPosture], eMoveAnim, uiBonePitch,
                                               iStep * NUM_SAMPLES_INV * fAnimDuration, mBone);
                        D3DXVec3TransformCoord(&vPitch, &vPitchAxisOffset, &mBone);
                        table.avPitchAxis[iEntry].x += vPitch.x * NUM_SAMPLES_INV;
                        table.avPitchAxis[iEntry].y += vPitch.y * NUM_SAMPLES_INV;
                        table.avPitchAxis[iEntry].z += vPitch.z * NUM_SAMPLES_INV;
                    }
                    table.avPitchAxis[iEntry].x = 0.0f;
                }
            }

            const SoldierAnimation* pAnim = GetUpperBodyActionAnimation(SoldierAnimType::JUMP_UP, eMap);
            fAnimDuration = (pAnim->GetAnimationNumFrames() - 1) * ANIM_FRAME_SPEED;
            PblVector3 vJumpHead;
            GetAnimationBoneMatrix(eMap, SoldierAnimType::JUMP_UP, uiBoneHead, fAnimDuration, mBone);
            D3DXVec3TransformCoord(&vJumpHead, &s_vHeadOffset, &mBone);
            table.avHeadPos[16] = vJumpHead;
            PblVector3 vJumpTorso;
            GetAnimationBoneMatrix(eMap, SoldierAnimType::JUMP_UP, uiBoneTorso, fAnimDuration, mBone);
            D3DXVec3TransformCoord(&vJumpTorso, &s_vTorsoOffset_Jump, &mBone);
            table.avTorsoPos[16] = vJumpTorso;

            GetAnimationBoneMatrix(eMap, SoldierAnimType::JUMP_UP, uiBoneRoot, fAnimDuration, mBone);
            table.avPitchAxis[16] = mBone.GetTranslation();
            GetAnimationBoneMatrix(eMap, SoldierAnimType::STAND, SoldierAnimType::IDLE, uiBoneRoot, 0.0f, mBone);
            table.avPitchAxis[16].x -= mBone.GetTranslation().x;
            table.avPitchAxis[16].y -= mBone.GetTranslation().y;
            table.avPitchAxis[16].z -= mBone.GetTranslation().z;

            table.avHeadPos[17] = table.avTorsoPos[17] = PblVector3(0.0f, 0.6f, 0.0f);
            table.avHeadPos[18] = table.avTorsoPos[18] = PblVector3(0.0f, 1.1f, 0.0f);

            for (int iCustom = 0; iCustom < SoldierAnimationBank::GetNumAnimations(); ++iCustom)
            {
                // GLOBAL: SWBF2 0x01fac1f4
                static const float FRAME_9POSE_CENTER = 4.0f / 30.0f;
                PblVector3 vHead;
                GetAnimationBoneMatrix(eMap, (SoldierAnimationBank::ANIMATION)iCustom, uiBoneHead, FRAME_9POSE_CENTER,
                                       mBone);
                D3DXVec3TransformCoord(&vHead, &s_vHeadOffset, &mBone);
                table.avHeadPos[19 + iCustom] = vHead;
                PblVector3 vTorso;
                GetAnimationBoneMatrix(eMap, (SoldierAnimationBank::ANIMATION)iCustom, uiBoneTorso, FRAME_9POSE_CENTER,
                                       mBone);
                D3DXVec3TransformCoord(&vTorso, &s_vTorsoOffset, &mBone);
                table.avTorsoPos[19 + iCustom] = vTorso;
            }
        }
    }

    tempZephyrSkeleton.Close();
    tempZephyrPoseStatic.Close();
    tempZephyrPoseDyn.Close();
}
