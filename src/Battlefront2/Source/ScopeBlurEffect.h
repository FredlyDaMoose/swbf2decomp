#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblConfig.h"

class RedTexture;
class pcClearShader;
class pcFilterCopyShader;

// VTABLE: SWBF2 0x007ae3cc
class ScopeBlurEffect : public FLEffect
{
public:

    ScopeBlurEffect();

    virtual ~ScopeBlurEffect();

    // SYNTHETIC: SWBF2 0x006333c0
    // ScopeBlurEffect::`scalar deleting destructor'

    virtual void Render(int value);

    virtual bool IsReflected();
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    static ScopeBlurEffect* s_pInstance;

    static int m_alpha[1];

    static void Start(int iCamera, int iAlpha)
    {
        if (iCamera > -1)
        {
            m_alpha[iCamera] = iAlpha;
        }
    }
    static void Stop(int iCamera)
    {
        if (iCamera > -1)
        {
            m_alpha[iCamera] = 0;
        }
    }

    pcFilterCopyShader* m_pFilterShader;
    pcClearShader* m_pClearShader;
    RedTexture* m_pBlurTexture;

    void PlatformInit();

    void PlatformCleanup();
    void PlatformReadConfigData(PblConfig::Data& data);

    void pcCopyBackBuffer() const;
};
 
 
 
 
 