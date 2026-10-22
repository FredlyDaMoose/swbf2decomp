#include "SoldierAnimatorLowRes.h"

#include <math.h>
#include <stdio.h>

#include "Combo.h"
#include "EntitySoldier.h"
#include "SoldierAnimation.h"
#include "SoldierAnimationData.h"
#include "SoldierAnimatorClass.h"
#include "SoldierAnimType.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrAnim.h"
#include "Zephyr/Source/ZephyrAnimBank.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"

static const char* const HUMAN_LOWRES_BANK = "humanlz";

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

namespace GameLoop { extern float sClientDeltaTime; }

extern PblVector3 gVectorZero;

extern "C" PblMatrix* __stdcall D3DXMatrixRotationAxis(PblMatrix* pOut, const PblVector3* pV, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationX(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationY(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

// GLOBAL: SWBF2 0x007e99d0
SoldierAnimatorLowResClass::PoseClass SoldierAnimatorLowResClass::s_PoseClasses[7] = {
    { "rifle_stand_idle_emote", 1 },
    { "rifle_crouch_idle_emote", 1 },
    { "rifle_crouch_idle_takeknee", 1 },
    { "rifle_jetpack_hover", 1 },
    { "rifle_diveforward", 1 },
    { "rifle_thrown_flail", 1 },
    { "", 1 },
};

// GLOBAL: SWBF2 0x007e9a08
SoldierAnimatorLowResClass::AnimClass SoldierAnimatorLowResClass::s_AnimClasses[2] = {
    { "rifle_stand_runforward", 0 },
    { "rifle_stand_death_backward", 0 },
};

// GLOBAL: SWBF2 0x007e9a20
SoldierAnimatorLowResClass::AnimPoolClass SoldierAnimatorLowResClass::s_AnimPoolClasses[2] = {
    { &s_AnimClasses[0], 1, 3, true },
    { &s_AnimClasses[1], 1, 8, false },
};

// GLOBAL: SWBF2 0x01eafb24
SoldierAnimatorLowResClass* SoldierAnimatorLowResClass::s_pHead;

// GLOBAL: SWBF2 0x01eafb20
SoldierAnimatorLowResClass* SoldierAnimatorLowResClass::s_pTail;

// FUNCTION: SWBF2 0x00647c90
const RedPose* SoldierAnimatorLowRes::GetPose()
{
    return &mPose;
}

// FUNCTION: SWBF2 0x00647ca0
int SoldierAnimatorLowRes::GetFramesSinceUsed() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00647cb0
SoldierAnimatorLowRes_UnsharedPool::~SoldierAnimatorLowRes_UnsharedPool()
{
}

// FUNCTION: SWBF2 0x00647cc0
bool SoldierAnimatorLowRes_UnsharedPool::GetRootMatrixAnimation(const EntitySoldier& soldier,
                                                                PblMatrix& kMatrix) const
{
    return false;
}

// FUNCTION: SWBF2 0x00647cd0
SoldierAnimatorLowRes_HighResPool::~SoldierAnimatorLowRes_HighResPool()
{
}

// FUNCTION: SWBF2 0x00647ce0
void SoldierAnimatorLowResClass::Create()
{
    s_pTail = s_pHead = new SoldierAnimatorLowResClass(HUMAN_LOWRES_BANK, NULL);
}

namespace
{
inline ZephyrAnim* FindBankAnimation(const RedAnimation* pAnimation, unsigned int uiHash)
{
    if (!pAnimation->_pZephyrAnimBank)
    {
        return NULL;
    }
    return const_cast<ZephyrAnim*>(pAnimation->_pZephyrAnimBank->Find(uiHash));
}

inline void FindLowResAnimation(ZephyrAnim*& pAnim, char* acName, int iPrefixLen, int iSuffixSize,
                                const char* pcAnimName, RedAnimation* const* apAnimation, int iNumAnimations)
{
    int iLen = iPrefixLen + _snprintf(&acName[iPrefixLen], iSuffixSize, pcAnimName);
    char* pcEnd = &acName[iLen];
    _snprintf(pcEnd, 256 - iLen, "_full");
    unsigned int uiFullHash = PblTEMPHash(acName);
    *pcEnd = 0;
    unsigned int uiHash = PblTEMPHash(acName);

    pAnim = FindBankAnimation(apAnimation[0], uiHash);
    if (!pAnim)
    {
        pAnim = FindBankAnimation(apAnimation[0], uiFullHash);
        for (int i = 1; i < iNumAnimations && !pAnim; ++i)
        {
            pAnim = FindBankAnimation(apAnimation[i], uiHash);
            if (!pAnim)
            {
                pAnim = FindBankAnimation(apAnimation[i], uiFullHash);
            }
        }
    }
}

struct HighResPoseClass
{
    SoldierAnimationBank::MAP eMap;
    unsigned char uiAnimIndex;
    unsigned short uiFrame;
};

inline void AddHighResPose(HighResPoseClass* aHighResPoseClass, int& iNumHighResPoses,
                           SoldierAnimationBank::MAP eMap, unsigned char uiAnimIndex)
{

    if (uiAnimIndex >= 164)
    {
        return;
    }

    const SoldierAnimationData* pData = SoldierAnimationData::Get(uiAnimIndex, SoldierAnimType::UPPER);
    if (!pData->IsLowResPose())
    {
        return;
    }

    int i;
    for (i = 0; i < iNumHighResPoses; ++i)
    {
        if (aHighResPoseClass[i].eMap == eMap && aHighResPoseClass[i].uiAnimIndex == uiAnimIndex)
        {
            break;
        }
    }
    if (i != iNumHighResPoses)
    {
        return;
    }

    if (!SoldierAnimatorClass::Instance()->GetUpperBodyAnimation(eMap, uiAnimIndex) ||
        !SoldierAnimatorClass::Instance()->GetUpperBodyAnimation(eMap, uiAnimIndex)->GetAnimation())
    {
        return;
    }

    aHighResPoseClass[i].eMap = eMap;
    aHighResPoseClass[i].uiAnimIndex = uiAnimIndex;
    aHighResPoseClass[i].uiFrame = pData->GetLowResPoseFrame();
    iNumHighResPoses = i + 1;
}
}

// FUNCTION: SWBF2 0x00647d40
void SoldierAnimatorLowResClass::PostLoad()
{
    int iNumPoses = SoldierAnimationBank::s_iNumAnimations + NUM_BASE_POSES;

    for (SoldierAnimatorLowResClass* pClass = s_pHead; pClass; pClass = pClass->m_pNext)
    {
        pClass->m_aPose = new Pose[iNumPoses];

        PblBitVector<32> kMask;
        SetupJointMask(pClass->m_pZephyrSkeletonShared, pClass->m_pZephyrSkeletonShared, kMask);

        char acName[256] = "_0";
        HighResPoseClass aHighResPoseClass[256];

        RedAnimation* apAnimation[11] = { RedAnimation::Find(pClass->m_uiNameHash) };
        int iNumAnimations = 1;
        if (!apAnimation[0])
        {
            apAnimation[0] = RedAnimation::Find(PblHash(pClass->m_uiNameHash, acName));
            for (iNumAnimations = 1; iNumAnimations < 10; ++iNumAnimations)
            {
                acName[1] = (char)('0' + iNumAnimations);
                apAnimation[iNumAnimations] = RedAnimation::Find(PblHash(pClass->m_uiNameHash, acName));
                if (!apAnimation[iNumAnimations])
                {
                    break;
                }
            }
        }

        int iPrefixLen = _snprintf(acName, sizeof(acName), "%s_", pClass->m_szName);
        int iSuffixSize = sizeof(acName) - iPrefixLen;

        int i;
        for (i = 0; i < NUM_BASE_POSES; ++i)
        {
            Pose* pPose = &pClass->m_aPose[i];
            FindLowResAnimation(pPose->m_pZephyrAnim, acName, iPrefixLen, iSuffixSize, s_PoseClasses[i].m_szName,
                                apAnimation, iNumAnimations);
            if (!pPose->m_pZephyrAnim)
            {
                if (pClass->m_pParent)
                {
                    pPose->m_pZephyrAnim = pClass->m_pParent->m_aPose[i].m_pZephyrAnim;
                }
                if (!pPose->m_pZephyrAnim)
                {
                    pPose->m_pZephyrAnim =
                        pClass->m_aPose[i == POSE_CROUCH_TAKEKNEE ? POSE_CROUCH : POSE_STAND].m_pZephyrAnim;
                }
            }

            pPose->m_pAnimator = new SoldierAnimatorLowRes;
            pPose->m_pAnimator->Initialize(pClass->m_pZephyrSkeletonShared, kMask, &s_PoseClasses[i], pPose,
                                           (POSE)i);
        }

        for (; i < iNumPoses; ++i)
        {
            Pose* pPose = &pClass->m_aPose[i];
            FindLowResAnimation(pPose->m_pZephyrAnim, acName, iPrefixLen, iSuffixSize,
                                SoldierAnimationBank::GetAnimationName(
                                    (SoldierAnimationBank::ANIMATION)(i - NUM_BASE_POSES)),
                                apAnimation, iNumAnimations);
            if (!pPose->m_pZephyrAnim && pClass->m_pParent)
            {
                pPose->m_pZephyrAnim = pClass->m_pParent->m_aPose[i].m_pZephyrAnim;
            }

            if (!pPose->m_pZephyrAnim)
            {
                pPose->m_pAnimator = NULL;
                continue;
            }

            pPose->m_pAnimator = new SoldierAnimatorLowRes;
            pPose->m_pAnimator->Initialize(pClass->m_pZephyrSkeletonShared, kMask, &s_PoseClasses[POSE_CUSTOM],
                                           pPose, (POSE)i);
        }

        for (int iPool = 0; iPool < NUM_BASE_ANIMPOOLS; ++iPool)
        {
            const AnimPoolClass* pPoolClass = &s_AnimPoolClasses[iPool];
            ZephyrAnim** apZephyrAnim = new ZephyrAnim*[pPoolClass->m_iNumAnims];

            ZephyrAnim** apParentAnim;
            if (!pClass->m_pParent)
            {
                apParentAnim = NULL;
            }
            else if (pPoolClass->m_bLoop)
            {
                apParentAnim = pClass->m_pParent->m_AnimPool.m_apZephyrAnim;
            }
            else
            {
                apParentAnim = pClass->m_pParent->m_UnsharedAnimPool.m_apZephyrAnim;
            }

            for (int j = 0; j < pPoolClass->m_iNumAnims; ++j)
            {
                FindLowResAnimation(apZephyrAnim[j], acName, iPrefixLen, iSuffixSize,
                                    pPoolClass->m_aAnims[j].m_szName, apAnimation, iNumAnimations);
                if (!apZephyrAnim[j])
                {
                    if (apParentAnim)
                    {
                        apZephyrAnim[j] = apParentAnim[j];
                    }
                    if (!apZephyrAnim[j])
                    {
                        apZephyrAnim[j] = pClass->m_aPose[POSE_STAND].m_pZephyrAnim;
                    }
                }
            }

            if (pPoolClass->m_bLoop)
            {
                AnimPool* pPool = &pClass->m_AnimPool;
                pPool->m_apZephyrAnim = apZephyrAnim;
                pPool->m_iLastUsed = 0;
                pPool->m_apAnimator = new SoldierAnimatorLowRes_Pool[pPoolClass->m_iPoolSize];
                for (int j = 0; j < pPoolClass->m_iPoolSize; ++j)
                {
                    pPool->m_apAnimator[j].Initialize(pClass->m_pZephyrSkeletonShared, kMask, pPoolClass, pPool,
                                                      (ANIMPOOL)iPool);
                }
            }
            else
            {
                UnsharedAnimPool* pPool = &pClass->m_UnsharedAnimPool;
                pPool->m_apZephyrAnim = apZephyrAnim;
                pPool->m_apAnimator = new SoldierAnimatorLowRes_UnsharedPool[pPoolClass->m_iPoolSize];
                for (int j = 0; j < pPoolClass->m_iPoolSize; ++j)
                {
                    pPool->m_apAnimator[j].Initialize(pClass->m_pZephyrSkeletonShared, kMask, pPoolClass, pPool,
                                                      (ANIMPOOL)iPool);
                }
            }
        }

        ZephyrSkeletonShared* pHighResSkeleton = NULL;
        pClass->m_pHighResAnimPool = NULL;
        pClass->m_aHighResPose = NULL;
        pClass->m_iNumHighResPoses = 0;

        for (int iMap = SoldierAnimationBank::NUM_STANDARD_MAPS; iMap < SoldierAnimationBank::s_iNumMaps; ++iMap)
        {
            SoldierAnimationBank::MAP eMap = (SoldierAnimationBank::MAP)iMap;
            SoldierAnimationBank::COMBO eCombo =
                SoldierAnimationBank::GetWeaponCombo(SoldierAnimationBank::GetMapWeapon(eMap));
            if (eCombo == SoldierAnimationBank::INVALID_COMBO)
            {
                continue;
            }

            SoldierAnimationBank::BANK eBank = SoldierAnimationBank::GetMapBank(eMap);
            if (SoldierAnimationBank::GetLowResBankNameHash(SoldierAnimationBank::GetBankLowResBank(eBank)) !=
                pClass->m_uiNameHash)
            {
                continue;
            }

            unsigned int uiBankHash = SoldierAnimationBank::GetBankNameHash(eBank);
            RedAnimation* pAnimation = RedAnimation::Find(uiBankHash);
            if (!pAnimation)
            {
                pAnimation = RedAnimation::Find(PblHash(uiBankHash, "_0"));
                if (!pAnimation)
                {
                    continue;
                }
            }

            if (!pHighResSkeleton)
            {
                pHighResSkeleton = pAnimation->GetZephyrSkeletonShared();
                SetupJointMask(pHighResSkeleton, pClass->m_pZephyrSkeletonShared, kMask);
                if (!pHighResSkeleton)
                {
                    continue;
                }
            }

            if (!pClass->m_pHighResAnimPool)
            {
                pClass->m_pHighResAnimPool = new HighResAnimPool;
                pClass->m_pHighResAnimPool->m_iPoolSize = 8;
                pClass->m_pHighResAnimPool->m_apAnimator =
                    new SoldierAnimatorLowRes_HighResPool[pClass->m_pHighResAnimPool->m_iPoolSize];
                for (int j = 0; j < pClass->m_pHighResAnimPool->m_iPoolSize; ++j)
                {
                    pClass->m_pHighResAnimPool->m_apAnimator[j].Initialize(pHighResSkeleton, kMask);
                }
            }

            const Combo* pCombo = Combo::Find(SoldierAnimationBank::GetComboNameHash(eCombo));
            if (!pCombo)
            {
                continue;
            }

            for (unsigned int uiState = 1; uiState < pCombo->GetNumStates(); ++uiState)
            {
                const Combo::State* pState = pCombo->GetState(uiState);
                if (pState->GetNumAttacks())
                {
                    AddHighResPose(aHighResPoseClass, pClass->m_iNumHighResPoses, eMap, pState->GetAnimIndex());
                }
                else if (const Combo::Deflect* pDeflect = pState->GetDeflect())
                {
                    AddHighResPose(aHighResPoseClass, pClass->m_iNumHighResPoses, eMap, pState->GetAnimIndex());
                    for (unsigned int j = 0; j < pDeflect->GetNumDeflectAnimations(); ++j)
                    {
                        AddHighResPose(aHighResPoseClass, pClass->m_iNumHighResPoses, eMap,
                                       pDeflect->GetDeflectAnimIndex(j));
                    }
                }
            }
        }

        if (pClass->m_iNumHighResPoses)
        {
            pClass->m_aHighResPose = new HighResPose[pClass->m_iNumHighResPoses];

            PoseClass kPoseClass;
            kPoseClass.m_szName = NULL;
            for (int j = 0; j < pClass->m_iNumHighResPoses; ++j)
            {
                HighResPose* pPose = &pClass->m_aHighResPose[j];
                kPoseClass.m_iFrame = aHighResPoseClass[j].uiFrame + 1;
                pPose->eMap = aHighResPoseClass[j].eMap;
                pPose->uiAnimIndex = aHighResPoseClass[j].uiAnimIndex;
                pPose->m_pZephyrAnim = const_cast<ZephyrAnim*>(
                    SoldierAnimatorClass::Instance()->GetUpperBodyAnimation(pPose->eMap, pPose->uiAnimIndex)
                        ->GetAnimation());
                pPose->m_pAnimator = new SoldierAnimatorLowRes;
                pPose->m_pAnimator->Initialize(pHighResSkeleton, kMask, &kPoseClass, pPose,
                                               (POSE)(POSE_HIGHRES + j));
            }
        }
    }
}

// SYNTHETIC: SWBF2 0x00648bb0
// SoldierAnimatorLowRes_Pool::`vector deleting destructor'

// SYNTHETIC: SWBF2 0x00648c10
// SoldierAnimatorLowRes_UnsharedPool::`vector deleting destructor'

// SYNTHETIC: SWBF2 0x00648c70
// SoldierAnimatorLowRes_HighResPool::`vector deleting destructor'

// FUNCTION: SWBF2 0x00648cd0
void SoldierAnimatorLowResClass::AddBank(const char* pcName, const char* pcParent)
{
    if (!pcParent)
    {
        pcParent = HUMAN_LOWRES_BANK;
    }

    unsigned int uiNameHash = PblHash(pcName);
    unsigned int uiParentHash = PblHash(pcParent);

    RedAnimation* pAnimation = RedAnimation::Find(uiNameHash);
    if (!pAnimation)
    {
        pAnimation = RedAnimation::Find(PblHash(uiNameHash, "_0"));
        if (!pAnimation)
        {
            return;
        }
    }

    ZephyrSkeletonShared* pSkeleton = pAnimation->GetZephyrSkeletonShared();
    if (!pSkeleton)
    {
        return;
    }

    SoldierAnimatorLowResClass* pParent = NULL;
    if (uiParentHash != 0x20fd0e45)
    {
        for (SoldierAnimatorLowResClass* pClass = s_pHead; pClass; pClass = pClass->m_pNext)
        {
            if (pClass->m_pZephyrSkeletonShared == pSkeleton)
            {
                return;
            }
            if (pClass->m_uiNameHash == uiParentHash)
            {
                pParent = pClass;
            }
        }
    }

    SoldierAnimatorLowResClass* pClass = new SoldierAnimatorLowResClass(pcName, pParent);
    s_pTail->m_pNext = pClass;
    s_pTail = pClass;
}

// FUNCTION: SWBF2 0x00648dd0
void SoldierAnimatorLowResClass::Destroy()
{
    while (s_pHead)
    {
        SoldierAnimatorLowResClass* pNext = s_pHead->m_pNext;
        delete s_pHead;
        s_pHead = pNext;
    }
    s_pTail = NULL;
}

// FUNCTION: SWBF2 0x00648e10
SoldierAnimatorLowRes* SoldierAnimatorLowResClass::GetAnimator(const EntitySoldier& soldier)
{
    ZephyrSkeletonShared* pSkeleton = soldier.mClass->mSpecialSkeletonLowRes;

    SoldierAnimatorLowResClass* pClass;
    for (pClass = s_pHead; pClass; pClass = pClass->m_pNext)
    {
        if (pClass->m_pZephyrSkeletonShared == pSkeleton)
        {
            break;
        }
    }
    if (!pClass)
    {
        pClass = s_pHead;
    }

    return pClass->GetAnimatorLocal(soldier);
}

// FUNCTION: SWBF2 0x00648e40
SoldierAnimatorLowResClass::SoldierAnimatorLowResClass(const char* pcName,
                                                       const SoldierAnimatorLowResClass* pParent)
    : m_pNext(NULL),
      m_pParent(const_cast<SoldierAnimatorLowResClass*>(pParent)),
      m_aPose(NULL),
      m_pHighResAnimPool(NULL),
      m_aHighResPose(NULL),
      m_iNumHighResPoses(0)
{
    unsigned int uiNameHash = PblHash(pcName);

    RedAnimation* pAnimation = RedAnimation::Find(uiNameHash);
    if (!pAnimation)
    {
        pAnimation = RedAnimation::Find(PblHash(uiNameHash, "_0"));
    }

    m_pZephyrSkeletonShared = pAnimation->GetZephyrSkeletonShared();
    strlcpy(m_szName, pcName, sizeof(m_szName) - 1);
    m_uiNameHash = uiNameHash;

    SetupJointTransformAnimationFlag(m_pZephyrSkeletonShared);
}

// FUNCTION: SWBF2 0x00648ee0
SoldierAnimatorLowResClass::~SoldierAnimatorLowResClass()
{
    int iNumPoses = SoldierAnimationBank::s_iNumAnimations + NUM_BASE_POSES;

    if (m_aPose)
    {
        for (int i = 0; i < iNumPoses; ++i)
        {
            delete m_aPose[i].m_pAnimator;
        }
        delete[] m_aPose;
    }

    delete[] m_AnimPool.m_apAnimator;
    delete[] m_AnimPool.m_apZephyrAnim;

    delete[] m_UnsharedAnimPool.m_apAnimator;
    delete[] m_UnsharedAnimPool.m_apZephyrAnim;

    if (m_pHighResAnimPool)
    {
        delete[] m_pHighResAnimPool->m_apAnimator;
        delete m_pHighResAnimPool;
    }

    if (m_aHighResPose)
    {
        for (int i = 0; i < m_iNumHighResPoses; ++i)
        {
            delete m_aHighResPose[i].m_pAnimator;
        }
        delete[] m_aHighResPose;
    }
}

// FUNCTION: SWBF2 0x00648ff0
SoldierAnimatorLowRes* SoldierAnimatorLowResClass::GetAnimatorLocal(const EntitySoldier& soldier)
{
    POSE ePose;
    ANIMPOOL eAnimPool = ANIMPOOL_POSE;
    unsigned char uiAnimIndex = soldier.m_uiWeaponAnimIndex;

    if (m_pHighResAnimPool && SoldierAnimType::IsWeaponMeleeAnimIndex(uiAnimIndex))
    {
        SoldierAnimationBank::MAP eMap = soldier.m_eSoldierAnimationMap;

        int i;
        for (i = 0; i < m_iNumHighResPoses; ++i)
        {
            if (m_aHighResPose[i].eMap == eMap && m_aHighResPose[i].uiAnimIndex == uiAnimIndex)
            {
                break;
            }
        }

        if (i < m_iNumHighResPoses)
        {
            if (soldier.mSoldierAnimatorLowRes)
            {
                soldier.mSoldierAnimatorLowRes->GetOwnerAnimator(soldier, (POSE)(POSE_HIGHRES + i), ANIMPOOL_POSE);
            }
            return m_aHighResPose[i].m_pAnimator;
        }

        SoldierAnimatorLowRes_HighResPool* pAnimator = NULL;
        if (soldier.mSoldierAnimatorLowRes)
        {
            pAnimator = static_cast<SoldierAnimatorLowRes_HighResPool*>(
                soldier.mSoldierAnimatorLowRes->GetOwnerAnimator(soldier, POSE_ANIMPOOL, ANIMPOOL_HIGHRES));
            if (pAnimator)
            {
                pAnimator->SetAnim(eMap, uiAnimIndex);
                return pAnimator;
            }
        }

        int iOldest = 0;
        float fOldest = 0.0f;
        for (i = 0; i < m_pHighResAnimPool->m_iPoolSize; ++i)
        {
            pAnimator = &m_pHighResAnimPool->m_apAnimator[i];
            if (!pAnimator->m_pOwner)
            {
                break;
            }
            if (pAnimator->mZephyrPoseDyn.m_fCurT > fOldest)
            {
                fOldest = pAnimator->mZephyrPoseDyn.m_fCurT;
                iOldest = i;
            }
        }
        if (i == m_pHighResAnimPool->m_iPoolSize)
        {
            pAnimator = &m_pHighResAnimPool->m_apAnimator[iOldest];
        }

        pAnimator->m_pOwner = &soldier;
        pAnimator->ResetAnim(eMap, uiAnimIndex);
        return pAnimator;
    }

    switch (soldier.mState)
    {
    case SoldierState::STAND:
    {

        float fMinSpeed =
            (soldier.mSoldierAnimatorLowRes && soldier.mSoldierAnimatorLowRes->m_eAnimPool == ANIMPOOL_MOVE) ? 0.2f
                                                                                                                 : 0.3f;
        const PblVector3& vVelocity = soldier.GetVelocity();
        if (vVelocity.x * vVelocity.x + vVelocity.z * vVelocity.z > fMinSpeed * fMinSpeed)
        {
            ePose = POSE_ANIMPOOL;
            eAnimPool = ANIMPOOL_MOVE;
        }
        else
        {
            ePose = POSE_STAND;
        }
        break;
    }
    case SoldierState::CROUCH:
    {
        const PblVector3& vVelocity = soldier.GetVelocity();
        ePose = vVelocity.x * vVelocity.x + vVelocity.z * vVelocity.z > 0.09f ? POSE_CROUCH : POSE_CROUCH_TAKEKNEE;
        break;
    }
    case SoldierState::PRONE:
        ePose = POSE_CROUCH;
        break;
    case SoldierState::SPRINT:
        ePose = POSE_ANIMPOOL;
        eAnimPool = ANIMPOOL_MOVE;
        break;
    case SoldierState::JUMP:
    case SoldierState::JET_JUMP:
    case SoldierState::JET_HOVER:
        ePose = POSE_JET;
        break;
    case SoldierState::ROLL:
        ePose = POSE_ROLL;
        break;
    case SoldierState::FALL:
    case SoldierState::FLY:
    case SoldierState::TUMBLE:
    case SoldierState::BOUNCE:
        ePose = POSE_FLY;
        break;
    case SoldierState::DEAD:
        ePose = POSE_ANIMPOOL;
        eAnimPool = ANIMPOOL_DEATH;
        break;
    case SoldierState::PILOT_ANIMATION:
        if (soldier.m_ePilotAnimation == SoldierAnimationBank::INVALID_ANIMATION)
        {
            return NULL;
        }
        ePose = (POSE)(POSE_CUSTOM + soldier.m_ePilotAnimation);
        break;
    default:
        ePose = POSE_STAND;
        break;
    }

    if (ePose != POSE_ANIMPOOL)
    {
        if (soldier.mSoldierAnimatorLowRes)
        {
            soldier.mSoldierAnimatorLowRes->GetOwnerAnimator(soldier, ePose, ANIMPOOL_POSE);
        }
        return m_aPose[ePose].m_pAnimator;
    }

    const AnimPoolClass* pPoolClass = &s_AnimPoolClasses[eAnimPool];
    if (pPoolClass->m_bLoop)
    {
        if (soldier.mSoldierAnimatorLowRes)
        {
            SoldierAnimatorLowRes* pAnimator =
                soldier.mSoldierAnimatorLowRes->GetOwnerAnimator(soldier, POSE_ANIMPOOL, eAnimPool);
            if (pAnimator)
            {
                return pAnimator;
            }
        }

        int iIndex = m_AnimPool.m_iLastUsed++;
        if (m_AnimPool.m_iLastUsed >= pPoolClass->m_iPoolSize)
        {
            m_AnimPool.m_iLastUsed %= pPoolClass->m_iPoolSize;
        }
        return &m_AnimPool.m_apAnimator[iIndex];
    }

    SoldierAnimatorLowRes_UnsharedPool* pAnimator = NULL;
    if (soldier.mSoldierAnimatorLowRes)
    {
        pAnimator = static_cast<SoldierAnimatorLowRes_UnsharedPool*>(
            soldier.mSoldierAnimatorLowRes->GetOwnerAnimator(soldier, POSE_ANIMPOOL, eAnimPool));
        if (pAnimator)
        {
            return pAnimator;
        }
    }

    int iOldest = 0;
    float fOldest = 0.0f;
    int i;
    for (i = 0; i < pPoolClass->m_iPoolSize; ++i)
    {
        pAnimator = &m_UnsharedAnimPool.m_apAnimator[i];
        if (!pAnimator->m_pOwner)
        {
            break;
        }
        if (pAnimator->mZephyrPoseDyn.m_fCurT > fOldest)
        {
            fOldest = pAnimator->mZephyrPoseDyn.m_fCurT;
            iOldest = i;
        }
    }
    if (i == pPoolClass->m_iPoolSize)
    {
        pAnimator = &m_UnsharedAnimPool.m_apAnimator[iOldest];
    }

    float fTime = soldier.mStateTimer;
    if (soldier.mSoldierAnimatorLowRes && soldier.mSoldierAnimatorLowRes->m_ePose == POSE_FLY)
    {
        fTime = 0.5f;
    }
    pAnimator->SetAnim(m_UnsharedAnimPool.m_apZephyrAnim[0], fTime);
    pAnimator->m_pOwner = &soldier;
    return pAnimator;
}

// GLOBAL: SWBF2 0x007ae908
static const unsigned int s_auiNoRootJoints[2] = { 0xe6e47876, 0x2b960099 };
// GLOBAL: SWBF2 0x007ae910
static const unsigned int s_auiRootJoints[2] = { 0x16e27226, 0 };

// FUNCTION: SWBF2 0x00649380
void SoldierAnimatorLowResClass::SetupJointTransformAnimationFlag(ZephyrSkeletonShared* pSkeleton)
{
    if ((int)pSkeleton->m_kHashCRC2Joint.Find(0x7a89c91f) - 1 >= 0)
    {
        for (int i = 0; s_auiRootJoints[i]; ++i)
        {
            int iJoint = (int)pSkeleton->m_kHashCRC2Joint.Find(s_auiRootJoints[i]) - 1;
            pSkeleton->SetJointTransformAnimationFlag(iJoint, true);
        }
    }
    else
    {
        for (int i = 1; i < (int)pSkeleton->m_uNumJoints; ++i)
        {
            pSkeleton->SetJointTransformAnimationFlag(i, false);
        }

        for (int i = 0; i < 2; ++i)
        {
            int iJoint = (int)pSkeleton->m_kHashCRC2Joint.Find(s_auiNoRootJoints[i]) - 1;
            pSkeleton->SetJointTransformAnimationFlag(iJoint, true);
        }
    }
}

// FUNCTION: SWBF2 0x00649430
void SoldierAnimatorLowResClass::SetupJointMask(const ZephyrSkeletonShared* pSkeleton,
                                                const ZephyrSkeletonShared* pSource,
                                                PblBitVector<32>& kMask)
{
    if (pSkeleton == pSource)
    {
        kMask.SetAll();
        return;
    }

    kMask.ClearAll();
    for (int i = 0; i < (int)pSource->m_uNumJoints; ++i)
    {
        int iJoint = (int)pSkeleton->m_kHashCRC2Joint.Find(pSource->m_pJoints[i].m_kCrc) - 1;
        if (iJoint >= 0)
        {
            kMask.SetBit(iJoint);
        }
    }
}

// FUNCTION: SWBF2 0x006494a0
SoldierAnimatorLowRes::SoldierAnimatorLowRes()
    : m_ePose(SoldierAnimatorLowResClass::POSE_ANIMPOOL),
      m_eAnimPool(SoldierAnimatorLowResClass::ANIMPOOL_POSE)
{
}

// SYNTHETIC: SWBF2 0x00649520
// SoldierAnimatorLowRes::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00649550
SoldierAnimatorLowRes::~SoldierAnimatorLowRes()
{
}

// FUNCTION: SWBF2 0x00649560
void SoldierAnimatorLowRes::Initialize(const ZephyrSkeletonShared* pSkeleton, const PblBitVector<32>& kMask,
                                       const SoldierAnimatorLowResClass::PoseClass* pPoseClass,
                                       const SoldierAnimatorLowResClass::Pose* pPose,
                                       SoldierAnimatorLowResClass::POSE ePose)
{
    m_ePose = ePose;

    ZephyrPoseDyn<32> kPoseDyn;
    ZephyrPoseStatic<32> kPoseStatic;

    mZephyrSkeleton.Open(*pSkeleton, kPoseStatic);
    kPoseStatic.Open(mZephyrSkeleton);
    kPoseDyn.Open(mZephyrSkeleton);
    kPoseDyn.SetLoop(true);
    kPoseDyn.m_bInterpolate = false;
    kPoseDyn.SetAnimation(pPose->m_pZephyrAnim, 30.0f);

    if (pPose->m_pZephyrAnim)
    {
        kPoseDyn.SetCurTime(0.0f);

        int iNumFrames = pPose->m_pZephyrAnim->m_u16NumFrames;
        float fTime;
        if (iNumFrames <= 1 || pPoseClass->m_iFrame <= 1)
        {
            fTime = 0.0f;
        }
        else if (pPoseClass->m_iFrame >= iNumFrames)
        {
            fTime = 1.0f;
        }
        else
        {
            fTime = (float)(pPoseClass->m_iFrame - 1) / (float)iNumFrames;
        }

        kPoseDyn.Update(fTime);
        kPoseStatic.Set(kPoseDyn, true);
    }

    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix, kMask);
    mPose.ConvertFromZephyrPose(kPoseStatic);
    mZephyrSkeleton.Close();
}

// FUNCTION: SWBF2 0x00649710
bool SoldierAnimatorLowRes::GetRootMatrixAnimation(const EntitySoldier& soldier, PblMatrix& kMatrix) const
{
    if ((m_ePose == SoldierAnimatorLowResClass::POSE_CROUCH && !soldier.m_uiWeaponAnimIndex) ||
        m_ePose >= SoldierAnimatorLowResClass::POSE_HIGHRES)
    {
        const PblVector3& vVelocity = soldier.GetVelocity();
        const PblMatrix& kSoldier = soldier.GetMatrix();
        float fRight = vVelocity.x * kSoldier.m[0][0] + vVelocity.y * kSoldier.m[0][1] + vVelocity.z * kSoldier.m[0][2];
        float fFront = vVelocity.x * kSoldier.m[2][0] + vVelocity.y * kSoldier.m[2][1] + vVelocity.z * kSoldier.m[2][2];

        float fSpeedSq = fRight * fRight + fFront * fFront;
        if (fSpeedSq < 0.09f || fRight == 0.0f)
        {
            return false;
        }

        if (fFront < 0.0f)
        {
            fFront = -fFront;
            fRight = -fRight;
        }

        float fInvSpeed = 1.0f / sqrtf(fSpeedSq);
        PblVector3 vFront(fRight * fInvSpeed, 0.0f, fFront * fInvSpeed);
        PblVector3 vRight(vFront.z, 0.0f, -vFront.x);

        PblMatrix kRoot;
        *(PblVector3*)&kRoot.m[0][0] = vRight;
        kRoot.m[0][3] = 0.0f;
        *(PblVector3*)&kRoot.m[1][0] = gVectorUnitY;
        kRoot.m[1][3] = 0.0f;
        *(PblVector3*)&kRoot.m[2][0] = vFront;
        kRoot.m[2][3] = 0.0f;
        *(PblVector3*)&kRoot.m[3][0] = gVectorZero;
        kRoot.m[3][3] = 1.0f;
        kMatrix = kRoot;
        return true;
    }

    if (soldier.mState == SoldierState::ROLL)
    {
        static PblVector3 s_vRollCenter(0.0f, 0.5f, 0.0f);

        const PblVector3& vVelocity = soldier.GetVelocity();
        const PblMatrix& kSoldier = soldier.GetMatrix();
        float fYaw = atan2f(vVelocity.x * kSoldier.m[0][0] + vVelocity.y * kSoldier.m[0][1] +
                                vVelocity.z * kSoldier.m[0][2],
                            vVelocity.x * kSoldier.m[2][0] + vVelocity.y * kSoldier.m[2][1] +
                                vVelocity.z * kSoldier.m[2][2]);

        D3DXMatrixRotationX(&kMatrix, soldier.mStateTimer * 8.0f + 1.0f);
        PblVector3 vCenter;
        D3DXVec3TransformNormal(&vCenter, &s_vRollCenter, &kMatrix);
        kMatrix.m[3][0] = s_vRollCenter.x - vCenter.x;
        kMatrix.m[3][1] = s_vRollCenter.y - vCenter.y;
        kMatrix.m[3][2] = s_vRollCenter.z - vCenter.z;
        kMatrix.m[3][3] = 1.0f;

        PblMatrix kYaw;
        D3DXMatrixMultiply(&kMatrix, &kMatrix, D3DXMatrixRotationY(&kYaw, fYaw));
        return true;
    }

    if (soldier.mState != SoldierState::FLY && soldier.mState != SoldierState::TUMBLE)
    {
        return false;
    }

    const PblVector3& vVelocity = soldier.GetVelocity();
    const PblMatrix& kSoldier = soldier.GetMatrix();
    float fRight = vVelocity.x * kSoldier.m[0][0] + vVelocity.y * kSoldier.m[0][1] + vVelocity.z * kSoldier.m[0][2];
    float fFront = vVelocity.x * kSoldier.m[2][0] + vVelocity.y * kSoldier.m[2][1] + vVelocity.z * kSoldier.m[2][2];

    float fSpeed = 0.0f;
    PblVector3 vAxis;
    float fSpeedSq = fRight * fRight + fFront * fFront;
    if (fSpeedSq > 0.0001f)
    {
        fSpeed = sqrtf(fSpeedSq);
        float fInvSpeed = 1.0f / fSpeed;
        vAxis.x = fFront * fInvSpeed;
        vAxis.y = 0.0f * fInvSpeed;
        vAxis.z = -fRight * fInvSpeed;
    }
    else
    {
        vAxis = gVectorUnitX;
    }

    float fSpin = fSpeed * 2.0f;
    if (fSpin < 2.0f)
    {
        fSpin = 2.0f;
    }
    else if (fSpin > 6.0f)
    {
        fSpin = 6.0f;
    }

    D3DXMatrixRotationAxis(&kMatrix, &vAxis, soldier.mStateTimer * fSpin);
    kMatrix.m[3][0] = 0.0f;
    kMatrix.m[3][1] = 0.0f;
    kMatrix.m[3][2] = soldier.mState == SoldierState::FLY ? 1.0f : 0.0f;
    kMatrix.m[3][3] = 1.0f;
    return true;
}

// FUNCTION: SWBF2 0x00649b90
SoldierAnimatorLowRes* SoldierAnimatorLowRes::GetOwnerAnimator(const EntitySoldier& soldier,
                                                               SoldierAnimatorLowResClass::POSE ePose,
                                                               SoldierAnimatorLowResClass::ANIMPOOL eAnimPool)
{
    return ePose == m_ePose ? this : NULL;
}

// FUNCTION: SWBF2 0x00649bb0
SoldierAnimatorLowRes_Pool::SoldierAnimatorLowRes_Pool()
    : m_iLastFrame(-1)
{
}

// FUNCTION: SWBF2 0x00649c20
SoldierAnimatorLowRes_Pool::~SoldierAnimatorLowRes_Pool()
{
    mZephyrPoseDyn.Close();
    mZephyrPoseStatic.Close();
    mZephyrSkeleton.Close();
}

// FUNCTION: SWBF2 0x00649cb0
void SoldierAnimatorLowRes_Pool::Initialize(const ZephyrSkeletonShared* pSkeleton, const PblBitVector<32>& kMask,
                                            const SoldierAnimatorLowResClass::AnimPoolClass* pPoolClass,
                                            const SoldierAnimatorLowResClass::AnimPool* pPool,
                                            SoldierAnimatorLowResClass::ANIMPOOL eAnimPool)
{
    m_eAnimPool = eAnimPool;
    m_iLastFrame = -1;
    mZephyrJointMask = kMask;

    mZephyrSkeleton.Open(*pSkeleton, mZephyrPoseStatic);
    mZephyrPoseStatic.Open(mZephyrSkeleton);
    mZephyrPoseDyn.Open(mZephyrSkeleton);
    mPose.ConvertFromZephyrPose(mZephyrPoseStatic);

    mZephyrPoseDyn.m_bInterpolate = false;
    mZephyrPoseDyn.SetLoop(pPoolClass->m_bLoop);
    mZephyrPoseDyn.SetAnimation(pPool->m_apZephyrAnim[0], 30.0f);

    if (pPool->m_apZephyrAnim[0])
    {
        mZephyrPoseDyn.SetCurTime(0.0f);

        int iNumFrames = pPool->m_apZephyrAnim[0]->m_u16NumFrames;
        float fTime;
        if (iNumFrames <= 1)
        {
            fTime = 0.0f;
        }
        else
        {
            fTime = (float)(int)(this - pPool->m_apAnimator) * (float)(iNumFrames - 1) /
                    ((float)pPoolClass->m_iPoolSize * 30.0f);
        }

        mZephyrPoseDyn.Update(fTime);
        mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
    }

    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix, kMask);
}

// FUNCTION: SWBF2 0x00649e20
const RedPose* SoldierAnimatorLowRes_Pool::GetPose()
{
    int iFrame = RedRenderer::GetFrameNumber();
    float fElapsed = (NetGame::IsEnabled() || !GameLoop::IsGameOver()) ? GameLoop::sClientDeltaTime : 0.0f;

    if (iFrame != m_iLastFrame && fElapsed > 0.0f)
    {
        mZephyrPoseDyn.Update(fElapsed);
        mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
        mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix, mZephyrJointMask);
        m_iLastFrame = iFrame;
    }

    return &mPose;
}

