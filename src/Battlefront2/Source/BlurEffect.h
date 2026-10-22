#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblConfig.h"

class pcClearShader;
class pcFilterCopyShader;

// VTABLE: SWBF2 0x00796f98
class BlurEffect : public FLEffect
{
public:

    BlurEffect();

    virtual ~BlurEffect();
    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    static BlurEffect* s_pInstance;

    pcFilterCopyShader* m_pFilterShader;
    pcClearShader* m_pClearShader;
    float m_constantBlend;
    float m_downSizeFactor;

    void PlatformInit();

    void PlatformCleanup();
    void PlatformReadConfigData(PblConfig::Data& data);

    void pcCopyBackBuffer() const;
};
 
 
 
 
 
 