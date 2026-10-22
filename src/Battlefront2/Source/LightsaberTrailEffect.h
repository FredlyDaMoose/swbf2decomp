#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"

class pcRedShader;

// VTABLE: SWBF2 0x007a3d58
class LightsaberTrailEffect : public FLEffect
{
public:

    LightsaberTrailEffect()
        : FLEffect(0xFBAB1AF0u, 3)
    {
        m_pShader = 0;
    }

    void RenderLightsaberTrail(const class LightsaberTrail* pTrail, unsigned int uiFlags) const;

    virtual ~LightsaberTrailEffect();
    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);
    virtual FLEffectClass* BuildClass(PblConfig& config) const;

    static void Create();

    static void Destroy();

    static LightsaberTrailEffect* s_pInstance;

    pcRedShader* m_pShader;
    float m_fLIGHTSABER_TRAIL_PERSIST_TIME;
    float m_fLIGHTSABER_TRAIL_FADE_TIME;
    float m_fLIGHTSABER_TRAIL_FADE_DISTANCE;
    float m_fLIGHTSABER_TRAIL_CORE_MAX_TIME;
    float m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE;
    float m_fLIGHTSABER_TRAIL_CORE_WIDTH;
    float m_fLIGHTSABER_TRAIL_MAX_TIME_PER_STEP;
    float m_fLIGHTSABER_TRAIL_MAX_DISTANCE_PER_STEP;

private:

    void _PlatformInit();

    void _PlatformCleanup();
};
 
 
 
 
 // VTABLE: SWBF2 0x007b156c
class LightsaberTrailClass : public FLEffectClass
{
public:

    LightsaberTrailClass();
    virtual ~LightsaberTrailClass();
    virtual FLEffectObject* BuildEffect();

    PblVector3 m_vOffsetPos;
    PblVector3 m_vOffsetDir;
    float m_fLength;
    float m_fWidth;
    RedColor m_colorTrail;
    RedColor m_colorHead;
};
 
 

// VTABLE: SWBF2 0x007a3d20
// VTABLE: SWBF2 0x007a3d18
class LightsaberTrail : public FLEffectObject
{
public:
    enum { MAX_SAMPLES_PER_TRAIL = 8 };

    LightsaberTrail(const LightsaberTrailClass* pClass);
    virtual ~LightsaberTrail();
    virtual bool IsEffectActive() const;
    virtual void ActivateEffect();
    virtual void DeactivateEffect();
protected:
    virtual void UpdatedWorldMatrix();
    virtual void AddToRenderList();
    virtual void RemoveFromRenderList();

public:

    float GetLastTime() const;

    int GetTotalNumSteps(float fMaxTimeStep, float fMaxDistanceStep, unsigned char* pSteps,
                         float* pTotalDistance) const;
    PblBox GetBoundingBox(float fLength) const;
    int _SetSample(float fTime) const;
    PblVector3* GetInterpolatedPos(float fTime, PblVector3* pResult) const;
    PblVector3* GetInterpolatedDir(float fTime, PblVector3* pResult) const;

protected:

    void _AddSample(float fTime, const PblVector3& vPos, const PblVector3& vDir, bool bHead);
    void _Update(float fDeltaTime, const PblVector3& vPos, const PblVector3& vDir,
                 const PblVector3& vPosPrev, const PblVector3& vDirPrev, bool bTrailOn);

public:
    LightsaberTrailClass* m_pClass;
    PblVector3 m_vPosPrev;
    PblVector3 m_vDirPrev;
    unsigned char m_bTrailOn : 1;
    unsigned char m_bTrailWasOn : 1;
    unsigned char m_bEffectActive : 1;
    unsigned char z_uiReserved : 5;
    unsigned char m_uiNumSamples;
    unsigned char m_uiSampleNext;
    unsigned char m_uiSampleFirst;
    float m_fTimer;
    float m_afTime[MAX_SAMPLES_PER_TRAIL];
    PblVector3 m_avPos[MAX_SAMPLES_PER_TRAIL];
    PblVector3 m_avVel[MAX_SAMPLES_PER_TRAIL];
    PblVector3 m_avDir[MAX_SAMPLES_PER_TRAIL];
    PblVector3 m_avOmg[MAX_SAMPLES_PER_TRAIL];
    float m_afDist[MAX_SAMPLES_PER_TRAIL];
public:
    static unsigned char s_uiSample;
    static unsigned char s_uiSamplePrev;
    static float s_fDeltaTimeInv;
};
 
 
 
 
 
 