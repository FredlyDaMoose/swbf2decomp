#pragma once

#include <stddef.h>
#include "FLEffect.h"

// VTABLE: SWBF2 0x007ae454
class ShadowEffect : public FLEffect
{
public:

    ShadowEffect();

    virtual ~ShadowEffect();

    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    void Enable(bool bEnable);

    static ShadowEffect* s_pInstance;

    bool m_blurEnable;
    float m_shadowIntensity;
};
 
 
 
 