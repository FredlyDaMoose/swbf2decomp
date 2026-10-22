#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblVector.h"

class RedDirectionalLight;

// VTABLE: SWBF2 0x007b1e2c
class WorldShadowMapEffect : public FLEffect
{
public:

    WorldShadowMapEffect()
        : FLEffect(0xC39291D3u, 3)
    {
        Enable(false);
    }

    virtual ~WorldShadowMapEffect();
    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Init();
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    static WorldShadowMapEffect* s_pInstance;

    PblVector3 m_vAnimationAmplitude0;
    PblVector3 m_vAnimationAmplitude1;
    float m_fAnimationFrequency;
    float m_fTimer;
    RedDirectionalLight* m_pDirLight;
    unsigned int m_uiLightName;
    unsigned int m_uiTextureHash;
    float m_fTextureScale;
};
 
 
 
 
 
 
 
 
 
 