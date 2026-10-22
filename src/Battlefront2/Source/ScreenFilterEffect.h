#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblTimeBase.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblConfig.h"

// VTABLE: SWBF2 0x007ae418
class ScreenFilterEffect : public FLEffect
{
public:

    struct FilterData
    {
        FilterData() {}

        int uiState;
        float fStartFadeInTime;
        float fStopFadeInTime;
        float fStartFadeOutTime;
        float fStopFadeOutTime;
        float fIntensity;
        unsigned int uiTextureHash;
        RedColor filterColor;
    };

    ScreenFilterEffect();

    virtual ~ScreenFilterEffect();
    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Init();
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    void TimedFilter(int iViewport, float fDuration, float fFadeInTime, float fFadeOutTime,
                     RedColor color, unsigned int uiTextureHash);

    static ScreenFilterEffect* Instance() { return s_pInstance; }

    static ScreenFilterEffect* s_pInstance;

    PblTimeBase::Stamp m_filterTimeStamp;
    FilterData m_data[1];

    void PlatformInit();

    void PlatformRender(unsigned int uiFlags) const;

    void PlatformCleanup();
};
 
 
 
 
 