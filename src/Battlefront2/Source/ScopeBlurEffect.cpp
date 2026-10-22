#include "ScopeBlurEffect.h"

// GLOBAL: SWBF2 0x01eaf018
ScopeBlurEffect* ScopeBlurEffect::s_pInstance;

// GLOBAL: SWBF2 0x01eaf01c
int ScopeBlurEffect::m_alpha[1];

// FUNCTION: SWBF2 0x006332b0
void ScopeBlurEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new ScopeBlurEffect();
    }
}

// FUNCTION: SWBF2 0x00633340
ScopeBlurEffect::ScopeBlurEffect()
    : FLEffect(0xF35CF5C6u, 9)
{
    Enable(true);
    m_alpha[0] = 0;
}

// FUNCTION: SWBF2 0x00633440
void ScopeBlurEffect::Init()
{
    PlatformInit();
}

// FUNCTION: SWBF2 0x006332a0
bool ScopeBlurEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x00633320
void ScopeBlurEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x006333f0
ScopeBlurEffect::~ScopeBlurEffect()
{
}

// FUNCTION: SWBF2 0x00633450
void ScopeBlurEffect::Cleanup()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x00633460
void ScopeBlurEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() == 0xAF8BB8CEu)
        {
            Enable(data.GetUint32Arg(0) != 0);
        }
        else
        {
            PlatformReadConfigData(data);
        }
    }
}
