#include "WaterEffect.h"
#include "ParticleEmitterObject.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "PebbleFL/Common/PblConfig.h"
#include "TriggerRegionManager.h"

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// GLOBAL: SWBF2 0x01eafd60
WaterEffect* WaterEffect::s_pInstance;

// FUNCTION: SWBF2 0x006770f0
void WaterEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new WaterEffect();
    }
}

// FUNCTION: SWBF2 0x00677200
void WaterEffect::Init()
{
    RedWater::Init();
    m_numWaterEffects = 0;
    for (int i = 0; i < 200; i++)
    {
        m_waterEffects[i] = new WaterEffectData;
    }
}

// FUNCTION: SWBF2 0x00677250
void WaterEffect::Cleanup()
{
    RedWater::Cleanup();

    for (unsigned int i = 0; i < m_numWaterEffects; i++)
    {
        if (m_waterEffects[i]->effect)
        {
            m_waterEffects[i]->effect->DeactivateEffect();
        }
    }

    for (int i = 0; i < 200; i++)
    {
        if (m_waterEffects[i])
        {
            delete m_waterEffects[i];
        }
    }
    m_numWaterEffects = 0;
}

// FUNCTION: SWBF2 0x006772f0
void WaterEffect::ReadConfig(PblConfig& config)
{
    RedWater::ReadConfig(config);
    if (RedWater::m_waterMode == RedWater::WATER_MODE_OCEAN)
    {
        float fHeight = 0.0f;
        RedWater::GetWaterHeight(PblVector3(0.0f, 0.0f, 0.0f), fHeight);
        fHeight += 20.0f;

        RedRegionDesc desc;
        desc.mType = (PblRegion::Type)0;
        desc.mMatrix = DirectionMatrix(PblVector3(0.0f, 1.0f, 0.0f), PblVector3(1.0f, 0.0f, 0.0f),
                                       PblVector3(0.0f, fHeight, 0.0f));
        desc.mNameId = 0x6EBDE457u;
        desc.mTypeId = 0xCCCB0055u;
        new RedRegion(desc);
        TriggerRegionManager::sInstance->AddSinkRegion("water_sink_region");
    }
}

// FUNCTION: SWBF2 0x00677470
void WaterEffect::Update(float fDeltaTime)
{
    for (unsigned int i = 0; i < m_numWaterEffects;)
    {
        if (m_waterEffects[i]->effect)
        {
            i++;
            continue;
        }
        WaterEffectData* pData = m_waterEffects[i];
        m_waterEffects[i] = m_waterEffects[m_numWaterEffects - 1];
        m_numWaterEffects--;
        m_waterEffects[m_numWaterEffects] = pData;
    }

    for (unsigned int i = 0; i < m_numWaterEffects; i++)
    {
        float fHeight;
        RedWater::GetWaterHeight(m_waterEffects[i]->worldPos, fHeight);
        m_waterEffects[i]->worldPos.y = fHeight + 0.02f;
    }
}

// FUNCTION: SWBF2 0x00677080
WaterEffect::~WaterEffect()
{
}

// FUNCTION: SWBF2 0x00677070
void WaterEffect::Render(int)
{
}

// FUNCTION: SWBF2 0x00677060
bool WaterEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x00677180
void WaterEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x006771a0
void WaterEffect::AddWaterEffect(unsigned int uiEffect, const PblVector3& vPos)
{
    if (s_pInstance)
    {
        s_pInstance->_AddWaterEffect(uiEffect, vPos, PblVector3(0.0f, 0.0f, 1.0f));
    }
}

// FUNCTION: SWBF2 0x006771e0
void WaterEffect::AddDirectionalWaterEffect(unsigned int uiEffect, const PblVector3& vPos,
                                            const PblVector3& vDir)
{
    if (s_pInstance)
    {
        s_pInstance->_AddWaterEffect(uiEffect, vPos, vDir);
    }
}

// FUNCTION: SWBF2 0x00677520
void WaterEffect::_AddWaterEffect(unsigned int uiEffect, const PblVector3& vPos, const PblVector3& vDir)
{
    WaterEffect* pWater = s_pInstance;
    if (pWater->m_numWaterEffects >= 200)
    {
        return;
    }

    FLEffectClass* pClass = FLEffect::Get(uiEffect);
    if (!pClass)
    {
        return;
    }
    FLEffectObject* pEffect = pClass->BuildEffect();
    if (!pEffect)
    {
        return;
    }

    WaterEffectData* pData = pWater->m_waterEffects[pWater->m_numWaterEffects];
    pData->effect = reinterpret_cast<ParticleEmitterObject*>(pEffect);
    pData->worldPos = vPos;
    pData->worldPos.y += 0.02f;
    pEffect->AttachEffectToVector(&pData->worldPos, vDir, true);
    reinterpret_cast<ParticleEmitterObject*>(pEffect)->mReflectionEnable = false;
    ++pWater->m_numWaterEffects;
}