// FUNCTION: SWBF2 0x00649ec0
SoldierAnimatorLowRes* SoldierAnimatorLowRes_Pool::GetOwnerAnimator(const EntitySoldier& soldier,
                                                                    SoldierAnimatorLowResClass::POSE ePose,
                                                                    SoldierAnimatorLowResClass::ANIMPOOL eAnimPool)
{
    if (ePose != SoldierAnimatorLowResClass::POSE_ANIMPOOL)
    {
        return NULL;
    }
    if (eAnimPool != m_eAnimPool)
    {
        return NULL;
    }
    return this;
}

// FUNCTION: SWBF2 0x00649ee0
bool SoldierAnimatorLowRes_Pool::GetRootMatrixAnimation(const EntitySoldier& soldier, PblMatrix& kMatrix) const
{
    if (m_eAnimPool != SoldierAnimatorLowResClass::ANIMPOOL_MOVE || soldier.m_uiWeaponAnimIndex)
    {
        return false;
    }

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, soldier.mLegAngle);

    PblVector3 vFront(fSin, 0.0f, fCos);
    PblVector3 vRight(fCos, 0.0f, -fSin);

    PblMatrix kRoot;
    *(PblVector3*)&kRoot.m[0][0] = vRight;
    kRoot.m[0][3] = 0.0f;
    *(PblVector3*)&kRoot.m[1][0] = gVectorUnitY;
    kRoot.m[1][3] = 0.0f;
    *(PblVector3*)&kRoot.m[2][0] = vFront;
    kRoot.m[2][3] = 0.0f;
    *(PblVector3*)&kRoot.m[3][0] = gVectorZero;
    kRoot.m[3][3] = 1.0f;
    kMatrix = kRoot;
    return true;
}

