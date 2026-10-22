#include <stdlib.h>
#include <string.h>

#include "SoldierAnimationData.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Memory/RedMemory.h"

using namespace SoldierAnimType;

const float SoldierAnimationData::BLEND_TIME_SCALE = 100.0f;
const float SoldierAnimationData::BLEND_TIME_SCALE_INV = 0.01f;
const float SoldierAnimationData::BLEND_TIME_MAX = 255.0f;
const int SoldierAnimationData::NUM_ANIM_DATA = 328;

static const float ANIM_FRAME_SPEED = 1.0f / 30.0f;

// GLOBAL: SWBF2 0x01eaf4dc
SoldierAnimationData* SoldierAnimationData::s_aAnimationData;
// GLOBAL: SWBF2 0x01eafb10
SoldierAnimationData::BlendOverrideT* SoldierAnimationData::s_aAnimationOverrideData;
// GLOBAL: SWBF2 0x01eaf648
unsigned int SoldierAnimationData::s_uiNumOverrides;
// GLOBAL: SWBF2 0x01eafb14
SoldierAnimationData::NameTable* SoldierAnimationData::s_pAnimationNameTable;
// GLOBAL: SWBF2 0x01eafb18
unsigned int* SoldierAnimationData::s_auiSyncTypeTable;
// GLOBAL: SWBF2 0x01eaf64c
char** SoldierAnimationData::s_aszSyncTypeNames;

// FUNCTION: SWBF2 0x0063ce00
void SoldierAnimationData::ConvertScope(const SoldierAnimationData& data)
{
    *this = data;
    m_uiScope = !m_uiScope;
    if (m_uiScope)
    {
        m_uiAimType = AIM_NONE;
    }
}

// FUNCTION: SWBF2 0x0063ce40
SoldierAnimationBank::COMBO_ANIMATION SoldierAnimationData::GetComboAnimation() const
{
    if (!IsWeaponMeleeAnimIndex((unsigned char)m_uiAnimIndex))
    {
        return SoldierAnimationBank::INVALID_COMBO_ANIMATION;
    }
    return (SoldierAnimationBank::COMBO_ANIMATION)GetAnimFromAnimIndex((unsigned char)m_uiAnimIndex);
}

// FUNCTION: SWBF2 0x0063ce60
float SoldierAnimationData::GetBlendTime(float fTime) const
{
    float fBlendOut = GetBlendOutTime();
    return (fTime > fBlendOut) ? fBlendOut : fTime;
}

// FUNCTION: SWBF2 0x0063ce90
float SoldierAnimationData::GetBlendTime(float fTime, unsigned char uiAnimIndex) const
{
    for (unsigned short i = 0; i < m_uiNumBlendOverrides; ++i)
    {
        if (m_aBlendOverride[i].uiAnimIndex == uiAnimIndex)
        {
            return m_aBlendOverride[i].GetBlendTime();
        }
    }
    return GetBlendTime(fTime);
}

// FUNCTION: SWBF2 0x0063cf10
void SoldierAnimationData::AddBlendOverride(unsigned char uiAnimIndex, float fTime)
{
    for (unsigned short i = 0; i < m_uiNumBlendOverrides; ++i)
    {
        if (m_aBlendOverride[i].uiAnimIndex == uiAnimIndex)
        {
            m_aBlendOverride[i].SetBlendTime(fTime);
            return;
        }
    }

    if (m_uiNumBlendOverrides < 0xff && s_uiNumOverrides < 128)
    {
        if (!m_aBlendOverride)
        {
            m_aBlendOverride = &s_aAnimationOverrideData[s_uiNumOverrides];
        }
        m_aBlendOverride[m_uiNumBlendOverrides].uiAnimIndex = uiAnimIndex;
        m_aBlendOverride[m_uiNumBlendOverrides].SetBlendTime(fTime);
        ++m_uiNumBlendOverrides;
        ++s_uiNumOverrides;
    }
}

