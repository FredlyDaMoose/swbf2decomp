#include "WorldShadowMapEffect.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"

// GLOBAL: SWBF2 0x01eafd80
WorldShadowMapEffect* WorldShadowMapEffect::s_pInstance;

// FUNCTION: SWBF2 0x006950f0
void WorldShadowMapEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new WorldShadowMapEffect();
    }
}

// FUNCTION: SWBF2 0x006951a0
void WorldShadowMapEffect::Init()
{
    m_pDirLight = 0;
    m_uiLightName = 0;
    m_uiTextureHash = 0;
    m_fTextureScale = 1.0f;
}

// FUNCTION: SWBF2 0x00695180
void WorldShadowMapEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// SYNTHETIC: SWBF2 0x00695080
// WorldShadowMapEffect::`scalar deleting destructor'

WorldShadowMapEffect::~WorldShadowMapEffect()
{
}

// FUNCTION: SWBF2 0x00695070
void WorldShadowMapEffect::Render(int)
{
}

// FUNCTION: SWBF2 0x00695060
bool WorldShadowMapEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x006951c0
void WorldShadowMapEffect::Cleanup()
{
}

// FUNCTION: SWBF2 0x006951d0
void WorldShadowMapEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x60837D9Eu:
            if (data.GetNumArgs() == 2)
            {
                m_vAnimationAmplitude0 = PblVector3(data.GetFloatArg(0), 0.0f, data.GetFloatArg(1));
            }
            break;
        case 0x61837F31u:
            if (data.GetNumArgs() == 2)
            {
                m_vAnimationAmplitude1 = PblVector3(data.GetFloatArg(0), 0.0f, data.GetFloatArg(1));
            }
            break;
        case 0x1F3B36E9u:
            m_fAnimationFrequency = data.GetFloatArg(0);
            break;
        case 0x3C6468F4u:
            m_uiTextureHash = PblHash(data.GetStringArg(0));
            break;
        case 0x7FAB682Cu:
            m_uiLightName = PblHash(data.GetStringArg(0));
            break;
        case 0x6D9B436Eu:
            m_fTextureScale = data.GetFloatArg(0);
            break;
        case 0xAF8BB8CEu:
            Enable(data.GetUint32Arg(0) != 0);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00695340
void WorldShadowMapEffect::Update(float fDeltaTime)
{
    m_fTimer += fDeltaTime;

    float fU;
    float fV;
    if (m_fAnimationFrequency != 0.0f)
    {
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, m_fAnimationFrequency * m_fTimer * PblMath::fTwoPi);
        fU = m_vAnimationAmplitude1.x * fSin + m_vAnimationAmplitude0.x * fCos;
        fV = m_vAnimationAmplitude1.z * fSin + m_vAnimationAmplitude0.z * fCos;
    }
    else
    {
        fU = m_vAnimationAmplitude0.x * m_fTimer;
        fV = m_vAnimationAmplitude0.z * m_fTimer;
    }

    if (m_pDirLight)
    {
        m_pDirLight->SetProjectionTextureOffset(fU, fV);
        return;
    }

    RedLight* pLight = (RedLight*)RedLight::Find(m_uiLightName);
    if (pLight && pLight->m_eType == RedLight::TYPE_DIRECTIONAL)
    {
        m_pDirLight = (RedDirectionalLight*)pLight;
        m_pDirLight->SetProjectionTexture(m_uiTextureHash);
        m_pDirLight->SetProjectionTextureWrapMode(1);
        m_pDirLight->SetProjectionTextureTile(m_fTextureScale, m_fTextureScale);
        m_pDirLight->SetProjectionTextureOffset(fU, fV);
        m_pDirLight->SetProjectionTextureBlendMode(0);
    }
}