// FUNCTION: SWBF2 0x00649fc0
SoldierAnimatorLowRes_UnsharedPool::SoldierAnimatorLowRes_UnsharedPool()
    : m_pOwner(NULL)
{
    mLastFrameUsed = RedRenderer::GetFrameNumber() - 10;
}

// FUNCTION: SWBF2 0x0064a020
void SoldierAnimatorLowRes_UnsharedPool::Initialize(const ZephyrSkeletonShared* pSkeleton,
                                                    const PblBitVector<32>& kMask,
                                                    const SoldierAnimatorLowResClass::AnimPoolClass* pPoolClass,
                                                    const SoldierAnimatorLowResClass::UnsharedAnimPool* pPool,
                                                    SoldierAnimatorLowResClass::ANIMPOOL eAnimPool)
{
    m_eAnimPool = eAnimPool;
    m_iLastFrame = -1;
    mZephyrJointMask = kMask;

    mZephyrSkeleton.Open(*pSkeleton, mZephyrPoseStatic);
    mZephyrPoseStatic.Open(mZephyrSkeleton);
    mZephyrPoseDyn.Open(mZephyrSkeleton);
    mPose.ConvertFromZephyrPose(mZephyrPoseStatic);

    mZephyrPoseDyn.m_bInterpolate = false;
    mZephyrPoseDyn.SetLoop(pPoolClass->m_bLoop);
    mZephyrPoseDyn.SetAnimation(pPool->m_apZephyrAnim[0], 30.0f);

    if (pPool->m_apZephyrAnim[0])
    {
        mZephyrPoseDyn.SetCurTime(0.0f);
    }

    mLastFrameUsed = RedRenderer::GetFrameNumber() - 10;
}