// FUNCTION: SWBF2 0x0063cff0
void SoldierAnimationData::ReadConfig(PblConfig& config, const char* pcName)
{
    m_bReadFromConfig = true;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x514e7f54:
            if (data.GetNumArgs() >= 1 && data.IsFloatArg(0))
            {
                float fFrame = data.GetFloatArg(0);
                if (data.GetNumArgs() >= 2 && data.IsStringArg(1))
                {
                    switch (PblHash(data.GetStringArg(1)))
                    {
                    case 0x66b6cdea:
                        fFrame /= ANIM_FRAME_SPEED;
                        break;
                    case 0x7b71324f:
                        break;
                    default:
                        fFrame /= ANIM_FRAME_SPEED;
                        break;
                    }
                }
                else
                {
                    fFrame /= ANIM_FRAME_SPEED;
                }
                m_uiLowResPoseFrame = (int)(fFrame + 0.5f) + 1;
            }
            break;

        case 0x3d31a706:
            if (data.IsStringArg(0))
            {
                switch (PblHash(data.GetStringArg(0)))
                {
                case 0xada7afdb:
                    SetAimType(AIM_NONE);
                    break;
                case 0x32694bc3:
                    SetAimType(AIM_HEAD);
                    break;
                case 0x8f5b73c4:
                    SetAimType(AIM_TORSO);
                    break;
                case 0x8d3fa718:
                    SetAimType(AIM_FULLBODY);
                    break;
                }
            }
            break;

        case 0x1739d050:
            if (data.GetNumArgs() >= 2 && data.IsStringArg(0) && data.IsFloatArg(1))
            {
                const char* pcTarget = data.GetStringArg(0);
                float fTime = data.GetFloatArg(1);

                if (data.GetNumArgs() >= 3 && data.IsStringArg(1) &&
                    PblHash(data.GetStringArg(2)) == 0x7b71324f)
                {
                    fTime *= ANIM_FRAME_SPEED;
                }

                const char* pcScope = strrchr(pcTarget, '_');
                if (pcScope && _stricmp(pcScope, GetScopeExtension(UPPER)) &&
                    _stricmp(pcScope, GetScopeExtension(LOWER)))
                {
                    pcScope = 0;
                }

                unsigned int uiHash = PblHash(pcTarget);
                if (!pcScope)
                {
                    uiHash = PblHash(uiHash, GetScopeExtension(UPPER));
                }

                SoldierAnimationData* pTarget = Find(uiHash);
                if (pTarget)
                {
                    AddBlendOverride(pTarget->GetAnimIndex(), fTime);
                }
            }
            break;

        case 0x8d1a7a7d:
            if (data.IsFloatArg(0))
            {
                float fTime = data.GetFloatArg(0);

                if (data.GetNumArgs() >= 2 && data.IsStringArg(1) &&
                    PblHash(data.GetStringArg(2)) == 0x7b71324f)
                {
                    fTime *= ANIM_FRAME_SPEED;
                }
                SetBlendOutTime(fTime);
            }
            break;

        case 0x8da5a75a:
            if (data.IsStringArg(0))
            {
                const char* pcSync = data.GetStringArg(0);
                bool bByTime = false;
                if (data.GetNumArgs() >= 2 && data.IsStringArg(1))
                {
                    switch (PblHash(data.GetStringArg(1)))
                    {
                    case 0x4dcb9735:
                        bByTime = true;
                        break;
                    case 0xfc82911d:
                        break;
                    }
                }
                m_uiSyncType = AddSyncType(pcSync, bByTime);
            }
            break;

        case 0x999ec9a6:
            if (data.IsFloatArg(0))
            {
                float fTime = data.GetFloatArg(0);

                if (data.GetNumArgs() >= 2 && data.IsStringArg(1) &&
                    PblHash(data.GetStringArg(2)) == 0x7b71324f)
                {
                    fTime *= ANIM_FRAME_SPEED;
                }
                SetBlendInTime(fTime);
            }
            break;

        case 0xddef486b:
            if (data.GetNumArgs() == 0)
            {
                SetLoopType(LOOP_RESTART);
            }
            else if (data.IsStringArg(0))
            {
                switch (PblHash(data.GetStringArg(0)))
                {
                case 0x1c36d31c:
                    SetLoopType(LOOP_FINAL_FRAME);
                    break;
                case 0x6a8e75aa:
                    SetLoopType(LOOP_END);
                    break;
                case 0xfe9c11ec:
                    SetLoopType(LOOP_RESTART);
                    break;
                }
            }
            else
            {
                unsigned int uiLoop = (int)data.GetFloatArg(0);
                if (uiLoop < NUM_LOOP_TYPES)
                {
                    SetLoopType((LOOP_TYPE)uiLoop);
                }
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0063d580
void SoldierAnimationData::InitLoadTimeData()
{
    if (s_pAnimationNameTable)
    {
        return;
    }

    int iOldHeap = RedSetCurrentHeap(__RedTempHeap);
    s_pAnimationNameTable = new NameTable;
    s_auiSyncTypeTable = new unsigned int[256];
    s_aszSyncTypeNames = new char*[256];
    RedSetCurrentHeap(iOldHeap);

    memset(s_auiSyncTypeTable, 0, 256 * sizeof(unsigned int));
    memset(s_aszSyncTypeNames, 0, 256 * sizeof(char*));

    for (int iAnim = 0; iAnim < NUM_ACTION_ANIMATIONS; ++iAnim)
    {
        unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex((ActionAnimation)iAnim);
        for (int iScope = 0; iScope < NUM_SCOPES; ++iScope)
        {
            s_pAnimationNameTable->Store(PblHash(AnimIndexToName(uiAnimIndex, (Scope)iScope)),
                                         (void*)((uiAnimIndex * 2 + iScope) | 0x80000000));
        }
    }
    for (int iPosture = 0; iPosture < NUM_POSTURES; ++iPosture)
    {
        for (int iAnim = 0; iAnim < NUM_MOVEMENT_ANIMATIONS; ++iAnim)
        {
            unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex((Posture)iPosture, (MovementAnimation)iAnim);
            for (int iScope = 0; iScope < NUM_SCOPES; ++iScope)
            {
                s_pAnimationNameTable->Store(PblHash(AnimIndexToName(uiAnimIndex, (Scope)iScope)),
                                             (void*)((uiAnimIndex * 2 + iScope) | 0x80000000));
            }
        }
    }
    for (int iPosture = 0; iPosture < NUM_STANDARD_POSTURES; ++iPosture)
    {
        for (int iAnim = 0; iAnim < NUM_WEAPON_ANIMATIONS; ++iAnim)
        {
            unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex((Posture)iPosture, (WeaponAnimation)iAnim);
            for (int iScope = 0; iScope < NUM_SCOPES; ++iScope)
            {
                s_pAnimationNameTable->Store(PblHash(AnimIndexToName(uiAnimIndex, (Scope)iScope)),
                                             (void*)((uiAnimIndex * 2 + iScope) | 0x80000000));
            }
        }
    }
}

// FUNCTION: SWBF2 0x0063d7d0
void SoldierAnimationData::InitLoadTimeDataPostLoad()
{
    InitLoadTimeData();

    for (int iAnim = 0; iAnim < SoldierAnimationBank::GetNumComboAnimations(); ++iAnim)
    {
        unsigned char uiAnimIndex = SoldierAnimType::GetAnimIndex((SoldierAnimationBank::COMBO_ANIMATION)iAnim);
        for (int iScope = 0; iScope < NUM_SCOPES; ++iScope)
        {
            const char* pcName = AnimIndexToName(uiAnimIndex, (Scope)iScope);
            if (!s_pAnimationNameTable->Find(PblHash(pcName)))
            {
                s_pAnimationNameTable->Store(PblHash(pcName),
                                             (void*)((uiAnimIndex * 2 + iScope) | 0x80000000));
            }
        }
    }
}

// FUNCTION: SWBF2 0x0063d890
void SoldierAnimationData::DiscardLoadTimeData()
{
    int iOldHeap = RedSetCurrentHeap(__RedTempHeap);

    delete s_pAnimationNameTable;
    s_pAnimationNameTable = 0;
    delete[] s_auiSyncTypeTable;
    s_auiSyncTypeTable = 0;

    if (s_aszSyncTypeNames)
    {
        for (int i = 0; i < 256; ++i)
        {
            delete[] s_aszSyncTypeNames[i];
        }
        delete[] s_aszSyncTypeNames;
        s_aszSyncTypeNames = 0;
    }

    RedSetCurrentHeap(iOldHeap);
}

// FUNCTION: SWBF2 0x0063d920
void SoldierAnimationData::InitAnimationData(bool bDefaults)
{
    if (bDefaults)
    {

        float fBlendInTime = 0.0f;
        float fBlendOutTime = 0.0f;
        for (int iAnim = 0; iAnim < SoldierAnimationBank::GetNumComboAnimations(); ++iAnim)
        {
            switch (SoldierAnimationBank::GetComboAnimationNameHash((SoldierAnimationBank::COMBO_ANIMATION)iAnim))
            {
            case 0x2509cd06:
                fBlendInTime = 0.15f;
                break;
            case 0x1e74e488:
            case 0x2309c9e0:
            case 0xb97d9099:
                fBlendOutTime = 0.15f;
                break;
            case 0x2409cb73:
                break;
            default:
                continue;
            }

            SoldierAnimationData* pUpper = Find((SoldierAnimationBank::COMBO_ANIMATION)iAnim, UPPER);
            if (!pUpper->m_bReadFromConfig)
            {
                pUpper->SetAimType(AIM_TORSO);
                pUpper->SetBlendInTime(fBlendInTime);
                pUpper->SetBlendOutTime(fBlendOutTime);
            }
            SoldierAnimationData* pLower = Find((SoldierAnimationBank::COMBO_ANIMATION)iAnim, LOWER);
            if (!pLower->m_bReadFromConfig)
            {
                pLower->SetBlendInTime(fBlendInTime);
                pLower->SetBlendOutTime(fBlendOutTime);
            }
        }
    }

    if (s_aAnimationData)
    {
        return;
    }

    struct AnimationSetting
    {
        bool bLoop;
        float fBlendInTime;
        float fBlendOutTime;
    };
    AnimationSetting aActionSettings[NUM_ACTION_ANIMATIONS] = {
        { true, 0.15f, 0.15f },
        { false, 0.15f, 0.8f },
        { false, 0.15f, 0.8f },
        { false, 0.15f, 0.8f },
        { false, 0.15f, 0.8f },
        { false, 0.15f, 0.8f },
        { false, 0.05f, 0.15f },
        { false, 0.05f, 0.15f },
        { false, 0.8f, 0.8f },
        { true, 0.5f, 0.5f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { true, 0.1f, 0.1f },
        { true, 0.1f, 0.1f },
        { false, 0.1f, 0.1f },
        { false, 0.1f, 0.1f },
        { false, 0.4f, 0.4f },
        { false, 0.4f, 0.4f },
        { false, 0.15f, 0.2f },
        { false, 0.15f, 0.2f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { true, 0.15f, 0.25f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
        { false, 0.15f, 0.15f },
    };
    const char* apcSyncTypeNames[4] = { "forward", "backward", "left", "right" };

    s_aAnimationData = new SoldierAnimationData[NUM_ANIM_DATA];
    s_aAnimationOverrideData = new BlendOverrideT[128];
    s_uiNumOverrides = 0;

    unsigned char auiSyncType[4];
    for (int i = 0; i < 4; ++i)
    {
        auiSyncType[i] = AddSyncType(apcSyncTypeNames[i], false);
    }

    SoldierAnimationData* aData = s_aAnimationData;
    for (int iAnimIndex = 0; iAnimIndex < NUM_ANIM_DATA / NUM_SCOPES; ++iAnimIndex)
    {
        for (int iScope = 0; iScope < NUM_SCOPES; ++iScope)
        {
            aData[iAnimIndex * NUM_SCOPES + iScope].Init((unsigned char)iAnimIndex, (Scope)iScope);
        }
    }

    for (int iAnim = 0; iAnim < NUM_ACTION_ANIMATIONS; ++iAnim)
    {
        int i = SoldierAnimType::GetAnimIndex((ActionAnimation)iAnim) * NUM_SCOPES;
        if (iAnim == SPRINT)
        {
            aData[i + UPPER].m_uiSyncType = auiSyncType[0];
            aData[i + LOWER].m_uiSyncType = auiSyncType[0];
        }
        else if (iAnim == JET)
        {
            aData[i + UPPER].SetAimType(AIM_TORSO);
        }
    }

    for (int iPosture = 0; iPosture < NUM_POSTURES; ++iPosture)
    {
        for (int iAnim = 0; iAnim < NUM_MOVEMENT_ANIMATIONS; ++iAnim)
        {
            int i = SoldierAnimType::GetAnimIndex((Posture)iPosture, (MovementAnimation)iAnim) * NUM_SCOPES;
            if (iAnim >= WALK_FORWARD)
            {
                aData[i + UPPER].m_uiSyncType = auiSyncType[(iAnim - WALK_FORWARD) >> 1];
            }
            if (iPosture >= NUM_STANDARD_POSTURES || iPosture == PRONE)
            {
                aData[i + UPPER].SetAimType(AIM_TORSO);
            }
            else
            {
                aData[i + UPPER].SetAimType(AIM_HEAD);
            }
            if (iAnim >= WALK_FORWARD)
            {
                aData[i + LOWER].m_uiSyncType = auiSyncType[(iAnim - WALK_FORWARD) >> 1];
            }
        }
    }

    for (int iPosture = 0; iPosture < NUM_STANDARD_POSTURES; ++iPosture)
    {
        for (int iAnim = 0; iAnim < NUM_WEAPON_ANIMATIONS; ++iAnim)
        {
            int i = SoldierAnimType::GetAnimIndex((Posture)iPosture, (WeaponAnimation)iAnim) * NUM_SCOPES;
            if (iAnim <= CHARGE || iPosture == PRONE)
            {
                aData[i + UPPER].SetAimType(AIM_TORSO);
            }
            else
            {
                aData[i + UPPER].SetAimType(AIM_HEAD);
            }
        }
    }

    if (!bDefaults)
    {
        return;
    }

    for (int iAnim = 0; iAnim < NUM_ACTION_ANIMATIONS; ++iAnim)
    {
        const AnimationSetting& setting = aActionSettings[iAnim];
        int i = SoldierAnimType::GetAnimIndex((ActionAnimation)iAnim) * NUM_SCOPES;
        aData[i + UPPER].SetLooping(setting.bLoop);
        aData[i + UPPER].SetBlendInTime(setting.fBlendInTime);
        aData[i + UPPER].SetBlendOutTime(setting.fBlendOutTime);
        aData[i + LOWER].SetLooping(setting.bLoop);
        aData[i + LOWER].SetBlendInTime(setting.fBlendInTime);
        aData[i + LOWER].SetBlendOutTime(setting.fBlendOutTime);
    }

    for (int iPosture = 0; iPosture < NUM_POSTURES; ++iPosture)
    {
        for (int iAnim = 0; iAnim < NUM_MOVEMENT_ANIMATIONS; ++iAnim)
        {
            bool bLoop = iAnim != TURN_LEFT && iAnim != TURN_RIGHT;
            bool bProne = iPosture == PRONE || iPosture == PRONE_ALERT;

            float fUpperTime;
            if (bProne)
            {
                fUpperTime = 0.4f;
            }
            else if (iAnim == IDLE)
            {
                fUpperTime = 0.5f;
            }
            else
            {
                fUpperTime = 0.25f;
            }

            int i = SoldierAnimType::GetAnimIndex((Posture)iPosture, (MovementAnimation)iAnim) * NUM_SCOPES;
            aData[i + UPPER].SetLooping(bLoop);
            aData[i + UPPER].SetBlendInTime(fUpperTime);
            aData[i + UPPER].SetBlendOutTime(fUpperTime);

            float fLowerTime;
            if (bProne)
            {
                fLowerTime = 0.4f;
            }
            else if (iAnim > IDLE_LOOK_AROUND)
            {
                fLowerTime = 0.15f;
            }
            else
            {
                fLowerTime = 0.5f;
            }

            aData[i + LOWER].SetLooping(bLoop);
            aData[i + LOWER].SetBlendInTime(fLowerTime);
            aData[i + LOWER].SetBlendOutTime(fLowerTime);

            if (iAnim >= WALK_FORWARD)
            {
                if (iPosture == STAND || iPosture == CROUCH)
                {
                    aData[i + LOWER].AddBlendOverride(
                        SoldierAnimType::GetAnimIndex((Posture)(iPosture + 3), (MovementAnimation)iAnim), 0.0f);
                }
                else if (iPosture == STAND_ALERT || iPosture == CROUCH_ALERT)
                {
                    aData[i + LOWER].AddBlendOverride(
                        SoldierAnimType::GetAnimIndex((Posture)(iPosture - 3), (MovementAnimation)iAnim), 0.0f);
                }
            }
            else if (iAnim <= IDLE_LOOK_AROUND && (iPosture == STAND || iPosture == CROUCH))
            {
                aData[i + LOWER].AddBlendOverride(
                    SoldierAnimType::GetAnimIndex((Posture)(iPosture + 3), (MovementAnimation)iAnim), 0.15f);
            }
        }
    }

    for (int iPosture = 0; iPosture < NUM_STANDARD_POSTURES; ++iPosture)
    {
        for (int iAnim = 0; iAnim < NUM_WEAPON_ANIMATIONS; ++iAnim)
        {
            float fUpperTime;
            if (iAnim <= SHOOT2)
            {
                fUpperTime = 0.02f;
            }
            else if (iAnim <= SHOOT_SECONDARY2)
            {
                fUpperTime = 0.15f;
            }
            else
            {
                fUpperTime = 0.5f;
            }

            int i = SoldierAnimType::GetAnimIndex((Posture)iPosture, (WeaponAnimation)iAnim) * NUM_SCOPES;
            aData[i + UPPER].SetBlendInTime(fUpperTime);
            aData[i + UPPER].SetBlendOutTime(fUpperTime);
            aData[i + LOWER].SetBlendInTime(0.5f);
            aData[i + LOWER].SetBlendOutTime(0.5f);
        }
    }
}

// FUNCTION: SWBF2 0x0063e390
void SoldierAnimationData::CleanupAnimationData()
{
    delete[] s_aAnimationData;
    s_aAnimationData = 0;
    delete[] s_aAnimationOverrideData;
    s_aAnimationOverrideData = 0;
    s_uiNumOverrides = 0;
}

// FUNCTION: SWBF2 0x0063e3d0
SoldierAnimationData* SoldierAnimationData::Find(unsigned int uiNameHash)
{
    if (!s_pAnimationNameTable)
    {
        return 0;
    }
    unsigned int uiIndex = (unsigned int)s_pAnimationNameTable->Find(uiNameHash);
    if (!uiIndex)
    {
        return 0;
    }
    uiIndex &= 0x7fffffff;
    if (uiIndex >= (unsigned int)NUM_ANIM_DATA)
    {
        return 0;
    }
    return &s_aAnimationData[uiIndex];
}

// FUNCTION: SWBF2 0x0063e410
SoldierAnimationData* SoldierAnimationData::Find(SoldierAnimationBank::COMBO_ANIMATION eAnimation,
                                                 Scope eScope)
{
    if (!s_pAnimationNameTable || eAnimation == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
    {
        return 0;
    }

    unsigned int uiHash =
        PblHash(SoldierAnimationBank::GetComboAnimationNameHash(eAnimation), GetScopeExtension(eScope));
    int iIndex = SoldierAnimType::GetAnimIndex(eAnimation) * NUM_SCOPES + eScope;
    if (!s_pAnimationNameTable->Find(uiHash))
    {
        s_pAnimationNameTable->Store(uiHash, (void*)(iIndex | 0x80000000));
    }
    return &s_aAnimationData[iIndex];
}

// FUNCTION: SWBF2 0x0063e4b0
unsigned char SoldierAnimationData::AddSyncType(const char* pcName, bool bByTime)
{
    if (!*pcName || !s_auiSyncTypeTable)
    {
        return 0;
    }

    unsigned int uiHash = PblHash(pcName);
    if (!uiHash)
    {
        uiHash = 0x80000000;
    }

    unsigned char uiStart = uiHash & 0x7f;
    if (!uiStart)
    {
        uiStart = 1;
    }
    if (bByTime)
    {
        uiStart |= 0x80;
    }

    unsigned char uiSync = uiStart;
    do
    {
        if (s_auiSyncTypeTable[uiSync] == uiHash)
        {
            return uiSync;
        }
        if (!s_auiSyncTypeTable[uiSync])
        {
            s_auiSyncTypeTable[uiSync] = uiHash;
            unsigned int uiLength = strlen(pcName);
            int iOldHeap = RedSetCurrentHeap(__RedTempHeap);
            char* pcCopy = new char[uiLength + 1];
            RedSetCurrentHeap(iOldHeap);
            strlcpy(pcCopy, pcName, uiLength + 1);
            s_aszSyncTypeNames[uiSync] = pcCopy;
            return uiSync;
        }
        if ((uiSync & 0x7f) == 0x7f)
        {
            uiSync = (uiSync & 0x80) | 1;
        }
        else
        {
            ++uiSync;
        }
    } while (uiSync != uiStart);

    return 0;
}

// FUNCTION: SWBF2 0x0063e5a0
void SoldierAnimationData::Read(PblFileChunk& chunk)
{
    InitLoadTimeData();
    InitAnimationData(false);

    PblConfig config(chunk);
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() != 0xe145ee5d || !config.IsScopeNext())
        {
            continue;
        }

        if (!data.IsStringArg(0))
        {
            config.ReadNextScope();
            continue;
        }

        const char* pcName = data.GetStringArg(0);
        const char* pcScope = strrchr(pcName, '_');
        unsigned int uiHash = PblHash(pcName);
        if (pcScope && _stricmp(pcScope, GetScopeExtension(LOWER)) &&
            _stricmp(pcScope, GetScopeExtension(UPPER)))
        {
            pcScope = 0;
        }

        PblConfig scope(config.ReadNextScope(), true);

        SoldierAnimationData* pData =
            Find(pcScope ? uiHash : (unsigned int)PblHash(uiHash, GetScopeExtension(UPPER)));
        if (!pData)
        {
            continue;
        }

        if (!pData->m_bReadFromConfig)
        {
            pData->ReadConfig(scope, pcName);
        }

        if (!pcScope)
        {
            SoldierAnimationData* pLower = Find(PblHash(uiHash, GetScopeExtension(LOWER)));
            if (pLower && !pLower->m_bReadFromConfig)
            {
                pLower->ConvertScope(*pData);
            }
        }
    }
}
