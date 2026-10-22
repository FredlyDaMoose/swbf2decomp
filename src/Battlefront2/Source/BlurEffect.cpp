#include "BlurEffect.h"

// GLOBAL: SWBF2 0x01e30318
BlurEffect* BlurEffect::s_pInstance;

// FUNCTION: SWBF2 0x0044c2f0
void BlurEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new BlurEffect();
    }
}

// FUNCTION: SWBF2 0x0044c380
BlurEffect::BlurEffect()
    : FLEffect(0x6BFBD198u, 7)
{
    Enable(false);
    m_pFilterShader = 0;
    m_pClearShader = 0;
}

// FUNCTION: SWBF2 0x0044c480
void BlurEffect::Init()
{
    PlatformInit();
}

// FUNCTION: SWBF2 0x0044c360
void BlurEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// SYNTHETIC: SWBF2 0x0044c400
// BlurEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0044c430
BlurEffect::~BlurEffect()
{
}

// FUNCTION: SWBF2 0x0044c2e0
bool BlurEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x0044c490
void BlurEffect::Cleanup()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x0044c4a0
void BlurEffect::ReadConfig(PblConfig& config)
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
