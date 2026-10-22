#include "HDREffect.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "PebbleFL/Common/PblConfig.h"

// GLOBAL: SWBF2 0x01e56bd0
HDREffect* HDREffect::s_pInstance;

// FUNCTION: SWBF2 0x005421d0
bool HDREffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x005421e0
void HDREffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new HDREffect();
    }
}

// FUNCTION: SWBF2 0x00542250
void HDREffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x00542270
HDREffect::HDREffect()
    : FLEffect(0xFEB6DBE1u, 9)
{
    Enable(false);
    m_downSizeFactor = 0.25f;
    m_numBloomPasses = 5;
    m_maxTotalWeight = 1.2f;
    m_glowThreshold = 0.5f;
    m_glowFactor = 1.0f;
    SetRenderPriority(0x271A);
}

// FUNCTION: SWBF2 0x00542360
HDREffect::~HDREffect()
{
}

namespace FLRenderer
{
unsigned int GetViewportCount();
}

// FUNCTION: SWBF2 0x005423b0
void HDREffect::Init()
{
    PlatformInit();
    if (FLRenderer::GetViewportCount() > 1)
    {
        Enable(false);
    }
}

// FUNCTION: SWBF2 0x00542520
void HDREffect::Enable(bool bEnable)
{
    FLEffect::Enable(bEnable);
    RedRenderer::SetLightingScale(bEnable ? 0.5f : 1.0f);
}

// FUNCTION: SWBF2 0x005423d0
void HDREffect::Cleanup()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x005423e0
void HDREffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0xAF8BB8CEu:
            Enable(data.GetUint32Arg(0) != 0);
            break;
        case 0x7E6C7187u:
            m_downSizeFactor = data.GetFloatArg(0);
            break;
        case 0xD4EF5803u:
            m_numBloomPasses = data.GetUint32Arg(0);
            break;
        case 0xEAFD07EDu:
            m_maxTotalWeight = data.GetFloatArg(0);
            break;
        case 0xB12C9B1Fu:
            m_glowThreshold = data.GetFloatArg(0);
            break;
        case 0xAC9EEE97u:
            m_glowFactor = data.GetFloatArg(0);
            break;
        }
    }
    if (FLRenderer::GetViewportCount() > 1)
    {
        Enable(false);
    }
    UpdateHDREnabled();
}

// FUNCTION: SWBF2 0x00542500
void HDREffect::UpdateHDREnabled()
{
    Enable(RedRenderer::pcGetRenderOptions().mEnableHDR);
}
