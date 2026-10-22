#pragma once

#include <string.h>

#include "SoldierAnimType.h"
#include "PebbleFL/Common/PblHashTable.h"

class PblConfig;
class PblFileChunk;

class SoldierAnimationData
{
public:

    enum AIM_TYPE
    {
        AIM_NONE = 0,
        AIM_HEAD = 1,
        AIM_TORSO = 2,
        AIM_FULLBODY = 3
    };

    enum LOOP_TYPE
    {
        LOOP_END = 0,
        LOOP_RESTART = 1,
        LOOP_FINAL_FRAME = 2,
        NUM_LOOP_TYPES = 3
    };

    struct BlendOverrideT
    {

        float GetBlendTime() const { return uiBlendTime * BLEND_TIME_SCALE_INV; }
        void SetBlendTime(float fTime)
        {
            float f = fTime * BLEND_TIME_SCALE;
            if (f < 0.0f)
            {
                f = 0.0f;
            }
            else if (f > BLEND_TIME_MAX)
            {
                f = BLEND_TIME_MAX;
            }
            uiBlendTime = (unsigned char)(int)f;
        }

        unsigned char uiAnimIndex;
        unsigned char uiBlendTime;
    };

    struct NameTable
    {
        NameTable() { _table.Clear(); _iNumEntries = 0; }
        void* Find(unsigned int uiHash) const { return _table.Find(uiHash); }
        void Store(unsigned int uiHash, void* pValue)
        {
            if (_table.Store(uiHash, pValue))
            {
                ++_iNumEntries;
            }
        }

        int _iNumEntries;
        PblHashTable<void, 512> _table;
    };

    void Init(unsigned char uiAnimIndex, SoldierAnimType::Scope eScope)
    {
        memset(this, 0, sizeof(*this));
        m_uiAnimIndex = uiAnimIndex;
        m_uiScope = eScope;
    }

    void SetLooping(bool bLooping) { m_uiLoop = bLooping; }
    void SetLoopType(LOOP_TYPE eLoop) { m_uiLoop = eLoop; }
    void SetAimType(AIM_TYPE eAim) { m_uiAimType = eAim; }

    bool IsLooping() const { return m_uiLoop == LOOP_RESTART; }

    static bool IsSyncTypeByTime(unsigned char uiSyncType) { return (uiSyncType & 0x80) != 0; }

    void SetBlendInTime(float fTime)
    {
        float f = fTime * BLEND_TIME_SCALE;
        if (f < 0.0f)
        {
            f = 0.0f;
        }
        else if (f > BLEND_TIME_MAX)
        {
            f = BLEND_TIME_MAX;
        }
        m_uiBlendInTime = (int)f;
    }
    void SetBlendOutTime(float fTime)
    {
        float f = fTime * BLEND_TIME_SCALE;
        if (f < 0.0f)
        {
            f = 0.0f;
        }
        else if (f > BLEND_TIME_MAX)
        {
            f = BLEND_TIME_MAX;
        }
        m_uiBlendOutTime = (int)f;
    }

    unsigned char GetAnimIndex() const { return (unsigned char)m_uiAnimIndex; }
    float GetBlendOutTime() const { return m_uiBlendOutTime * BLEND_TIME_SCALE_INV; }

    bool IsLowResPose() const { return m_uiLowResPoseFrame != 0; }
    unsigned short GetLowResPoseFrame() const { return (unsigned short)(m_uiLowResPoseFrame - 1); }

    SoldierAnimationBank::COMBO_ANIMATION GetComboAnimation() const;
    float GetBlendTime(float fTime) const;
    float GetBlendTime(float fTime, unsigned char uiAnimIndex) const;
    void AddBlendOverride(unsigned char uiAnimIndex, float fTime);
    void ConvertScope(const SoldierAnimationData& data);
    void ReadConfig(PblConfig& config, const char* pcName);

    static void InitLoadTimeData();
    static void InitLoadTimeDataPostLoad();
    static void DiscardLoadTimeData();
    static void InitAnimationData(bool bDefaults);
    static SoldierAnimationData* Find(unsigned int uiNameHash);
    static SoldierAnimationData* Find(SoldierAnimationBank::COMBO_ANIMATION eAnimation,
                                      SoldierAnimType::Scope eScope);
    static void Read(PblFileChunk& chunk);

    static const SoldierAnimationData* Get(unsigned char uiAnimIndex, SoldierAnimType::Scope eScope)
    {
        return &s_aAnimationData[uiAnimIndex * SoldierAnimType::NUM_SCOPES + eScope];
    }

    static void CleanupAnimationData();

private:
    static unsigned char AddSyncType(const char* pcName, bool bByTime);

public:
    unsigned int m_uiAnimIndex : 8;
    unsigned int m_uiSyncType : 8;
    unsigned int m_uiAimType : 2;
    unsigned int z_uiReserved0 : 2;
    unsigned int m_uiScope : 1;
    unsigned int m_bReadFromConfig : 1;
    unsigned int m_uiLoop : 2;
    unsigned int m_uiBlendInTime : 8;
    unsigned int m_uiBlendOutTime : 8;
    unsigned int m_uiLowResPoseFrame : 8;
    unsigned int m_uiNumBlendOverrides : 8;
    BlendOverrideT* m_aBlendOverride;

    static const float BLEND_TIME_SCALE;
    static const float BLEND_TIME_SCALE_INV;
    static const float BLEND_TIME_MAX;
    static const int NUM_ANIM_DATA;

    static SoldierAnimationData* s_aAnimationData;
    static BlendOverrideT* s_aAnimationOverrideData;
    static unsigned int s_uiNumOverrides;
    static NameTable* s_pAnimationNameTable;
    static unsigned int* s_auiSyncTypeTable;
    static char** s_aszSyncTypeNames;
};
 
 
 
