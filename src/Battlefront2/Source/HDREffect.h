#pragma once

#include <stddef.h>
#include "FLEffect.h"

class pcHDRShader;
class RedTexture;

// VTABLE: SWBF2 0x007a0288
class HDREffect : public FLEffect
{
public:

    struct HistogramT
    {
        float Lums[256];
        float fMin;
        float fMax;
        float fAvg;
    };

    HDREffect();

    virtual ~HDREffect();

    // SYNTHETIC: SWBF2 0x00542330
    // HDREffect::`scalar deleting destructor'

    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    void Enable(bool bEnable);

    static HDREffect* s_pInstance;

    static HDREffect* Instance() { return s_pInstance; }

    HistogramT m_histogram;
    float m_downSizeFactor;
    unsigned int m_numBloomPasses;
    float m_maxTotalWeight;
    float m_glowThreshold;
    float m_glowFactor;
    pcHDRShader* m_pHDRShader;

    void PlatformInit();

    void PlatformCleanup();

    void pcCopyBackBuffer(const RedTexture* pTexture) const;

    void UpdateHDREnabled();
};
 
 
 
 
 
 
 
 
 
 