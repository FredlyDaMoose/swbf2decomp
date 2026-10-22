#pragma once

#include <stddef.h>
#include <d3d9.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblConfig.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedVectorElement.h"

// VTABLE: SWBF2 0x007974b4

enum ControlT
{
    COLORCONTROL_GAMMA_CONTRAST = 0,
    COLORCONTROL_GAMMA_BRIGHTNESS = 1,
    COLORCONTROL_GAMMA_CORRECTION = 2,
    COLORCONTROL_MAX = 3
};

class ColorControl : public FLEffect
{
public:

    ColorControl();

    virtual ~ColorControl();
    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Init();
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    void PlatformEnable(bool bEnable);

    void PlatformInit();

    void PlatformReadConfigData(PblConfig::Data& data);

    void SetLevel(ControlT eControl, float fLevel);

    static ColorControl* s_pInstance;

private:

    void InitGammaRampDisplay();

    void UpdateDisplay(D3DGAMMARAMP& ramp);

public:
    float m_level[3];
    float m_levelInternal[3];
    D3DGAMMARAMP m_gammaRamp;
    RedScreenGroupElement m_graph;
    RedVectorElement m_red;
    RedVectorElement m_green;
    RedVectorElement m_blue;
    RedVectorElement m_axes;
};
 
 
 
 
 
 
 
 
 
 