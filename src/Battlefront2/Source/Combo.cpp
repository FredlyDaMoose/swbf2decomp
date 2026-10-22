#include "Combo.h"

#include <math.h>
#include <string.h>

#include "PebbleFL/Common/PblConfig.h"
#include "SoldierAnimationBank.h"
#include "SoldierAnimationData.h"
#include "SoldierAnimType.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblRandom.h"
#include "SoldierAnimatorClass.h"
#include "SoldierAnimation.h"
#include "WeaponMelee.h"
#include "WeaponMeleeInstance.h"
#include "Zephyr/Source/ZephyrAnim.h"
#include "Zephyr/Source/ZephyrSkeleton.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "PebbleFL/Common/PblBitVector.h"
#include "PebbleFL/Common/PblMatrix.h"
#include <float.h>
#include <d3dx9math.h>

bool __fastcall StateHasBothResolveFlags(const void* pState);

// FUNCTION: SWBF2 0x00475640
const Combo* Combo::ResolveForWeapon(const WeaponMelee* pWeapon,
                                     const ZephyrSkeletonShared* pSkeleton) const
{
    const int eMap = pWeapon->mSoldierAnimationMap;
    if (m_eMap != -1 || m_pSkeleton)
    {
        return 0;
    }
    const unsigned int uiNameHash = Combo::GetNameHashForMap(m_uiNameHash, (SoldierAnimationBank::MAP)eMap);
    const Combo* pFound = s_ComboTable.Find(uiNameHash);
    if (pFound)
    {
        return pFound;
    }
    if (sMemoryPool.mUsed >= sMemoryPool.mCount)
    {
        return 0;
    }
    Combo* pResolved = static_cast<Combo*>(sMemoryPool.Allocate(0x24));
    if (pResolved)
    {
        pResolved->CopyConstructor(*this);
    }
    pResolved->m_uiNameHash = uiNameHash;
    pResolved->m_pSkeleton = const_cast<ZephyrSkeletonShared*>(pSkeleton);
    pResolved->m_eMap = eMap;
    if (pResolved->m_aState != m_aState)
    {
        for (unsigned int i = 0; i < pResolved->m_uiNumStates; ++i)
        {
            State& state = pResolved->m_aState[i];
            float fDuration = 0.0f;
            float fVelocityZ = 0.0f;
            float fVelocityX = 0.0f;
            if (state.m_uiDuration != State::DURATION_INVALID &&
                (!state.m_bAnimatedMove || !state.m_bVelocityZFromAnim) &&
                !StateHasBothResolveFlags(&state))
            {
                goto resolve_damage;
            }
            if (state.m_uiAnimIndex < 0xa4)
            {
                SoldierAnimatorClass* pAnimator = SoldierAnimatorClass::Instance();
                const SoldierAnimation* pAnim = pAnimator->GetUpperBodyAnimation(
                    (SoldierAnimationBank::MAP)eMap, (unsigned char)state.m_uiAnimIndex);
                if (pAnim)
                {
                    fDuration = (pAnim->m_pZephyrAnim->GetNumFrames() - 1) * 0.033333335f;
                    fVelocityZ = pAnim->m_iVelocityZ * 0.01f;
                    fVelocityX = pAnim->m_iVelocityX * 0.01f;
                }
            }
            if (state.m_uiDuration == State::DURATION_INVALID)
            {
                state.SetDuration(fDuration);
            }
            if (state.m_bVelocityZFromAnim && state.m_bAnimatedMove)
            {
                state.SetVelocityZ(state.m_iVelocityZ * 0.02f * fVelocityZ, false);
            }
            if (state.m_bVelocityXFromAnim && state.m_bAnimatedMove)
            {
                state.SetVelocityX(state.m_iVelocityX * 0.02f * fVelocityX, false);
            }
resolve_damage:
            const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(pWeapon->mClass);
            const unsigned int nEdges = pClass->m_uiNumDamageEdges;
            if (!nEdges)
            {
                continue;
            }
            for (unsigned int j = 0; j < state.m_uiNumAttacks; ++j)
            {
                Attack& attack = state.m_aAttack[j];
                if (attack.m_uiDamageEdge >= nEdges)
                {
                    continue;
                }
                unsigned int uiAttach;
                PblVector3 vBegin, vEnd;
                pClass->GetDamageEdgeRay(attack.m_uiDamageEdge, uiAttach, vBegin, vEnd);
                attack._ResolveDamageData(pResolved, &state, uiAttach,
                    vBegin, vEnd);
            }
        }
    }
    if (s_ComboTable.Store(uiNameHash, pResolved))
    {
        ++s_ComboTableNumEntries;
    }
    return pResolved;
}