// FUNCTION: SWBF2 0x0064a120
SoldierAnimatorLowRes* SoldierAnimatorLowRes_UnsharedPool::GetOwnerAnimator(
    const EntitySoldier& soldier, SoldierAnimatorLowResClass::POSE ePose,
    SoldierAnimatorLowResClass::ANIMPOOL eAnimPool)
{
    if (m_pOwner == &soldier)
    {
        if (ePose == SoldierAnimatorLowResClass::POSE_ANIMPOOL && eAnimPool == m_eAnimPool)
        {
            mLastFrameUsed = RedRenderer::GetFrameNumber();
            return this;
        }
        m_pOwner = NULL;
    }
    return NULL;
}

// FUNCTION: SWBF2 0x0064a170
int SoldierAnimatorLowRes_UnsharedPool::GetFramesSinceUsed() const
{
    return RedRenderer::GetFrameNumber() - mLastFrameUsed;
}

// FUNCTION: SWBF2 0x0064a180
void SoldierAnimatorLowRes_UnsharedPool::SetAnim(const ZephyrAnim* pAnim, float fTime)
{
    mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);
    if (pAnim)
    {
        mZephyrPoseDyn.SetCurTime(0.0f);
        mZephyrPoseDyn.Update(fTime);
    }
    m_iLastFrame = -1;
}

