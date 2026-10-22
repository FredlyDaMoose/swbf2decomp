#include "ShadowEffect.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Application/pc/pcRenderStateHelpers.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcShadowQuadRenderable.h"
#include "PebbleFL/Common/PblConfig.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"

// GLOBAL: SWBF2 0x01eaf02c
ShadowEffect* ShadowEffect::s_pInstance;

// FUNCTION: SWBF2 0x00635a50
void ShadowEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new ShadowEffect();
    }
}

// FUNCTION: SWBF2 0x00635ae0
ShadowEffect::ShadowEffect()
    : FLEffect(0x3A43C561u, 3)
{
    Enable(true);
    m_blurEnable = false;
    m_shadowIntensity = 0.5f;
}

namespace FLRenderer
{
unsigned int GetViewportCount();
}

// FUNCTION: SWBF2 0x00635be0
void ShadowEffect::Init()
{
    s_pInstance->Enable(!(FLRenderer::GetViewportCount() > 1));
}

// FUNCTION: SWBF2 0x00635d00
void ShadowEffect::Enable(bool bEnable)
{
    FLEffect::Enable(bEnable);
    RedShadowMap::_bShadowMapEnable = bEnable;
    RedStencilShadow::s_bShadowsEnabled = bEnable;
}

// SYNTHETIC: SWBF2 0x00635b60
// ShadowEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00635b90
ShadowEffect::~ShadowEffect()
{
}

// FUNCTION: SWBF2 0x00635c00
void ShadowEffect::Cleanup()
{
}

// FUNCTION: SWBF2 0x00635c10
void ShadowEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x1ED46057u:
            m_blurEnable = data.GetUint32Arg(0) != 0;
            break;
        case 0x8563E50Au:
        {
            float fIntensity = *(float*)&data.m_auiArgs[0];
            if (fIntensity < 0.0f)
            {
                fIntensity = 0.0f;
            }
            else if (fIntensity > 1.0f)
            {
                fIntensity = 1.0f;
            }
            m_shadowIntensity = fIntensity;
            break;
        }
        case 0xAF8BB8CEu:
            Enable(data.GetUint32Arg(0) != 0);
            break;
        }
    }
    ShadowEffect::Init();
}

// FUNCTION: SWBF2 0x00635d20
void ShadowEffect::Render(int)
{
    RedRenderer::SetShadowIntensity(m_shadowIntensity);
    pcShadowQuadRenderable::Instance()->EnableBlur(m_blurEnable);
}

// FUNCTION: SWBF2 0x00635ac0
void ShadowEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}
