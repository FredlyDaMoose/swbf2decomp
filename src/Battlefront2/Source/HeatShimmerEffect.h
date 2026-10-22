#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblTimeBase.h"

class pcRedPrimitive;
class pcRefractionShader;

// VTABLE: SWBF2 0x007a02bc
class HeatShimmerEffect : public FLEffect
{
public:

    HeatShimmerEffect();

    virtual ~HeatShimmerEffect();
    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Init();
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    static HeatShimmerEffect* s_pInstance;

    static PblTimeBase::Stamp s_shimmerTimeStamp;

    static float s_distortionScale;

    static float s_height;
    static float s_scrollSpeed;
    static float s_posY;

    static unsigned int s_tessellationX;
    static unsigned int s_tessellationY;

    static float s_tileU;
    static unsigned int s_bumpMapHash;
    static float s_tileV;
    static pcRedPrimitive* s_pPrimitive;
    static pcRefractionShader* s_pShimmerShader;

    void PlatformPreReadInit();

    void PlatformCleanup();
    void PlatformPostReadInit();
    void PlatformReadConfigData(PblConfig::Data& data);
    void PlatformRender(unsigned int flags) const;
};
 
 