// FUNCTION: SWBF2 0x0064a220
SoldierAnimatorLowRes_HighResPool::SoldierAnimatorLowRes_HighResPool()
    : m_eMap(SoldierAnimationBank::INVALID_MAP),
      m_uiAnimIndex(0)
{
}

// FUNCTION: SWBF2 0x0064a250
void SoldierAnimatorLowRes_HighResPool::Initialize(const ZephyrSkeletonShared* pSkeleton,
                                                   const PblBitVector<32>& kMask)
{
    m_eMap = SoldierAnimationBank::INVALID_MAP;
    m_uiAnimIndex = 0;
    m_pOwner = NULL;
    m_eAnimPool = SoldierAnimatorLowResClass::ANIMPOOL_HIGHRES;
    mZephyrJointMask = kMask;

    mZephyrSkeleton.Open(*pSkeleton, mZephyrPoseStatic);
    mZephyrPoseStatic.Open(mZephyrSkeleton);
    mZephyrPoseDyn.Open(mZephyrSkeleton);
    mPose.ConvertFromZephyrPose(mZephyrPoseStatic);

    mZephyrPoseDyn.m_bInterpolate = false;
    mZephyrPoseDyn.SetLoop(false);
    mZephyrPoseDyn.SetAnimation(NULL, 30.0f);
}

// FUNCTION: SWBF2 0x0064a2f0
void SoldierAnimatorLowRes_HighResPool::SetAnim(SoldierAnimationBank::MAP eMap, unsigned char uiAnimIndex)
{
    if (eMap != m_eMap || uiAnimIndex != m_uiAnimIndex)
    {
        ResetAnim(eMap, uiAnimIndex);
    }
}

// FUNCTION: SWBF2 0x0064a320
void SoldierAnimatorLowRes_HighResPool::ResetAnim(SoldierAnimationBank::MAP eMap, unsigned char uiAnimIndex)
{
    m_eMap = eMap;
    m_uiAnimIndex = uiAnimIndex;
    const SoldierAnimation* pAnimation = SoldierAnimatorClass::Instance()->GetUpperBodyAnimation(eMap, uiAnimIndex);
    const ZephyrAnim* pAnim = pAnimation->GetAnimation();
    mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);
    if (pAnim)
    {
        mZephyrPoseDyn.SetCurTime(0.0f);
    }
    m_iLastFrame = -1;
}
