#pragma once

#include <stddef.h>
#include "FLEffect.h"

// VTABLE: SWBF2 0x007aa560
class MotionBlurEffect : public FLEffect
{
public:

    MotionBlurEffect();

    virtual ~MotionBlurEffect();
    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Init();
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static MotionBlurEffect* s_pInstance;

    static float m_curBlurAmount[1];

    static int m_state[1];

    void PlatformInit();

    static void Destroy();

    static void Start(int, float fBlurAmount, int iCamera);
    static void Stop(int, int iCamera);

    void PlatformCleanup();
    void PlatformRender(unsigned int uiFlags) const;
};
 
 