// FUNCTION: SWBF2 0x00473840
bool Combo::Deflect::TestDeflect(const float* pMatrix, const float* pRight, const float* pAttack, int iUnused, unsigned char uiFallback, unsigned char* pResult) const
{
    const Combo::Deflect* pDeflect = (const Combo::Deflect*)this;
    const int iMin = (static_cast<int>(pDeflect->m_auiPackedData[0] << 19)) >> 23;
    const int iMax = (static_cast<int>(pDeflect->m_auiPackedData[0] << 10)) >> 23;
    if (iMin != iMax)
    {
        const float fAngle = static_cast<float>(atan2(
            -(pMatrix[1] * pAttack[1] + pMatrix[0] * pAttack[0] + pMatrix[2] * pAttack[2]),
            -(pMatrix[9] * pAttack[1] + pMatrix[8] * pAttack[0] + pMatrix[10] * pAttack[2])));
        const float fMin = iMin * 0.017453292f;
        const float fMax = iMax * 0.017453292f;
        if (iMin >= iMax ? (fAngle <= fMin && fAngle >= fMax)
                         : (fAngle < fMin || fAngle > fMax))
        {
            return false;
        }
    }
    const float fSide = pAttack[2] * pRight[0] - pRight[2] * pAttack[0];
    const float fAhead = -(pRight[0] * pAttack[0] + pRight[1] * pAttack[1] + pRight[2] * pAttack[2]);
    const unsigned int n = pDeflect->m_uiNumDeflectAnimations;
    unsigned char all = 0;
    for (unsigned int i = 0; i < n; ++i)
    {
        all |= pDeflect->m_aDeflectAnim[i].uiDirections;
    }
    if (!all)
    {
        *pResult = uiFallback;
        return true;
    }
    const bool bLeft = fSide < 0.0f;
    unsigned char mask;
    if (fAhead > 0.92387998f)
    {
        mask = 1;
        if (!(mask & all))
        {
            mask |= bLeft ? 2 : 8;
        }
        if (!(mask & all))
        {
            mask |= bLeft ? 12 : 18;
        }
    }
    else if (fAhead > 0.38268f)
    {
        mask = bLeft ? 2 : 8;
        if (!(mask & all))
        {
            mask |= bLeft ? 4 : 16;
        }
        if (!(mask & all))
        {
            mask |= 1;
        }
    }
    else if (fAhead > -0.70710999f)
    {
        mask = bLeft ? 4 : 16;
        if (!(mask & all))
        {
            mask |= bLeft ? 2 : 8;
        }
        if (!(mask & all))
        {
            mask |= 32;
        }
    }
    else
    {
        mask = 32;
        if (!(mask & all))
        {
            mask |= bLeft ? 4 : 16;
        }
        if (!(mask & all))
        {
            mask |= bLeft ? 18 : 12;
        }
    }
    if (!(mask & all))
    {
        mask = 63;
    }
    unsigned int matches = 0;
    unsigned int chosen = 0;
    for (unsigned int i = 0; i < n; ++i)
    {
        if (pDeflect->m_aDeflectAnim[i].uiDirections & mask)
        {
            ++matches;
            chosen = i;
        }
    }
    if (!matches)
    {
        *pResult = uiFallback;
    }
    else if (matches == 1)
    {
        *pResult = pDeflect->m_aDeflectAnim[chosen].uiAnimIndex;
    }
    else
    {
        unsigned int pick = static_cast<unsigned int>(PblRandom::GetGlobalFloat() * matches);
        for (unsigned int i = 0; i < n; ++i)
        {
            if ((pDeflect->m_aDeflectAnim[i].uiDirections & mask) && pick-- == 0)
            {
                *pResult = pDeflect->m_aDeflectAnim[i].uiAnimIndex;
                break;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00470ea0
bool __fastcall StateHasBothResolveFlags(const void* pState)
{
    unsigned int value = static_cast<const Combo::State*>(pState)->m_auiPackedData[3];
    return (value & 0x00880000u) == 0x00880000u;
}

// FUNCTION: SWBF2 0x00475d60
void* Combo::_AllocDamageSample()
{
    MemoryPool& pool = Combo::DamageSample::sMemoryPool;
    if (pool.mUsed >= pool.mCount)
    {
        return 0;
    }
    return pool.Allocate(0x1c);
}

// FUNCTION: SWBF2 0x00473280
void Combo::Attack::AddDamageSample(float fTime, const PblVector3& vBegin, const PblVector3& vEnd)
{
    Combo::Attack* pAttack = (Combo::Attack*)this;
    char* pSample = static_cast<char*>(Combo::_AllocDamageSample());
    if (!pSample)
    {
        return;
    }
    if (pAttack->m_uiNumDamageSamples == 0)
    {
        pAttack->m_aDamageSample = reinterpret_cast<Combo::DamageSample*>(pSample);
    }
    if (pAttack->m_uiNumDamageSamples < 255)
    {
        ++pAttack->m_uiNumDamageSamples;
        *reinterpret_cast<float*>(pSample) = fTime;
        memcpy(pSample + 4, &vBegin, sizeof(PblVector3));
        memcpy(pSample + 16, &vEnd, sizeof(PblVector3));
    }
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x004727a0
bool Combo::Attack::_ResolveDamageData(const Combo* pCombo, const State* pState,
                                       unsigned int uiAttach, const PblVector3& vBegin,
                                       const PblVector3& vEnd)
{
    static float fTimeStep = 1.0f / 30.0f;
    static float fMinTimeStep = 1.0f / 300.0f;
    static float fMaxMoveSq = 0.25f;

    const int iJoint = pCombo->m_pSkeleton->GetJointIdxByCrc(uiAttach);
    if (iJoint < 0)
    {
        return false;
    }

    ZephyrSkeleton<32> kSkel;
    ZephyrPoseStatic<32> kPose;
    ZephyrPoseDyn<32> kUpperPose;
    ZephyrPoseDyn<32> kLowerPose;
    kSkel.Open(*pCombo->m_pSkeleton, kPose);
    kPose.Open(kSkel);
    kUpperPose.Open(kSkel);
    kLowerPose.Open(kSkel);

    PblBitVector<32> kUpperMask;
    PblBitVector<32> kLowerMask;
    SoldierAnimatorClass::SetupBodyMasks(pCombo->m_pSkeleton, kUpperMask, kLowerMask);
    bool bLowerAnim = true;
    kUpperMask.SetBit(iJoint);
    kLowerMask.ClearBit(iJoint);

    const SoldierAnimationBank::MAP eMap = (SoldierAnimationBank::MAP)pCombo->m_eMap;
    const unsigned char uiAnimIndex = pState->GetAnimIndex();
    const SoldierAnimation* pUpperAnim =
        SoldierAnimatorClass::Instance()->GetUpperBodyAnimation(eMap, uiAnimIndex);
    const SoldierAnimation* pLowerAnim;
    if (pState->m_bAnimatedMove)
    {
        pLowerAnim = SoldierAnimatorClass::Instance()->GetLowerBodyAnimation(eMap, uiAnimIndex);
        if (!pLowerAnim)
        {
            pLowerAnim = SoldierAnimatorClass::Instance()->GetLowerBodyMovementAnimation(
                (SoldierAnimType::MovementAnimation)0, (SoldierAnimType::Posture)0, eMap, false);
            bLowerAnim = false;
        }
    }
    else
    {
        if (kLowerMask.IsBitSet(iJoint))
        {
            return false;
        }
        pLowerAnim = SoldierAnimatorClass::Instance()->GetLowerBodyMovementAnimation(
            (SoldierAnimType::MovementAnimation)0, (SoldierAnimType::Posture)0, eMap, false);
        bLowerAnim = false;
    }

    float fTimeStart = m_uiDamageTimeStart * 0.01f;
    float fTimeEnd = m_uiDamageTimeEnd * 0.01f;
    float fDuration = pState->m_uiDuration * 0.01f;
    if (fDuration > 0.0f)
    {
        if (m_bDamageTimeStartFromAnim)
        {
            fTimeEnd = fDuration * fTimeEnd;
        }
        if (fTimeEnd > fDuration)
        {
            fTimeEnd = fDuration;
        }
        SetDamageTimeEnd(fTimeEnd, false);
    }
    else
    {
        fDuration = (pUpperAnim->m_pZephyrAnim->GetNumFrames() - 1) * 0.033333335f;
        if (m_bDamageTimeStartFromAnim)
        {
            fTimeEnd = FLT_MAX;
            SetDamageTimeEnd(1.0f, true);
        }
        else
        {
            if (fTimeEnd > fDuration)
            {
                fTimeEnd = fDuration;
            }
            SetDamageTimeEnd(fTimeEnd, false);
        }
    }
    if (m_bDamageTimeStartFromAnim)
    {
        fTimeStart = fDuration * fTimeStart;
    }
    if (fTimeStart > fDuration)
    {
        fTimeStart = fDuration;
    }
    SetDamageTimeStart(fTimeStart, false);

    if (fTimeStart > (pUpperAnim->m_pZephyrAnim->GetNumFrames() - 1) * 0.033333335f)
    {
        fTimeStart = (pUpperAnim->m_pZephyrAnim->GetNumFrames() - 1) * 0.033333335f;
    }
    if (fTimeEnd > (pUpperAnim->m_pZephyrAnim->GetNumFrames() - 1) * 0.033333335f)
    {
        fTimeEnd = (pUpperAnim->m_pZephyrAnim->GetNumFrames() - 1) * 0.033333335f;
    }

    kUpperPose.SetAnimation(pUpperAnim->m_pZephyrAnim, 30.0f);
    kUpperPose.SetCurTime(0.0f);
    kUpperPose.Update(fTimeStart);
    kUpperPose.SetLoop(false);
    kUpperPose.SetInterpolate(true);
    kLowerPose.SetAnimation(pLowerAnim->m_pZephyrAnim, 30.0f);
    kLowerPose.SetLoop(false);
    kLowerPose.SetCurTime(0.0f);
    kLowerPose.SetInterpolate(bLowerAnim);
    if (bLowerAnim)
    {
        kLowerPose.Update(fTimeStart);
    }

    kPose.Set(kUpperPose, kUpperMask, true);
    kPose.Set(kLowerPose, kLowerMask, true);
    kSkel.BuildWorldMatrices(_IdentityMatrix, iJoint);
    const PblMatrix& mJoint = kSkel.m_pWorldMatrices[iJoint];
    PblVector3 vPos, vDir;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vPos, (const D3DXVECTOR3*)&vBegin, (const D3DXMATRIX*)&mJoint);
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vEnd, (const D3DXMATRIX*)&mJoint);
    AddDamageSample(fTimeStart, vPos, vDir);

    while (fTimeEnd > fTimeStart)
    {
        float fStep = fTimeStep;
        if (fStep + fTimeStart > fTimeEnd)
        {
            fStep = fTimeEnd - fTimeStart;
        }
        kUpperPose.Update(fStep);
        if (bLowerAnim)
        {
            kLowerPose.Update(fStep);
        }
        kPose.Set(kUpperPose, kUpperMask, true);
        kPose.Set(kLowerPose, kLowerMask, true);
        kSkel.BuildWorldMatrices(_IdentityMatrix, iJoint);
        PblVector3 vNewPos, vNewDir;
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vNewPos, (const D3DXVECTOR3*)&vBegin, (const D3DXMATRIX*)&mJoint);
        D3DXVec3TransformNormal((D3DXVECTOR3*)&vNewDir, (const D3DXVECTOR3*)&vEnd, (const D3DXMATRIX*)&mJoint);

        for (;;)
        {
            const float dx = vNewPos.x - vPos.x;
            const float dy = vNewPos.y - vPos.y;
            const float dz = vNewPos.z - vPos.z;
            const float fBaseMoveSq = dx * dx + dy * dy + dz * dz;
            const float tx = (vNewDir.x + vNewPos.x) - vPos.x - vDir.x;
            const float ty = (vNewDir.y + vNewPos.y) - vPos.y - vDir.y;
            const float tz = (vNewDir.z + vNewPos.z) - vPos.z - vDir.z;
            const float fTipMoveSq = tx * tx + ty * ty + tz * tz;
            if (fBaseMoveSq <= fMaxMoveSq && fTipMoveSq <= fMaxMoveSq)
            {
                break;
            }
            const float fMove = (float)sqrt((double)(fBaseMoveSq > fTipMoveSq ? fBaseMoveSq : fTipMoveSq));
            const float fNewStep = fStep * 0.5f * 0.9f / fMove;
            if (fMinTimeStep > fNewStep)
            {
                break;
            }
            fStep = fNewStep;
            kUpperPose.SetCurTime(0.0f);
            kUpperPose.Update(fStep + fTimeStart);
            if (bLowerAnim)
            {
                kLowerPose.SetCurTime(0.0f);
                kLowerPose.Update(fStep + fTimeStart);
            }
            kPose.Set(kUpperPose, kUpperMask, true);
            kPose.Set(kLowerPose, kLowerMask, true);
            kSkel.BuildWorldMatrices(_IdentityMatrix, iJoint);
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vNewPos, (const D3DXVECTOR3*)&vBegin, (const D3DXMATRIX*)&mJoint);
            D3DXVec3TransformNormal((D3DXVECTOR3*)&vNewDir, (const D3DXVECTOR3*)&vEnd, (const D3DXMATRIX*)&mJoint);
        }

        fTimeStart = fStep + fTimeStart;
        AddDamageSample(fTimeStart, vNewPos, vNewDir);
        vPos = vNewPos;
        vDir = vNewDir;
    }
    return true;
}

// FUNCTION: SWBF2 0x00473c20
const Combo::State& Combo::State::operator=(const State& other)
{
    m_uiNameHash = other.m_uiNameHash;
    m_sound = other.m_sound;
    m_aTransition = other.m_aTransition;
    m_aAttack = other.m_aAttack;
    m_pDeflect = other.m_pDeflect;
    m_aAnimatedMoveCondition = other.m_aAnimatedMoveCondition;
    memcpy(m_auiPackedData, other.m_auiPackedData, sizeof(m_auiPackedData));
    m_uiNumAttacks = 0;
    m_aAttack = 0;
    for (unsigned int i = 0; i < other.m_uiNumAttacks; ++i)
    {
        Attack* pAttack = AddAttack();
        if (!pAttack)
        {
            break;
        }
        memcpy(pAttack, other.m_aAttack + i, sizeof(Attack));
    }
    return *this;
}

// FUNCTION: SWBF2 0x00474fd0
Combo* Combo::CopyConstructor(const Combo& other)
{
    m_uiNameHash = other.m_uiNameHash;
    m_eCombo = other.m_eCombo;
    m_eMap = other.m_eMap;
    m_pSkeleton = other.m_pSkeleton;
    memcpy(m_auiOffhandAnimIndex, other.m_auiOffhandAnimIndex, sizeof(m_auiOffhandAnimIndex));
    m_uiThrowAnimIndex = other.m_uiThrowAnimIndex;
    m_uiDeflectButton = other.m_uiDeflectButton;

    bool bDeepCopy = false;
    for (unsigned int i = 0; i < other.m_uiNumStates; ++i)
    {
        const State& state = other.m_aState[i];
        const unsigned int* pWords = reinterpret_cast<const unsigned int*>(&state);
        const unsigned int flags = pWords[10];
        if (state.m_uiDuration == 1023 ||
            ((flags & 0x00400000u) && (flags & 0x00080000u)) ||
            ((flags & 0x00800000u) && (flags & 0x00080000u)) ||
            state.m_uiNumAttacks)
        {
            bDeepCopy = true;
            break;
        }
    }
    if (!bDeepCopy)
    {
        m_aState = other.m_aState;
        m_pCatchState = other.m_pCatchState;
        m_pTakeKneeState = other.m_pTakeKneeState;
        m_uiNumStates = other.m_uiNumStates;
        return this;
    }
    m_uiNumStates = 0;
    m_aState = 0;
    for (unsigned int i = 0; i < other.m_uiNumStates; ++i)
    {
        State* pState = AddState(other.m_aState[i].m_uiNameHash);
        if (!pState)
        {
            break;
        }
        *pState = other.m_aState[i];
    }
    return this;
}

// FUNCTION: SWBF2 0x00475b20
unsigned int __fastcall Combo::GetNameHashForMap(unsigned int uComboHash, SoldierAnimationBank::MAP eMap)
{
    const char* pBankName = SoldierAnimationBank::GetBankName(SoldierAnimationBank::GetMapBank(eMap));
    const char* pWeaponName = SoldierAnimationBank::GetWeaponName(SoldierAnimationBank::GetMapWeapon(eMap));
    PblHash first(uComboHash, "_");
    PblHash second(first, pBankName);
    PblHash third(second, "_");
    return PblHash(third, pWeaponName);
}

// FUNCTION: SWBF2 0x00475a10
unsigned char Combo::GetOffhandAnimIndex(int eState) const
{
    if (eState >= 1 && eState <= 4)
    {
        return m_auiOffhandAnimIndex[eState - 1];
    }
    return (unsigned char)-1;
}

// GLOBAL: SWBF2 0x01eb9310
// SYNTHETIC: SWBF2 0x00402540
// `dynamic initializer for 'Combo::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e10
// `dynamic atexit destructor for 'Combo::sMemoryPool''
MemoryPool Combo::sMemoryPool("Combo", 0x24);
// GLOBAL: SWBF2 0x01eb9360
// SYNTHETIC: SWBF2 0x00402600
// `dynamic initializer for 'Combo::Deflect::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e70
// `dynamic atexit destructor for 'Combo::Deflect::sMemoryPool''
MemoryPool Combo::Deflect::sMemoryPool("Combo::Deflect", 0x24);
// GLOBAL: SWBF2 0x01eb93b0
// SYNTHETIC: SWBF2 0x00402580
// `dynamic initializer for 'Combo::Transition::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e30
// `dynamic atexit destructor for 'Combo::Transition::sMemoryPool''
MemoryPool Combo::Transition::sMemoryPool("Combo::Transition", 0x0c);
// GLOBAL: SWBF2 0x01eb9400
// SYNTHETIC: SWBF2 0x004025a0
// `dynamic initializer for 'Combo::Condition::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e40
// `dynamic atexit destructor for 'Combo::Condition::sMemoryPool''
MemoryPool Combo::Condition::sMemoryPool("Combo::Condition", 0x14);

int Combo::s_ComboTableNumEntries;
// GLOBAL: SWBF2 0x01eb9454
// SYNTHETIC: SWBF2 0x00402520
// `dynamic initializer for 'Combo::s_ComboTable''

PblHashTable<const Combo, 32> Combo::s_ComboTable;

// GLOBAL: SWBF2 0x01eb9558
// SYNTHETIC: SWBF2 0x004025e0
// `dynamic initializer for 'Combo::DamageSample::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e60
// `dynamic atexit destructor for 'Combo::DamageSample::sMemoryPool''
MemoryPool Combo::DamageSample::sMemoryPool("Combo::DamageSample", 0x1c);
// GLOBAL: SWBF2 0x01eb95a8
// SYNTHETIC: SWBF2 0x004025c0
// `dynamic initializer for 'Combo::Attack::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e50
// `dynamic atexit destructor for 'Combo::Attack::sMemoryPool''
MemoryPool Combo::Attack::sMemoryPool("Combo::Attack", 0x10);
// GLOBAL: SWBF2 0x01eb95f8
// SYNTHETIC: SWBF2 0x00402560
// `dynamic initializer for 'Combo::State::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e20
// `dynamic atexit destructor for 'Combo::State::sMemoryPool''
MemoryPool Combo::State::sMemoryPool("Combo::State", 0x34);

const float Combo::Condition::TIME_SCALE_INV = 100.0f;
const float Combo::Condition::TIME_SCALE = 1.0f / Combo::Condition::TIME_SCALE_INV;
const float Combo::Condition::ENERGY_SCALE_INV = 10.0f;
const float Combo::Condition::ENERGY_SCALE = 1.0f / Combo::Condition::ENERGY_SCALE_INV;
const float Combo::Condition::INPUT_MAG_SCALE_INV = 100.0f;
const float Combo::Condition::INPUT_MAG_SCALE = 1.0f / Combo::Condition::INPUT_MAG_SCALE_INV;
const float Combo::Condition::INPUT_DIR_SCALE_RAD = 0.017453292f;
const float Combo::Condition::VELOCITYY_MAG_SCALE_INV = 4.0f;
const float Combo::Condition::VELOCITYY_MAG_SCALE = 1.0f / Combo::Condition::VELOCITYY_MAG_SCALE_INV;
const float Combo::Condition::VELOCITYY_VAL_SCALE_INV = 2.0f;
const float Combo::Condition::VELOCITYY_VAL_SCALE = 1.0f / Combo::Condition::VELOCITYY_VAL_SCALE_INV;
const float Combo::Condition::VELOCITYY_VAL_OFFSET = 16.0f;
const float Combo::Condition::VELOCITYXZ_MAG_SCALE_INV = 100.0f;
const float Combo::Condition::VELOCITYXZ_MAG_SCALE = 1.0f / Combo::Condition::VELOCITYXZ_MAG_SCALE_INV;
const float Combo::Condition::VELOCITYXZ_DIR_SCALE_RAD = 0.017453292f;

const float Combo::State::TIME_SCALE_INV = 100.0f;
const float Combo::State::ENERGY_RATE_SCALE_INV = 5.0f;
const float Combo::State::INPUT_SCALE_INV = 20.0f;
const float Combo::State::VELOCITY_SCALE_INV = 50.0f;
const float Combo::State::GRAVITY_SCALE_INV = 10.0f;
const float Combo::State::VELOCITYY_SCALE_INV = 10.0f;
const float Combo::State::VELOCITYDELTA_SCALE_INV = 10.0f;

const float Combo::Transition::ENERGY_SCALE_INV = 10.0f;
const float Combo::Attack::WIDTH_SCALE_INV = 100.0f;
const float Combo::Attack::PUSH_SCALE_INV = 10.0f;
const float Combo::Attack::TIME_SCALE_INV = 100.0f;
const float Combo::Attack::LENGTH_SCALE_INV = 100.0f;
const float Combo::Deflect::ENERGY_SCALE_INV = 10.0f;

static const float ANIM_FRAME_SPEED = 1.0f / 30.0f;

static const unsigned int SCOPE_ID = 0x504f4353;

// FUNCTION: SWBF2 0x00470910
bool IsNextScopeChunk(const PblConfig& config)
{
    return config.m_pChunk->PeekNextChildId() == SCOPE_ID;
}

// FUNCTION: SWBF2 0x00470960
void Combo::Condition::SetBreak(float fTime)
{
    int iTime = (int)(fTime * TIME_SCALE_INV);
    if (iTime < 0)
    {
        iTime = 0;
    }
    else if (iTime > 511)
    {
        iTime = 511;
    }
    m_uiBreakTime = iTime;
    m_bBreak = 1;
}

// FUNCTION: SWBF2 0x004709a0
void Combo::Condition::SetTimeStart(float fTime, bool bFromEnd)
{
    int iTime = (int)(fTime * TIME_SCALE_INV);
    if (iTime < -512)
    {
        iTime = -512;
    }
    else if (iTime > 511)
    {
        iTime = 511;
    }
    m_iTimeStart = iTime;
    m_bTimeStartFromEnd = bFromEnd;
}

// FUNCTION: SWBF2 0x00470a00
void Combo::Condition::SetTimeEnd(float fTime, bool bFromEnd, bool bSyncIfOvertime)
{
    int iTime = (int)(fTime * TIME_SCALE_INV);
    if (iTime < -512)
    {
        iTime = -512;
    }
    else if (iTime > 511)
    {
        iTime = 511;
    }
    m_iTimeEnd = iTime;
    m_bTimeEndFromEnd = bFromEnd;
    m_bSyncIfOvertime = bSyncIfOvertime;
}

// FUNCTION: SWBF2 0x00470a70
void Combo::Condition::SetSoldierStateTime(float fTime, bool bLess)
{
    int iTime = (int)(fTime * TIME_SCALE_INV);
    if (iTime < 0)
    {
        iTime = 0;
    }
    else if (iTime > 2047)
    {
        iTime = 2047;
    }
    m_uiSoldierStateTimeRef = iTime;
    m_bSoldierStateTimeLess = bLess;
}

// FUNCTION: SWBF2 0x00470ac0
void Combo::Condition::SetEnergy(float fEnergy, bool bLess)
{
    int iEnergy = (int)(fEnergy * ENERGY_SCALE_INV);
    if (iEnergy < 0)
    {
        iEnergy = 0;
    }
    else if (iEnergy > 1023)
    {
        iEnergy = 1023;
    }
    m_uiEnergyRef = iEnergy;
    m_bEnergyLess = bLess;
}

// FUNCTION: SWBF2 0x00470b10
void Combo::Condition::SetInputMag(float fMag, bool bLess)
{
    int iMag = (int)(fMag * INPUT_MAG_SCALE_INV);
    if (iMag < 0)
    {
        iMag = 0;
    }
    else if (iMag > 100)
    {
        iMag = 100;
    }
    m_uiInputMagRef = iMag;
    m_bInputMagLess = bLess;
}

// FUNCTION: SWBF2 0x00470b60
void Combo::Condition::SetInputDir(float fMin, float fMax)
{
    m_iInputDirMin = (((int)fMin + 179) % 360 + 360) % 360 - 179;
    m_iInputDirMax = (((int)fMax + 179) % 360 + 360) % 360 - 179;
}

// FUNCTION: SWBF2 0x00470bd0
void Combo::Condition::SetVelocityY(float fVelocity, bool bSigned, bool bLess)
{
    int iVelocity = (int)(bSigned ? (fVelocity + VELOCITYY_VAL_OFFSET) * VELOCITYY_VAL_SCALE_INV
                                  : fVelocity * VELOCITYY_MAG_SCALE_INV);
    if (iVelocity < 0)
    {
        iVelocity = 0;
    }
    else if (iVelocity > 63)
    {
        iVelocity = 63;
    }
    m_uiVelocityYRef = iVelocity;
    m_bVelocityYSigned = bSigned;
    m_bVelocityYLess = bLess;
}

// FUNCTION: SWBF2 0x00470c50
void Combo::Condition::SetVelocityXZMag(float fMag, bool bLess)
{
    int iMag = (int)(fMag * VELOCITYXZ_MAG_SCALE_INV);
    if (iMag < 0)
    {
        iMag = 0;
    }
    else if (iMag > 2047)
    {
        iMag = 2047;
    }
    m_uiVelocityXZMagRef = iMag;
    m_bVelocityXZMagLess = bLess;
}

// FUNCTION: SWBF2 0x00470ca0
void Combo::Condition::SetVelocityXZDir(float fMin, float fMax)
{
    m_iVelocityXZDirMin = (((int)fMin + 179) % 360 + 360) % 360 - 179;
    m_iVelocityXZDirMax = (((int)fMax + 179) % 360 + 360) % 360 - 179;
}

// FUNCTION: SWBF2 0x00470d10
void Combo::Attack::SetDamageTimeStart(float fTime, bool bFromAnim)
{
    int iTime = (int)(fTime * TIME_SCALE_INV);
    if (iTime < 0)
    {
        iTime = 0;
    }
    else if (iTime > 1023)
    {
        iTime = 1023;
    }
    m_uiDamageTimeStart = iTime;
    m_bDamageTimeStartFromAnim = bFromAnim;
}

// FUNCTION: SWBF2 0x00470d60
void Combo::Attack::SetDamageTimeEnd(float fTime, bool bFromAnim)
{
    int iTime = (int)(fTime * TIME_SCALE_INV);
    if (iTime < 0)
    {
        iTime = 0;
    }
    else if (iTime > 1023)
    {
        iTime = 1023;
    }
    m_uiDamageTimeEnd = iTime;
    m_bDamageTimeEndFromAnim = bFromAnim;
}

// FUNCTION: SWBF2 0x00470db0
void Combo::Attack::SetDamageLength(float fLength, bool bFromEdge)
{
    int iLength = (int)(fLength * LENGTH_SCALE_INV);
    if (iLength < 0)
    {
        iLength = 0;
    }
    else if (iLength > 1023)
    {
        iLength = 1023;
    }
    m_uiDamageLength = iLength;
    m_bDamageLengthFromEdge = bFromEdge;
}

// FUNCTION: SWBF2 0x00470e00
void Combo::Attack::SetDamage(float fDamage)
{
    int iDamage = (int)fDamage;
    if (iDamage < 0)
    {
        iDamage = 0;
    }
    else if (iDamage > 4095)
    {
        iDamage = 4095;
    }
    m_uiDamage = iDamage;
}

// FUNCTION: SWBF2 0x00470e30
void Combo::Deflect::SetDeflectAngle(float fMin, float fMax)
{
    m_iDeflectAngleMin = (((int)fMin + 179) % 360 + 360) % 360 - 179;
    m_iDeflectAngleMax = (((int)fMax + 179) % 360 + 360) % 360 - 179;
}

// FUNCTION: SWBF2 0x00470ec0
void Combo::State::SetDuration(float fDuration)
{
    if (fDuration < 0.0f)
    {
        m_uiDuration = DURATION_INVALID;
        return;
    }
    int iDuration = (int)(fDuration * TIME_SCALE_INV);
    if (iDuration < 0)
    {
        iDuration = 0;
    }
    else if (iDuration > DURATION_MAX)
    {
        iDuration = DURATION_MAX;
    }
    m_uiDuration = iDuration;
}

// FUNCTION: SWBF2 0x00470f10
void Combo::State::SetSound(const char* pName, float fTime)
{
    m_sound.SetID(pName, true, false);
    int iTime = (int)(fTime * TIME_SCALE_INV);
    if (iTime < 0)
    {
        iTime = 0;
    }
    else if (iTime > SOUND_TIME_MAX)
    {
        iTime = SOUND_TIME_MAX;
    }
    m_uiSoundTime = iTime;
}

// FUNCTION: SWBF2 0x00470f70
void Combo::State::SetVelocityZ(float fVelocity, bool bFromAnim)
{
    int iVelocity = (int)(fVelocity * VELOCITY_SCALE_INV);
    if (iVelocity < VELOCITY_MIN)
    {
        iVelocity = VELOCITY_MIN;
    }
    else if (iVelocity > VELOCITY_MAX)
    {
        iVelocity = VELOCITY_MAX;
    }
    m_iVelocityZ = iVelocity;
    m_bVelocityZFromAnim = bFromAnim;
}

// FUNCTION: SWBF2 0x00470fc0
void Combo::State::SetVelocityX(float fVelocity, bool bFromAnim)
{
    int iVelocity = (int)(fVelocity * VELOCITY_SCALE_INV);
    if (iVelocity < VELOCITY_MIN)
    {
        iVelocity = VELOCITY_MIN;
    }
    else if (iVelocity > VELOCITY_MAX)
    {
        iVelocity = VELOCITY_MAX;
    }
    m_iVelocityX = iVelocity;
    m_bVelocityXFromAnim = bFromAnim;
}

// FUNCTION: SWBF2 0x00471010
void Combo::State::SetInputLock(unsigned int uiInputLock, float fDuration)
{
    m_uiInputLock = uiInputLock;
    int iDuration = (int)(fDuration * TIME_SCALE_INV);
    if (iDuration < 0)
    {
        iDuration = 0;
    }
    else if (iDuration > INPUTLOCK_DURATION_MAX)
    {
        iDuration = INPUTLOCK_DURATION_MAX;
    }
    m_uiInputLockDuration = iDuration;
}

// FUNCTION: SWBF2 0x00471060
void Combo::State::SetEnergyRestoreRate(float fRate, bool b)
{
    int iRate = (int)(fRate * ENERGY_RATE_SCALE_INV);
    if (iRate < ENERGY_RATE_MIN)
    {
        iRate = ENERGY_RATE_MIN;
    }
    else if (iRate > ENERGY_RATE_MAX)
    {
        iRate = ENERGY_RATE_MAX;
    }
    m_iEnergyRestoreRate = iRate;
    m_bEnergyRestoreRateOverride = !b;
}

// FUNCTION: SWBF2 0x004710c0
void Combo::State::SetThrustFactor(float fFactor)
{
    int iFactor = (int)(fFactor * INPUT_SCALE_INV);
    if (iFactor < 0)
    {
        iFactor = 0;
    }
    else if (iFactor > INPUT_MAX)
    {
        iFactor = INPUT_MAX;
    }
    m_uiThrustFactor = iFactor;
}

// FUNCTION: SWBF2 0x004710f0
void Combo::State::SetStrafeFactor(float fFactor)
{
    int iFactor = (int)(fFactor * INPUT_SCALE_INV);
    if (iFactor < 0)
    {
        iFactor = 0;
    }
    else if (iFactor > INPUT_MAX)
    {
        iFactor = INPUT_MAX;
    }
    m_uiStrafeFactor = iFactor;
}

// FUNCTION: SWBF2 0x00471120
void Combo::State::SetPitchFactor(float fFactor)
{
    int iFactor = (int)(fFactor * INPUT_SCALE_INV);
    if (iFactor < 0)
    {
        iFactor = 0;
    }
    else if (iFactor > INPUT_MAX)
    {
        iFactor = INPUT_MAX;
    }
    m_uiPitchFactor = iFactor;
}

// FUNCTION: SWBF2 0x00471150
void Combo::State::SetGravity(float fGravity)
{
    int iGravity = (int)(fGravity * GRAVITY_SCALE_INV);
    if (iGravity < GRAVITY_MIN)
    {
        iGravity = GRAVITY_MIN;
    }
    else if (iGravity > GRAVITY_MAX)
    {
        iGravity = GRAVITY_MAX;
    }
    m_iGravity = iGravity;
}

// FUNCTION: SWBF2 0x00471180
void Combo::State::SetGravityVelocityYTarget(float fVelocity, bool bZeroAtTarget)
{
    int iVelocity = (int)(fVelocity * VELOCITYY_SCALE_INV);
    if (iVelocity < VELOCITYY_MIN)
    {
        iVelocity = VELOCITYY_MIN;
    }
    else if (iVelocity > VELOCITYY_MAX)
    {
        iVelocity = VELOCITYY_MAX;
    }
    m_iGravityVelocityYTarget = iVelocity;
    m_bGravityZeroAtTarget = bZeroAtTarget;
}

// FUNCTION: SWBF2 0x004711e0
void Combo::State::SetVelocityYImpulse(float fVelocity, bool bZeroAtTarget)
{
    m_iGravity = GRAVITY_IMPULSE;
    SetGravityVelocityYTarget(fVelocity, bZeroAtTarget);
}

// FUNCTION: SWBF2 0x00471240
bool Combo::Condition::TestSoldierState(int iState, float fTime) const
{
    if (m_uiSoldierStates && !(m_uiSoldierStates & (1 << iState)))
    {
        return false;
    }
    if (m_uiSoldierStateTimeCnd)
    {
        float fRef = m_uiSoldierStateTimeRef * TIME_SCALE;
        if (m_bSoldierStateTimeLess)
        {
            if (fTime > fRef)
            {
                return false;
            }
        }
        else if (fRef > fTime)
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x004712b0
bool Combo::Condition::TestEnergy(float fEnergy) const
{
    if (m_uiEnergyCnd)
    {
        float fRef = m_uiEnergyRef * ENERGY_SCALE;
        if (m_bEnergyLess)
        {
            if (fEnergy > fRef)
            {
                return false;
            }
        }
        else if (fRef > fEnergy)
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00471300
bool Combo::Condition::TestInputThrust(float fThrust, float fStrafe) const
{
    if (m_uiInputMagCnd)
    {
        float fMagSq = fThrust * fThrust + fStrafe * fStrafe;
        float fRef = m_uiInputMagRef * INPUT_MAG_SCALE;
        fRef *= fRef;
        if (m_bInputMagLess)
        {
            if (fMagSq > fRef)
            {
                return false;
            }
        }
        else if (fRef > fMagSq)
        {
            return false;
        }
    }
    int iDirMin = m_iInputDirMin;
    int iDirMax = m_iInputDirMax;
    if (iDirMin != iDirMax)
    {
        float fDir = (float)atan2(fStrafe, fThrust);
        float fDirMin = iDirMin * INPUT_DIR_SCALE_RAD;
        if (iDirMin < iDirMax)
        {
            if (fDirMin > fDir)
            {
                return false;
            }
            if (fDir > iDirMax * INPUT_DIR_SCALE_RAD)
            {
                return false;
            }
        }
        else if (fDirMin >= fDir && fDir >= iDirMax * INPUT_DIR_SCALE_RAD)
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00471400
bool Combo::Condition::TestVelocity(float fX, float fY, float fZ) const
{
    if (m_uiVelocityYCnd)
    {
        float fRef = (float)m_uiVelocityYRef;
        if (m_bVelocityYSigned)
        {
            fRef = fRef * VELOCITYY_VAL_SCALE - VELOCITYY_VAL_OFFSET;
        }
        else
        {
            fY *= fY;
            fRef *= VELOCITYY_MAG_SCALE;
            fRef *= fRef;
        }
        if (m_bVelocityYLess)
        {
            if (fY > fRef)
            {
                return false;
            }
        }
        else if (fRef > fY)
        {
            return false;
        }
    }
    if (m_uiVelocityXZMagCnd)
    {
        float fMagSq = fX * fX + fZ * fZ;
        float fRef = m_uiVelocityXZMagRef * VELOCITYXZ_MAG_SCALE;
        fRef *= fRef;
        if (m_bVelocityXZMagLess)
        {
            if (fMagSq > fRef)
            {
                return false;
            }
        }
        else if (fRef > fMagSq)
        {
            return false;
        }
    }
    int iDirMin = m_iVelocityXZDirMin;
    int iDirMax = m_iVelocityXZDirMax;
    if (iDirMin != iDirMax)
    {
        float fDir = (float)atan2(fX, fZ);
        float fDirMin = iDirMin * VELOCITYXZ_DIR_SCALE_RAD;
        if (iDirMin < iDirMax)
        {
            if (fDirMin > fDir)
            {
                return false;
            }
            if (fDir > iDirMax * VELOCITYXZ_DIR_SCALE_RAD)
            {
                return false;
            }
        }
        else if (fDirMin >= fDir && fDir >= iDirMax * VELOCITYXZ_DIR_SCALE_RAD)
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00471590
void Combo::Condition::_ReadConfig(PblConfig& config, const char* pszComboName, const char* pszStateName)
{
    while (!config.IsDone())
    {
        if (config.IsScopeNext())
        {
            PblConfig scope(config.ReadNextScope(), true);
            continue;
        }

        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3e693899:
            if (data.GetNumArgs())
            {
                float fTime = data.GetFloatArg(0);
                bool bFromEnd = false;
                bool bSyncIfOvertime = false;
                for (unsigned int i = 1; i < data.GetNumArgs(); ++i)
                {
                    if (!data.IsStringArg(i))
                    {
                        continue;
                    }
                    switch (PblHash(data.GetStringArg(i)))
                    {
                    case 0x3a7dcd06:
                        bSyncIfOvertime = true;
                        break;
                    case 0x7b71324f:
                        fTime *= ANIM_FRAME_SPEED;
                        break;
                    case 0xfa9122ba:
                        bFromEnd = true;
                        break;
                    }
                }
                SetTimeEnd(fTime, bFromEnd, bSyncIfOvertime);
            }
            break;

        case 0x2be29e1f:
            if (data.GetNumArgs() >= 2 && data.IsStringArg(0) && data.IsFloatArg(1))
            {
                int iRef = (int)(data.GetFloatArg(1) * VELOCITYY_MAG_SCALE_INV);
                if (iRef < 0)
                {
                    iRef = 0;
                }
                else if (iRef > 63)
                {
                    iRef = 63;
                }
                m_uiVelocityYRef = iRef;
                m_bVelocityYLess = data.GetStringArg(0)[0] == '<';
                m_bVelocityYSigned = false;
            }
            break;

        case 0x29cc5095:
            if (data.GetNumArgs() >= 2 && data.IsStringArg(0) && data.IsFloatArg(1))
            {
                SetEnergy(data.GetFloatArg(1), data.GetStringArg(0)[0] == '<');
            }
            break;

        case 0x43b27471:
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                unsigned int eButton = 0;
                unsigned int eAction = 0;
                switch (PblHash(data.GetStringArg(0)))
                {
                case 0x8eab16d9: eButton = 1; break;
                case 0xa70c98dd: eButton = 2; break;
                case 0xa73f5c0d: eButton = 3; break;
                case 0x76f15155: eButton = 4; break;
                case 0x4e4fa58b: eButton = 5; break;
                case 0xed7cdd8c: eButton = 6; break;
                case 0x986b9eb6: eButton = 7; break;
                }
                if (data.GetNumArgs() > 1 && data.IsStringArg(1))
                {
                    switch (PblHash(data.GetStringArg(1)))
                    {
                    case 0xaaf74aa2: eAction = 1; break;
                    case 0xadc649b8: eAction = 2; break;
                    case 0x389c8f75: eAction = 3; break;
                    case 0x46d056a1: eAction = 4; break;
                    case 0x486aebad: eAction = 5; break;
                    case 0x0c1f28db: eAction = 6; break;
                    case 0x25bef6e8: eAction = 7; break;
                    case 0x43430b20: eAction = 8; break;
                    case 0x1036ae7e: eAction = 9; break;
                    case 0x3db9b915: eAction = 10; break;
                    }
                }
                SetButton(eButton, eAction);
            }
            break;

        case 0xbfd3c98c:
        case 0x802b3931:
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                unsigned int uiButtons = 0;
                for (unsigned int i = 0; i < data.GetNumArgs(); ++i)
                {
                    switch (PblHash(data.GetStringArg(0)))
                    {
                    case 0x8eab16d9: uiButtons |= 0x02; break;
                    case 0x19d9d4a0: uiButtons &= ~0x02; break;
                    case 0xa70c98dd: uiButtons |= 0x04; break;
                    case 0x01850be2: uiButtons &= ~0x04; break;
                    case 0xa73f5c0d: uiButtons |= 0x08; break;
                    case 0x2739df58: uiButtons &= ~0x08; break;
                    case 0x76f15155: uiButtons |= 0x10; break;
                    case 0x6e55a4e0: uiButtons &= ~0x10; break;
                    case 0x4e4fa58b: uiButtons |= 0x20; break;
                    case 0x4f92add2: uiButtons &= ~0x20; break;
                    case 0xed7cdd8c: uiButtons |= 0x40; break;
                    case 0x53a68851: uiButtons &= ~0x40; break;
                    case 0x2c29f04d:
                    case 0x13254bc4: uiButtons |= 0x3f; break;
                    }
                }
                if (data.GetId() == 0xbfd3c98c)
                {
                    SetInputButtonsPressed(uiButtons);
                }
                else
                {
                    SetInputButtonsReleased(uiButtons);
                }
            }
            break;

        case 0x70a3cdf2:
            if (data.GetNumArgs() >= 2 && data.IsFloatArg(0) && data.IsFloatArg(1))
            {
                SetInputDir(data.GetFloatArg(0), data.GetFloatArg(1));
            }
            break;

        case 0x82a306b7:
            if (data.GetNumArgs() >= 2 && data.IsFloatArg(0) && data.IsFloatArg(1))
            {
                SetVelocityXZDir(data.GetFloatArg(0), data.GetFloatArg(1));
            }
            break;

        case 0x8f04cb51:
            if (data.GetNumArgs() >= 2 && data.IsStringArg(0) && data.IsFloatArg(1))
            {
                float fTime = data.GetFloatArg(1);
                if (data.GetNumArgs() >= 3 && data.IsStringArg(2) &&
                    PblHash(data.GetStringArg(2)) == 0x7b71324f)
                {
                    fTime *= ANIM_FRAME_SPEED;
                }
                SetSoldierStateTime(fTime, data.GetStringArg(0)[0] == '<');
            }
            break;

        case 0xb7c88a11:
            if (data.GetNumArgs() >= 2 && data.IsStringArg(0) && data.IsFloatArg(1))
            {
                SetVelocityY(data.GetFloatArg(1), true, data.GetStringArg(0)[0] == '<');
            }
            break;

        case 0xc44b98e8:
            if (data.GetNumArgs())
            {
                float fTime = data.GetFloatArg(0);
                bool bFromEnd = false;
                for (unsigned int i = 1; i < data.GetNumArgs(); ++i)
                {
                    if (!data.IsStringArg(i))
                    {
                        continue;
                    }
                    switch (PblHash(data.GetStringArg(i)))
                    {
                    case 0x7b71324f:
                        fTime *= ANIM_FRAME_SPEED;
                        break;
                    case 0xfa9122ba:
                        bFromEnd = true;
                        break;
                    }
                }
                SetTimeStart(fTime, bFromEnd);
            }
            break;

        case 0xc1ae1e4c:
            if (data.GetNumArgs() >= 2 && data.IsStringArg(0) && data.IsFloatArg(1))
            {
                SetVelocityXZMag(data.GetFloatArg(1), data.GetStringArg(0)[0] == '<');
            }
            break;

        case 0xc9648178:
        {
            float fTime = data.GetNumArgs() ? data.GetFloatArg(0) : 0.0f;
            if (data.GetNumArgs() >= 2 && data.IsStringArg(1) &&
                PblHash(data.GetStringArg(1)) == 0x7b71324f)
            {
                fTime *= ANIM_FRAME_SPEED;
            }
            SetBreak(fTime);
            break;
        }

        case 0xd0624e33:
            if (data.GetNumArgs() >= 2 && data.IsStringArg(0) && data.IsFloatArg(1))
            {
                SetInputMag(data.GetFloatArg(1), data.GetStringArg(0)[0] == '<');
            }
            break;

        case 0xf0f686cb:
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                unsigned int uiStates = 0;
                for (unsigned int i = 0; i < data.GetNumArgs(); ++i)
                {
                    if (!data.IsStringArg(i))
                    {
                        continue;
                    }
                    switch (PblHash(data.GetStringArg(i)))
                    {
                    case 0x75493323: uiStates |= 0x01; break;
                    case 0xb837312c: uiStates &= ~0x01; break;
                    case 0x4e4fa58b: uiStates |= 0x02; break;
                    case 0x4f92add2: uiStates &= ~0x02; break;
                    case 0x76f15155: uiStates |= 0x08; break;
                    case 0x6e55a4e0: uiStates &= ~0x08; break;
                    case 0xa73f5c0d: uiStates |= 0x10; break;
                    case 0x2739df58: uiStates &= ~0x10; break;
                    case 0x16ff3d1e: uiStates |= 0x20; break;
                    case 0xa8bfb92f: uiStates &= ~0x20; break;
                    case 0xd7e330f4: uiStates |= 0x40; break;
                    case 0x7a24b7d3: uiStates &= ~0x40; break;
                    case 0x2c29f04d:
                    case 0x13254bc4: uiStates |= 0x7f; break;
                    }
                }
                SetSoldierStates(uiStates);
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00472000
void Combo::Transition::_ReadConfig(PblConfig& config, const char* pszComboName, const char* pszStateName)
{
    while (!config.IsDone())
    {
        if (config.IsScopeNext())
        {
            PblConfig scope(config.ReadNextScope(), true);
            continue;
        }

        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x39386e06:
        case 0x5d342984:
            if (config.IsScopeNext())
            {
                PblConfig scope(config.ReadNextScope(), true);
                Condition* pCondition = AddCondition();
                if (pCondition)
                {
                    pCondition->_ReadConfig(scope, pszComboName, pszStateName);
                }
            }
            break;

        case 0x495b1788:
            if (data.GetNumArgs())
            {
                SetEnergyCost(data.GetFloatArg(0));
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00472170
Combo::Condition* Combo::Transition::AddCondition()
{
    if (m_uiNumConditions >= 63)
    {
        return 0;
    }
    Condition* pCondition = _AllocCondition();
    if (!pCondition)
    {
        return 0;
    }
    pCondition->_Init();
    if (!m_uiNumConditions)
    {
        m_aCondition = pCondition;
    }
    ++m_uiNumConditions;
    return pCondition;
}

// FUNCTION: SWBF2 0x004721c0
void Combo::Attack::_Init()
{
    m_auiPackedData[0] = 0;
    m_auiPackedData[1] = 0;
    m_auiPackedData[2] = 0;
    m_aDamageSample = 0;
    m_uiDamage = 0xfff;
    m_uiNumDamageSamples = 0;
    m_bDamageTimeStartFromAnim = 0;
    m_bDamageTimeEndFromAnim = 0;
    m_uiDamageTimeStart = 0;
    m_uiDamageTimeEnd = 16;
    m_uiDamageEdge = 0;
    m_bDamageLengthFromEdge = 1;
    m_bDamageWidthFromEdge = 1;
    m_uiDamageLength = 100;
    m_uiDamageWidth = 100;
}

// FUNCTION: SWBF2 0x00472210
void Combo::Attack::_ReadConfig(PblConfig& config, const char* pszStateName)
{
    while (!config.IsDone())
    {
        if (config.IsScopeNext())
        {
            PblConfig scope(config.ReadNextScope(), true);
            continue;
        }

        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x56f6d83c:
            if (data.GetNumArgs())
            {
                SetDamageEdge(data.GetUint32Arg(0));
            }
            break;

        case 0x13fb99f9:
            if (data.GetNumArgs() >= 2 && data.IsFloatArg(0) && data.IsFloatArg(1))
            {
                float fStart = data.GetFloatArg(0);
                float fEnd = data.GetFloatArg(1);
                if (data.GetNumArgs() > 2)
                {
                    if (data.IsStringArg(2))
                    {
                        switch (PblHash(data.GetStringArg(2)))
                        {
                        case 0x65db4ec0:
                            if (fStart > 0.0f)
                            {
                                fStart = -fStart;
                            }
                            if (fStart < -1.0f)
                            {
                                fStart = -1.0f;
                            }
                            if (fEnd > 0.0f)
                            {
                                fEnd = -fEnd;
                            }
                            if (fEnd < -1.0f)
                            {
                                fEnd = -1.0f;
                            }
                            break;
                        case 0x66b6cdea:
                            if (fStart < 0.0f)
                            {
                                fStart = 0.0f;
                            }
                            if (fEnd < 0.0f)
                            {
                                fEnd = 0.0f;
                            }
                            break;
                        case 0x7b71324f:
                            if (fStart < 0.0f)
                            {
                                fStart = 0.0f;
                            }
                            else
                            {
                                fStart *= ANIM_FRAME_SPEED;
                            }
                            if (fEnd < 0.0f)
                            {
                                fEnd = 0.0f;
                            }
                            else
                            {
                                fEnd *= ANIM_FRAME_SPEED;
                            }
                            break;
                        }
                    }
                }
                else
                {
                    if (fStart < -1.0f)
                    {
                        fStart = -1.0f;
                    }
                    if (fEnd < -1.0f)
                    {
                        fEnd = -1.0f;
                    }
                }
                if (fStart < 0.0f || fEnd < 0.0f)
                {
                    SetDamageTimeStart(-fStart, true);
                    SetDamageTimeEnd(-fEnd, true);
                }
                else
                {
                    SetDamageTimeStart(fStart, false);
                    SetDamageTimeEnd(fEnd, false);
                }
            }
            break;

        case 0x59e94c40:
            if (!data.GetNumArgs())
            {
                SetDamage();
            }
            else if (data.IsStringArg(0))
            {
                if (PblHash(data.GetStringArg(0)) == 0x933b5bde)
                {
                    SetDamage();
                }
            }
            else
            {
                int iDamage = (int)data.GetFloatArg(0);
                if (iDamage < 0)
                {
                    iDamage = 0;
                }
                else if (iDamage > 0xfff)
                {
                    iDamage = 0xfff;
                }
                m_uiDamage = iDamage;
            }
            break;

        case 0x876fffdd:
            if (data.GetNumArgs())
            {
                SetPushStrength(data.GetFloatArg(0));
            }
            break;

        case 0xc3b311dc:
        case 0xf081f01b:
            if (data.GetNumArgs())
            {
                float fLength = data.GetFloatArg(0);
                bool bFromEdge = false;
                if (data.GetNumArgs() >= 2 && data.IsStringArg(1) &&
                    PblHash(data.GetStringArg(1)) == 0xdda74cac)
                {
                    bFromEdge = true;
                }
                else if (fLength < 0.0f)
                {
                    fLength = -fLength;
                    bFromEdge = true;
                }
                SetDamageLength(fLength, bFromEdge);
            }
            break;

        case 0xe4b667ac:
            if (data.GetNumArgs())
            {
                float fWidth = data.GetFloatArg(0);
                bool bFromEdge = false;
                if (data.GetNumArgs() >= 2 && data.IsStringArg(1) &&
                    PblHash(data.GetStringArg(1)) == 0xdda74cac)
                {
                    bFromEdge = true;
                }
                else if (fWidth < 0.0f)
                {
                    fWidth = -fWidth;
                    bFromEdge = true;
                }
                SetDamageWidth(fWidth, bFromEdge);
            }
            break;

        case 0xe9869f48:
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                SetAttackId(PblHash(data.GetStringArg(0)));
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x004732e0
void Combo::Deflect::_ReadConfig(PblConfig& config, const Combo& combo, const char* pszStateName)
{
    while (!config.IsDone())
    {
        if (config.IsScopeNext())
        {
            PblConfig scope(config.ReadNextScope(), true);
            continue;
        }

        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x495b1788:
            if (data.GetNumArgs())
            {
                SetEnergyCost(data.GetFloatArg(0));
            }
            break;

        case 0xd9ae70ee:
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                const char* pcName = data.GetStringArg(0);
                const char* pcScope = strrchr(pcName, '_');
                SoldierAnimType::Scope eScope = SoldierAnimType::UPPER;
                char acName[32];
                if (pcScope)
                {
                    if (!_stricmp(pcScope, "_lower"))
                    {
                        eScope = SoldierAnimType::LOWER;
                    }
                    else if (_stricmp(pcScope, "_upper"))
                    {
                        pcScope = 0;
                    }
                    if (pcScope)
                    {
                        unsigned int uiLen = pcScope - pcName;
                        if (uiLen > 31)
                        {
                            uiLen = 31;
                        }
                        strncpy(acName, pcName, uiLen);
                        acName[uiLen] = 0;
                        pcName = acName;
                    }
                }
                unsigned int uiHash = PblHash(pcName);
                if (!pcScope)
                {
                    uiHash = PblHash(uiHash, "_upper");
                }

                SoldierAnimationBank::COMBO_ANIMATION eAnimation;
                SoldierAnimationData* pAnimData = SoldierAnimationData::Find(uiHash);
                if (pAnimData)
                {
                    eAnimation = pAnimData->GetComboAnimation();
                    if (eAnimation == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
                    {
                        pAnimData = 0;
                    }
                    else
                    {
                        SoldierAnimationBank::AddComboAnimation((SoldierAnimationBank::COMBO)combo.m_eCombo,
                                                                eAnimation);
                    }
                }
                else
                {
                    eAnimation = SoldierAnimationBank::AddComboAnimation(
                        (SoldierAnimationBank::COMBO)combo.m_eCombo, pcName);
                    if (eAnimation != SoldierAnimationBank::INVALID_COMBO_ANIMATION)
                    {
                        pAnimData = SoldierAnimationData::Find(eAnimation, eScope);
                    }
                }

                if (pAnimData)
                {
                    unsigned char uiWeapon = 0;
                    unsigned char uiDirections;
                    if (data.GetNumArgs() < 2)
                    {
                        uiDirections = 0x3f;
                    }
                    else
                    {
                        uiDirections = 0;
                        for (unsigned int i = 1; i < data.GetNumArgs(); ++i)
                        {
                            if (!data.IsStringArg(i))
                            {
                                continue;
                            }
                            switch (PblHash(data.GetStringArg(i)))
                            {
                            case 0xb45fa81a: uiDirections |= 0x01; break;
                            case 0x936e62b1: uiDirections &= ~0x01; break;
                            case 0x78e32de5: uiDirections |= 0x02; break;
                            case 0xee304d76: uiDirections &= ~0x02; break;
                            case 0x1c73b020: uiDirections |= 0x04; break;
                            case 0xf6d88fc3: uiDirections &= ~0x04; break;
                            case 0x124aec70: uiDirections |= 0x08; break;
                            case 0x4daf96fd: uiDirections &= ~0x08; break;
                            case 0xdf68e22f: uiDirections |= 0x10; break;
                            case 0x679544ee: uiDirections &= ~0x10; break;
                            case 0x5bb421a2: uiDirections |= 0x20; break;
                            case 0xd191b387: uiDirections &= ~0x20; break;
                            case 0x2c29f04d:
                            case 0x13254bc4: uiDirections |= 0x3f; break;
                            case 0x18fa636f: uiWeapon = 1; break;
                            case 0xaf2a6b67: uiWeapon = 2; break;
                            case 0xae2a69d4: uiWeapon = 3; break;
                            }
                        }
                        if (!uiDirections)
                        {
                            uiDirections = 0x3f;
                        }
                    }
                    AddDeflectAnimation(pAnimData->GetAnimIndex(), uiDirections, uiWeapon);
                }

                if (config.IsScopeNext())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    if (pAnimData && eAnimation != SoldierAnimationBank::INVALID_COMBO_ANIMATION)
                    {
                        if (!pAnimData->m_bReadFromConfig)
                        {
                            pAnimData->ReadConfig(scope, pcName);
                        }
                        if (!pcScope)
                        {
                            SoldierAnimationData* pLower =
                                SoldierAnimationData::Find(eAnimation, SoldierAnimType::LOWER);
                            if (pLower && !pLower->m_bReadFromConfig)
                            {
                                pLower->ConvertScope(*pAnimData);
                            }
                        }
                    }
                }
            }
            else if (config.IsScopeNext())
            {
                config.ReadNextScope();
            }
            break;

        case 0xe46f6ff3:
            if (data.GetNumArgs() >= 2 && data.IsFloatArg(0) && data.IsFloatArg(1))
            {
                SetDeflectAngle(data.GetFloatArg(0), data.GetFloatArg(1));
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x004737d0
unsigned int Combo::Deflect::AddDeflectAnimation(unsigned char uiAnimIndex, unsigned char uiDirections,
                                                 unsigned char uiWeapon)
{

    if (uiAnimIndex >= 164 || m_uiNumDeflectAnimations >= 15)
    {
        return 0;
    }
    DEFLECT_ANIM& anim = m_aDeflectAnim[m_uiNumDeflectAnimations];
    anim.uiAnimIndex = uiAnimIndex;
    anim.uiDirections = uiDirections;
    anim.uiWeapon = uiWeapon;
    return m_uiNumDeflectAnimations++;
}

// FUNCTION: SWBF2 0x00473cd0
void Combo::State::_ReadConfig(PblConfig& config, const Combo& combo, const char* pszComboName)
{
    while (!config.IsDone())
    {
        if (config.IsScopeNext())
        {
            PblConfig scope(config.ReadNextScope(), true);
            continue;
        }

        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x0e0d9594:
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                float fTime = 0.0f;
                if (data.GetNumArgs() >= 2 && data.IsFloatArg(1))
                {
                    fTime = data.GetFloatArg(1);
                    if (data.GetNumArgs() >= 3 && data.IsStringArg(2) &&
                        PblHash(data.GetStringArg(2)) == 0x7b71324f)
                    {
                        fTime *= ANIM_FRAME_SPEED;
                    }
                }
                SetSound(data.GetStringArg(0), fTime);
            }
            break;

        case 0x024630ca:
            if (data.GetNumArgs())
            {
                SetPitchFactor(data.GetFloatArg(0));
            }
            break;

        case 0x0ec601dc:
            if (data.GetNumArgs())
            {
                SetThrustFactor(data.GetFloatArg(0));
            }
            break;

        case 0xadd895e9:
            if (data.GetNumArgs())
            {
                SetStrafeFactor(data.GetFloatArg(0));
            }
            break;

        case 0x6307b6ad:
            if (data.GetNumArgs())
            {
                SetTurnFactor(data.GetFloatArg(0));
            }
            break;

        case 0x11b79e53:
            if (!data.GetNumArgs())
            {
                SetEnergyRestoreRate();
            }
            else
            {
                float fRate = data.GetFloatArg(0);
                bool bFromSoldier = false;
                if (data.GetNumArgs() >= 2 && data.IsStringArg(1) &&
                    PblHash(data.GetStringArg(1)) == 0xa3d53205)
                {
                    bFromSoldier = true;
                }
                SetEnergyRestoreRate(fRate, bFromSoldier);
            }
            break;

        case 0x1baf6514:
        {
            bool bAligned = true;
            bool bInitial = false;
            if (data.GetNumArgs())
            {
                if (data.IsStringArg(0))
                {
                    if (PblHash(data.GetStringArg(0)) == 0x2b251de3)
                    {
                        bInitial = true;
                    }
                }
                else
                {
                    bAligned = data.GetBoolArg(0);
                }
            }
            SetAlignedToThrust(bAligned, bInitial);
            break;
        }

        case 0x2b43d2e9:
        {
            bool bAnimatedMove = data.GetNumArgs() ? data.GetBoolArg(0) : true;
            SetAnimatedMove(bAnimatedMove);
            if (bAnimatedMove)
            {
                SetVelocityX(1.0f, true);
                SetVelocityZ(1.0f, true);
                m_uiVelocityFromThrust = 0;
                m_uiVelocityFromStrafe = 0;
            }
            if (config.m_pChunk->PeekNextChildId() == SCOPE_ID)
            {
                PblConfig scope(config.ReadNextScope(), true);
                if (bAnimatedMove)
                {
                    _ReadAnimatedMoveScope(scope, pszComboName);
                }
            }
            break;
        }

        case 0x2fa0fd0d:
            if (data.GetNumArgs())
            {
                float fDuration = data.GetFloatArg(0);
                if (data.GetNumArgs() > 1)
                {
                    if (data.IsStringArg(1))
                    {
                        switch (PblHash(data.GetStringArg(1)))
                        {
                        case 0x65db4ec0:
                            fDuration = -1.0f;
                            break;
                        case 0x66b6cdea:
                            if (fDuration < 0.0f)
                            {
                                fDuration = 0.0f;
                            }
                            break;
                        case 0x7b71324f:
                            if (fDuration < 0.0f)
                            {
                                fDuration = 0.0f;
                            }
                            else
                            {
                                fDuration *= ANIM_FRAME_SPEED;
                            }
                            break;
                        }
                    }
                }
                else if (fDuration < 0.0f)
                {
                    fDuration = -1.0f;
                }
                SetDuration(fDuration);
            }
            break;

        case 0x4595b8fd:
        {
            Attack* pAttack = AddAttack();
            if (config.IsScopeNext())
            {
                PblConfig scope(config.ReadNextScope(), true);
                if (pAttack)
                {
                    pAttack->_ReadConfig(scope, 0);
                }
            }
            break;
        }

        case 0xa62782e2:
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                Transition* pTransition = AddTransition(PblHash(data.GetStringArg(0)));
                if (config.IsScopeNext())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    if (pTransition)
                    {
                        pTransition->_ReadConfig(scope, pszComboName, 0);
                    }
                }
            }
            else if (config.IsScopeNext())
            {
                config.ReadNextScope();
            }
            break;

        case 0xe1e901c0:
            if (config.IsScopeNext())
            {
                PblConfig scope(config.ReadNextScope(), true);
                Deflect* pDeflect = AddDeflect();
                if (pDeflect)
                {
                    pDeflect->_ReadConfig(scope, combo, 0);
                }
            }
            break;

        case 0x66a4c1b6:
            if (!data.GetNumArgs())
            {
                SetLightsaberOff(true);
            }
            else
            {
                for (unsigned int i = 0; i < data.GetNumArgs(); ++i)
                {
                    if (data.IsFloatArg(i))
                    {
                        SetLightsaberOff(data.GetUint32Arg(i), true);
                    }
                }
            }
            break;

        case 0xa23a54ca:
            SetRestartAnimationOnEnter(true);
            break;

        case 0xd45f29ea:
            SetPlayExplosionOnEnter(true);
            break;

        case 0xd59694f0:
            SetMustShowOneFrame(true);
            break;

        case 0xd209ff45:
            if (data.GetNumArgs())
            {
                SetGravity(data.GetFloatArg(0));
            }
            break;

        case 0xae8dc6b3:
            if (data.GetNumArgs())
            {
                float fVelocity = data.GetFloatArg(0);
                bool bImpulse = false;
                bool bZeroGravity = false;
                for (unsigned int i = 1; i < data.GetNumArgs(); ++i)
                {
                    if (!data.IsStringArg(i))
                    {
                        continue;
                    }
                    switch (PblHash(data.GetStringArg(1)))
                    {
                    case 0x4cbb5054:
                        bImpulse = true;
                        break;
                    case 0xf79d3b9b:
                        bZeroGravity = true;
                        break;
                    }
                }
                if (bImpulse)
                {
                    SetVelocityYImpulse(fVelocity, bZeroGravity);
                }
                else
                {
                    SetGravityVelocityYTarget(fVelocity, bZeroGravity);
                }
            }
            break;

        case 0xe145ee5d:
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                const char* pcName = data.GetStringArg(0);
                const char* pcScope = strrchr(pcName, '_');
                SoldierAnimType::Scope eScope = SoldierAnimType::UPPER;
                char acName[32];
                if (pcScope)
                {
                    if (!_stricmp(pcScope, "_lower"))
                    {
                        eScope = SoldierAnimType::LOWER;
                    }
                    else if (_stricmp(pcScope, "_upper"))
                    {
                        pcScope = 0;
                    }
                    if (pcScope)
                    {
                        unsigned int uiLen = pcScope - pcName;
                        if (uiLen > 31)
                        {
                            uiLen = 31;
                        }
                        strncpy(acName, pcName, uiLen);
                        acName[uiLen] = 0;
                        pcName = acName;
                    }
                }
                unsigned int uiHash = PblHash(pcName);
                if (!pcScope)
                {
                    uiHash = PblHash(uiHash, "_upper");
                }

                SoldierAnimationBank::COMBO_ANIMATION eAnimation;
                SoldierAnimationData* pAnimData = SoldierAnimationData::Find(uiHash);
                if (pAnimData)
                {
                    eAnimation = pAnimData->GetComboAnimation();
                    if (eAnimation == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
                    {
                        if (SoldierAnimType::IsWeaponAnimIndex(pAnimData->GetAnimIndex()))
                        {
                            pAnimData = 0;
                        }
                    }
                    else
                    {
                        SoldierAnimationBank::AddComboAnimation((SoldierAnimationBank::COMBO)combo.m_eCombo,
                                                                eAnimation);
                    }
                }
                else
                {
                    eAnimation = SoldierAnimationBank::AddComboAnimation(
                        (SoldierAnimationBank::COMBO)combo.m_eCombo, pcName);
                    if (eAnimation != SoldierAnimationBank::INVALID_COMBO_ANIMATION)
                    {
                        pAnimData = SoldierAnimationData::Find(eAnimation, eScope);
                    }
                }
                m_uiAnimIndex = pAnimData ? pAnimData->GetAnimIndex() : 0xff;

                if (config.IsScopeNext())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    if (pAnimData && eAnimation != SoldierAnimationBank::INVALID_COMBO_ANIMATION)
                    {
                        if (!pAnimData->m_bReadFromConfig)
                        {
                            pAnimData->ReadConfig(scope, pcName);
                        }
                        if (!pcScope)
                        {
                            SoldierAnimationData* pLower =
                                SoldierAnimationData::Find(eAnimation, SoldierAnimType::LOWER);
                            if (pLower && !pLower->m_bReadFromConfig)
                            {
                                pLower->ConvertScope(*pAnimData);
                            }
                        }
                    }
                }
            }
            else if (config.IsScopeNext())
            {
                config.ReadNextScope();
            }
            break;

        case 0xf0f686cb:
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                unsigned int uiStates = 0;
                for (unsigned int i = 0; i < data.GetNumArgs(); ++i)
                {
                    if (!data.IsStringArg(i))
                    {
                        continue;
                    }
                    switch (PblHash(data.GetStringArg(i)))
                    {
                    case 0x75493323: uiStates |= 0x01; break;
                    case 0xb837312c: uiStates &= ~0x01; break;
                    case 0x4e4fa58b: uiStates |= 0x02; break;
                    case 0x4f92add2: uiStates &= ~0x02; break;
                    case 0x76f15155: uiStates |= 0x08; break;
                    case 0x6e55a4e0: uiStates &= ~0x08; break;
                    case 0xa73f5c0d: uiStates |= 0x10; break;
                    case 0x2739df58: uiStates &= ~0x10; break;
                    case 0x16ff3d1e: uiStates |= 0x20; break;
                    case 0xa8bfb92f: uiStates &= ~0x20; break;
                    case 0xd7e330f4: uiStates |= 0x40; break;
                    case 0x7a24b7d3: uiStates &= ~0x40; break;
                    case 0x2c29f04d:
                    case 0x13254bc4: uiStates |= 0x7f; break;
                    }
                }
                SetSoldierStates(uiStates);
            }
            break;

        case 0xf94f0054:
        {

            unsigned int uiInputLock = 0x7f;
            float fDuration = 0.0f;
            if (data.GetNumArgs())
            {
                unsigned int i = 0;
                if (!data.IsStringArg(0))
                {
                    fDuration = data.GetFloatArg(0);
                    i = 1;
                }
                uiInputLock = 0;
                for (; i < data.GetNumArgs(); ++i)
                {
                    if (!data.IsStringArg(i))
                    {
                        continue;
                    }
                    switch (PblHash(data.GetStringArg(i)))
                    {
                    case 0x7b71324f: fDuration *= ANIM_FRAME_SPEED; break;
                    case 0xd0624e33: uiInputLock |= 0x01; break;
                    case 0xa37fa49e: uiInputLock &= ~0x01; break;
                    case 0x8eab16d9: uiInputLock |= 0x02; break;
                    case 0x19d9d4a0: uiInputLock &= ~0x02; break;
                    case 0xa70c98dd: uiInputLock |= 0x04; break;
                    case 0x01850be2: uiInputLock &= ~0x04; break;
                    case 0xa73f5c0d: uiInputLock |= 0x08; break;
                    case 0x2739df58: uiInputLock &= ~0x08; break;
                    case 0x76f15155: uiInputLock |= 0x10; break;
                    case 0x6e55a4e0: uiInputLock &= ~0x10; break;
                    case 0x4e4fa58b: uiInputLock |= 0x20; break;
                    case 0x4f92add2: uiInputLock &= ~0x20; break;
                    case 0xed7cdd8c: uiInputLock |= 0x40; break;
                    case 0x53a68851: uiInputLock &= ~0x40; break;
                    case 0x2c29f04d:
                    case 0x13254bc4: uiInputLock |= 0x7f; break;
                    }
                }
            }
            SetInputLock(uiInputLock, fDuration);
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x00474a60
void Combo::State::_ReadAnimatedMoveScope(PblConfig& config, const char* pszComboName)
{
    while (!config.IsDone())
    {
        if (config.IsScopeNext())
        {
            PblConfig scope(config.ReadNextScope(), true);
            continue;
        }

        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x1aa5cb38:
            if (data.GetNumArgs())
            {
                SetVelocityFromThrust(data.GetFloatArg(0));
            }
            break;

        case 0x8a5c043d:
            if (data.GetNumArgs())
            {
                SetVelocityFromStrafe(data.GetFloatArg(0));
            }
            break;

        case 0xb4c88558:
            if (data.GetNumArgs())
            {
                float fVelocity = data.GetFloatArg(0);
                bool bFromAnim = false;
                if (data.GetNumArgs() >= 2 && data.IsStringArg(1) &&
                    PblHash(data.GetStringArg(1)) == 0x65db4ec0)
                {
                    bFromAnim = true;
                }
                SetVelocityZ(fVelocity, bFromAnim);
            }
            break;

        case 0xb6c8887e:
            if (data.GetNumArgs())
            {
                float fVelocity = data.GetFloatArg(0);
                bool bFromAnim = false;
                if (data.GetNumArgs() >= 2 && data.IsStringArg(1) &&
                    PblHash(data.GetStringArg(1)) == 0x65db4ec0)
                {
                    bFromAnim = true;
                }
                SetVelocityX(fVelocity, bFromAnim);
            }
            break;

        case 0x39386e06:
        case 0x5d342984:
        case 0xbab5214f:
            if (config.IsScopeNext())
            {
                PblConfig scope(config.ReadNextScope(), true);
                Condition* pCondition = AddAnimatedMoveCondition();
                if (pCondition)
                {
                    pCondition->_ReadConfig(scope, pszComboName, 0);
                    if (!pCondition->IsBreak())
                    {
                        pCondition->SetBreak(true);
                    }
                }
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00474d80
void Combo::State::_Init(unsigned int uiNameHash)
{
    m_uiNameHash = uiNameHash;
    m_aTransition = 0;
    m_aAttack = 0;
    m_pDeflect = 0;
    m_aAnimatedMoveCondition = 0;
    for (int i = 0; i < 6; ++i)
    {
        m_auiPackedData[i] = 0;
    }
    m_iGravity = 10;
    m_iGravityVelocityYTarget = -2048;
    m_bRestartAnimation = 0;
    m_bPlayExplosion = 0;
    m_uiAnimIndex = 0xff;
    m_uiThrustFactor = 20;
    m_uiStrafeFactor = 20;
    m_uiTurnFactor = 20;
    m_uiPitchFactor = 20;
    m_uiDuration = DURATION_INVALID;
    m_bMustShowOneFrame = 0;
}

// FUNCTION: SWBF2 0x00474df0
Combo::Transition* Combo::State::AddTransition(unsigned int uiTargetState)
{
    for (unsigned int i = 0; i < m_uiNumTransitions; ++i)
    {
        if (m_aTransition[i].m_uiTargetState == uiTargetState)
        {
            return &m_aTransition[i];
        }
    }
    if (m_uiNumTransitions >= 63)
    {
        return 0;
    }
    Transition* pTransition = _AllocTransition();
    if (!pTransition)
    {
        return 0;
    }
    pTransition->_Init(uiTargetState);
    if (!m_uiNumTransitions)
    {
        m_aTransition = pTransition;
    }
    ++m_uiNumTransitions;
    return pTransition;
}

// FUNCTION: SWBF2 0x00474e90
Combo::Attack* Combo::State::AddAttack()
{
    if (m_uiNumAttacks >= 15)
    {
        return 0;
    }
    Attack* pAttack = _AllocAttack();
    if (!pAttack)
    {
        return 0;
    }
    pAttack->_Init();
    if (!m_uiNumAttacks)
    {
        m_aAttack = pAttack;
    }
    ++m_uiNumAttacks;
    return pAttack;
}

// FUNCTION: SWBF2 0x00474ee0
Combo::Deflect* Combo::State::AddDeflect()
{
    if (m_pDeflect)
    {
        return 0;
    }
    Deflect* pDeflect = _AllocDeflect();
    if (!pDeflect)
    {
        return 0;
    }
    pDeflect->_Init();
    m_pDeflect = pDeflect;
    return pDeflect;
}

// FUNCTION: SWBF2 0x00474f10
Combo::Condition* Combo::State::AddAnimatedMoveCondition()
{
    if (m_uiNumAnimatedMoveConditions >= 63)
    {
        return 0;
    }
    Condition* pCondition = _AllocCondition();
    if (!pCondition)
    {
        return 0;
    }
    pCondition->_Init();
    pCondition->SetBreak(true);
    if (!m_uiNumAnimatedMoveConditions)
    {
        m_aAnimatedMoveCondition = pCondition;
    }
    ++m_uiNumAnimatedMoveConditions;
    return pCondition;
}

// FUNCTION: SWBF2 0x00474f80
Combo::Combo()
{
    m_uiNumStates = 0;
    m_uiNameHash = 0;
    m_eCombo = -1;
    m_eMap = -1;
    m_aState = 0;
    m_pCatchState = 0;
    m_pTakeKneeState = 0;
    m_auiOffhandAnimIndex[0] = 0xff;
    m_auiOffhandAnimIndex[1] = 0xff;
    m_auiOffhandAnimIndex[2] = 0xff;
    m_auiOffhandAnimIndex[3] = 0xff;
    m_uiThrowAnimIndex = 0xff;
    m_uiDeflectButton = 0;
}

// FUNCTION: SWBF2 0x00475100
void Combo::ReadConfig(PblConfig& config)
{
    m_uiNameHash = config.GetNameHash();
    m_eCombo = SoldierAnimationBank::AddCombo(m_uiNameHash);
    m_eMap = -1;
    m_pSkeleton = 0;

    State* pIdle = AddState(0xc301cf93);
    if (!pIdle)
    {
        return;
    }
    pIdle->SetEnergyRestoreRate();

    while (!config.IsDone())
    {
        if (config.IsScopeNext())
        {
            PblConfig scope(config.ReadNextScope(), true);
            continue;
        }

        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x783132f6:
            if (config.IsScopeNext())
            {
                if (data.GetNumArgs() && data.IsStringArg(0))
                {
                    unsigned int uiStateHash = PblHash(data.GetStringArg(0));
                    PblConfig scope(config.ReadNextScope(), true);
                    State* pState = AddState(uiStateHash);
                    if (pState)
                    {
                        pState->_ReadConfig(scope, *this, 0);
                    }
                }
                else
                {
                    config.ReadNextScope();
                }
            }
            break;

        case 0xe145ee5d:
        case 0x27d0b9e7:
        case 0x5dcbeca7:
        {
            bool bOffhand = data.GetId() == 0x27d0b9e7;
            bool bThrow = data.GetId() == 0x5dcbeca7;
            if (data.GetNumArgs() && data.IsStringArg(0))
            {
                const char* pcName = data.GetStringArg(0);
                const char* pcScope = strrchr(pcName, '_');
                SoldierAnimType::Scope eScope = SoldierAnimType::UPPER;
                char acName[32];
                if (pcScope)
                {
                    if (!_stricmp(pcScope, "_lower"))
                    {
                        eScope = SoldierAnimType::LOWER;
                    }
                    else if (_stricmp(pcScope, "_upper"))
                    {
                        pcScope = 0;
                    }
                    if (pcScope)
                    {
                        unsigned int uiLen = pcScope - pcName;
                        if (uiLen > 31)
                        {
                            uiLen = 31;
                        }
                        strncpy(acName, pcName, uiLen);
                        acName[uiLen] = 0;
                        pcName = acName;
                    }
                }
                unsigned int uiHash = PblHash(pcName);
                if (!pcScope)
                {
                    uiHash = PblHash(uiHash, "_upper");
                }

                SoldierAnimationBank::COMBO_ANIMATION eAnimation;
                SoldierAnimationData* pAnimData = SoldierAnimationData::Find(uiHash);
                if (pAnimData)
                {
                    eAnimation = pAnimData->GetComboAnimation();
                    if (eAnimation == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
                    {
                        break;
                    }
                    unsigned int uiIndex = SoldierAnimationBank::AddComboAnimation(
                        (SoldierAnimationBank::COMBO)m_eCombo, eAnimation);
                    if (uiIndex >= SoldierAnimationBank::GetComboNumAnimations((SoldierAnimationBank::COMBO)m_eCombo))
                    {
                        break;
                    }
                }
                else
                {
                    eAnimation = SoldierAnimationBank::AddComboAnimation(
                        (SoldierAnimationBank::COMBO)m_eCombo, pcName);
                    if (eAnimation != SoldierAnimationBank::INVALID_COMBO_ANIMATION)
                    {
                        pAnimData = SoldierAnimationData::Find(eAnimation, eScope);
                    }
                }

                if (pAnimData)
                {
                    if (bOffhand)
                    {

                        int eState = 1;
                        if (data.GetNumArgs() >= 2 && data.IsStringArg(1))
                        {
                            switch (PblHash(data.GetStringArg(1)))
                            {
                            case 0x825513f1: eState = 2; break;
                            case 0xc0ca2371: eState = 3; break;
                            case 0xed7cdd8c: eState = 4; break;
                            }
                        }
                        SetOffhandAnimIndex(eState, pAnimData->GetAnimIndex());
                    }
                    else if (bThrow)
                    {
                        SetThrowAnimIndex(pAnimData->GetAnimIndex());
                    }
                }

                if (config.IsScopeNext())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    if (pAnimData && eAnimation != SoldierAnimationBank::INVALID_COMBO_ANIMATION)
                    {
                        if (!pAnimData->m_bReadFromConfig)
                        {
                            pAnimData->ReadConfig(scope, pcName);
                        }
                        if (!pcScope)
                        {
                            SoldierAnimationData* pLower =
                                SoldierAnimationData::Find(eAnimation, SoldierAnimType::LOWER);
                            if (pLower && !pLower->m_bReadFromConfig)
                            {
                                pLower->ConvertScope(*pAnimData);
                            }
                        }
                    }
                }
            }
            else if (config.IsScopeNext())
            {
                config.ReadNextScope();
            }
            break;
        }
        }
    }

    for (unsigned int i = 0; i < m_uiNumStates; ++i)
    {
        for (unsigned int j = 0; j < m_aState[i].m_uiNumTransitions; ++j)
        {
            Transition& transition = m_aState[i].m_aTransition[j];
            unsigned int k;
            for (k = 0; k < m_uiNumStates; ++k)
            {
                if (m_aState[k].m_uiNameHash == transition.m_uiTargetState && k != i)
                {
                    transition.m_uiTargetState = k;
                    break;
                }
            }
            if (k >= m_uiNumStates)
            {
                transition.m_uiTargetState = 0;
            }
        }
    }

    if (s_ComboTable.Store(m_uiNameHash, this))
    {
        ++s_ComboTableNumEntries;
    }
}

// FUNCTION: SWBF2 0x00475980
void Combo::_PostLoad()
{
    m_uiDeflectButton = _GetDeflectButton();
}

// FUNCTION: SWBF2 0x00475990
unsigned int Combo::_GetDeflectButton() const
{
    unsigned int uiState;
    for (uiState = 0; uiState < m_uiNumStates; ++uiState)
    {
        if (m_aState[uiState].m_pDeflect)
        {
            break;
        }
    }
    if (uiState >= m_uiNumStates || !uiState)
    {
        return 0;
    }

    const State& idle = m_aState[0];
    if (!idle.m_uiNumTransitions)
    {
        return 0;
    }
    const Transition* pTransition = 0;
    for (unsigned int i = 0; i < idle.m_uiNumTransitions; ++i)
    {
        pTransition = &idle.m_aTransition[i];
        if (pTransition->m_uiTargetState == uiState)
        {
            break;
        }
    }
    for (unsigned int i = 0; i < pTransition->m_uiNumConditions; ++i)
    {
        if (pTransition->m_aCondition[i].m_uiButton)
        {
            return pTransition->m_aCondition[i].m_uiButton;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00475a60
void Combo::SetOffhandAnimIndex(int eState, unsigned char uiAnimIndex)
{
    switch (eState)
    {
    case 1:
        m_auiOffhandAnimIndex[0] = uiAnimIndex;
    case 2:
        m_auiOffhandAnimIndex[1] = uiAnimIndex;
    case 3:
        m_auiOffhandAnimIndex[2] = uiAnimIndex;
    case 4:
        m_auiOffhandAnimIndex[3] = uiAnimIndex;
        break;
    }
}

// FUNCTION: SWBF2 0x00475aa0
Combo::State* Combo::AddState(unsigned int uiNameHash)
{
    for (unsigned int i = 0; i < m_uiNumStates; ++i)
    {
        if (m_aState[i].m_uiNameHash == uiNameHash)
        {
            return &m_aState[i];
        }
    }
    State* pState = _AllocState();
    if (!pState)
    {
        return 0;
    }
    pState->_Init(uiNameHash);
    if (uiNameHash == 0x8718d358)
    {
        m_pCatchState = pState;
    }
    else if (uiNameHash == 0xd7e2ce48)
    {
        m_pTakeKneeState = pState;
    }
    if (!m_uiNumStates)
    {
        m_aState = pState;
    }
    ++m_uiNumStates;
    return pState;
}

// FUNCTION: SWBF2 0x00475b80
void Combo::Init()
{
    s_ComboTable.Clear();
    s_ComboTableNumEntries = 0;

    sMemoryPool.Setup(0x20, sMemoryPool.mSize, sMemoryPool.mGrow);
    State::sMemoryPool.Setup(0x64, State::sMemoryPool.mSize, State::sMemoryPool.mGrow);
    Transition::sMemoryPool.Setup(0x55, Transition::sMemoryPool.mSize,
                                  Transition::sMemoryPool.mGrow);
    Condition::sMemoryPool.Setup(0x46, Condition::sMemoryPool.mSize,
                                 Condition::sMemoryPool.mGrow);
    Attack::sMemoryPool.Setup(0x78, Attack::sMemoryPool.mSize, Attack::sMemoryPool.mGrow);
    DamageSample::sMemoryPool.Setup(0x4b0, DamageSample::sMemoryPool.mSize,
                                    DamageSample::sMemoryPool.mGrow);
    Deflect::sMemoryPool.Setup(8, Deflect::sMemoryPool.mSize, Deflect::sMemoryPool.mGrow);
}

// FUNCTION: SWBF2 0x00475c50
void Combo::PostLoad()
{
    for (PblHashTable<const Combo, 32>::Itor itor = s_ComboTable.Begin();
         (const Combo*)itor != (const Combo*)s_ComboTable.End(); ++itor)
    {
        const_cast<Combo*>((const Combo*)itor)->_PostLoad();
    }
}

// FUNCTION: SWBF2 0x00475cc0
Combo::State* Combo::_AllocState()
{
    if (State::sMemoryPool.mUsed >= State::sMemoryPool.GetCount())
    {
        return 0;
    }
    return new State;
}

// FUNCTION: SWBF2 0x00475d00
Combo::Transition* Combo::_AllocTransition()
{
    if (Transition::sMemoryPool.mUsed >= Transition::sMemoryPool.GetCount())
    {
        return 0;
    }
    return new Transition;
}

// FUNCTION: SWBF2 0x00475d20
Combo::Condition* Combo::_AllocCondition()
{
    if (Condition::sMemoryPool.mUsed >= Condition::sMemoryPool.GetCount())
    {
        return 0;
    }
    return new Condition;
}

// FUNCTION: SWBF2 0x00475d40
Combo::Attack* Combo::_AllocAttack()
{
    if (Attack::sMemoryPool.mUsed >= Attack::sMemoryPool.GetCount())
    {
        return 0;
    }
    return new Attack;
}

// FUNCTION: SWBF2 0x00475d80
Combo::Deflect* Combo::_AllocDeflect()
{
    if (Deflect::sMemoryPool.mUsed >= Deflect::sMemoryPool.GetCount())
    {
        return 0;
    }
    return new Deflect;
}
