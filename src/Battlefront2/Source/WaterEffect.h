#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class ParticleEmitterObject;

// VTABLE: SWBF2 0x007b011c
class WaterEffect : public FLEffect
{
public:

    struct WaterEffectData
    {
        WaterEffectData()
        {
            effect.mObject = 0;
            effect.mSavedHandleId = 0;
        }

        PblVector3 worldPos;
        PblHandle<ParticleEmitterObject> effect;
    };

    WaterEffect()
        : FLEffect(0x49C69A10u, 3)
    {
    }

    virtual ~WaterEffect();
    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Init();
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    static void AddWaterEffect(unsigned int uiEffect, const PblVector3& vPos);

    static void AddDirectionalWaterEffect(unsigned int uiEffect, const PblVector3& vPos,
                                          const PblVector3& vDir);

    static WaterEffect* s_pInstance;

    WaterEffectData* m_waterEffects[200];
    unsigned int m_numWaterEffects;

private:

    void _AddWaterEffect(unsigned int uiEffect, const PblVector3& vPos, const PblVector3& vDir);
};
 
 
 
 
 
 