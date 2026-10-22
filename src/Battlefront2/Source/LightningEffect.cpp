#include "LightningEffect.h"
#include "CameraManager.h"
#include "LightningBoltEffect.h"
#include "SkyDome.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcTerrainShader.h"
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndspace.h"

#undef PlaySound

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

namespace
{

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t)
{
    RedColor result;
    result.b = (unsigned char)(int)(a.b * (1.0f - t) + b.b * t + 0.5f);
    result.g = (unsigned char)(int)(a.g * (1.0f - t) + b.g * t + 0.5f);
    result.r = (unsigned char)(int)(a.r * (1.0f - t) + b.r * t + 0.5f);
    result.a = (unsigned char)(int)(a.a * (1.0f - t) + b.a * t + 0.5f);
    return result;
}

inline unsigned char ClampColorComponent(float f)
{
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    return (unsigned char)(int)f;
}
}

// FUNCTION: SWBF2 0x00572830
bool LightningEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x00572840
void LightningEffect::Render(int)
{
}

// GLOBAL: SWBF2 0x01e573e4
LightningEffect* LightningEffect::s_pInstance;

// FUNCTION: SWBF2 0x00572850
void LightningEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new LightningEffect();
    }
}

// FUNCTION: SWBF2 0x005728c0
void LightningEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x005728e0
LightningEffect::LightningEffect()
    : FLEffect(0xE812F405u, 3)
{
    Enable(false);
    m_colorLightning.r = 0.937254965f;
    m_colorLightning.g = 0.972549081f;
    m_colorLightning.b = 1.0f;
    m_colorLightning.a = 1.0f;
    m_fSunlightFadeMax = 0.1f;
    m_fSkyDomeDarken = 0.4f;
    m_fBrightnessMin = 0.9f;
    m_fTimeToFade = 0.2f;
    m_fTimeBetweenFlashesMin = 3.0f;
    m_fTimeBetweenFlashesMax = 8.0f;
    m_fTimeBetweenSubFlashesMin = 0.08f;
    m_fTimeBetweenSubFlashesMax = 0.16f;
    m_iNumSubFlashesMin = 2;
    m_iNumSubFlashesMax = 5;
    m_fHorizonAngleMin = 0.523598790f;
    m_fHorizonAngleMax = 1.04719758f;
    m_fTimer = 0.0f;
    m_fTimeNextFlash = 8.0f;
    m_iNumSubFlashesRemaining = 0;
    m_bInitRestore = false;
    mBoltFrequencyMin = 4.0f;
    mBoltFrequencyMax = 7.0f;
    mBoltTimer = 0.0f;
}

// SYNTHETIC: SWBF2 0x00572a00
// LightningEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00572a30
LightningEffect::~LightningEffect()
{
}

// FUNCTION: SWBF2 0x00572a80
void LightningEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0xAF8BB8CEu:
            Enable(data.GetUint32Arg(0) != 0);
            break;
        case 0x3D7E6258u:
        {
            unsigned char r = ClampColorComponent(data.GetFloatArg(0));
            unsigned char g = ClampColorComponent(data.GetFloatArg(1));
            unsigned char b = ClampColorComponent(data.GetFloatArg(2));
            RedColorValue color;
            color.r = r * (1.0f / 255.0f);
            color.g = g * (1.0f / 255.0f);
            color.b = b * (1.0f / 255.0f);
            color.a = 1.0f;
            m_colorLightning = color;
            break;
        }
        case 0x15F3DEB6u:
            m_fSunlightFadeMax = data.GetFloatArg(0);
            break;
        case 0x034AEBFDu:
            m_fSkyDomeDarken = data.GetFloatArg(0);
            break;
        case 0xDB50529Eu:
            m_fBrightnessMin = data.GetFloatArg(0);
            break;
        case 0xA40F930Au:
            m_fTimeToFade = data.GetFloatArg(0);
            break;
        case 0x260E9234u:
            m_fTimeBetweenFlashesMin = data.GetFloatArg(0);
            m_fTimeBetweenFlashesMax = data.GetFloatArg(1);
            break;
        case 0xD71D3C32u:
            m_fTimeBetweenSubFlashesMin = data.GetFloatArg(0);
            m_fTimeBetweenSubFlashesMax = data.GetFloatArg(1);
            break;
        case 0x67F9208Du:
            m_iNumSubFlashesMin = data.GetUint32Arg(0);
            m_iNumSubFlashesMax = data.GetUint32Arg(1);
            break;
        case 0xFC6B14B1u:
            m_fHorizonAngleMin = data.GetFloatArg(0) * 0.017453292f;
            m_fHorizonAngleMax = data.GetFloatArg(1) * 0.017453292f;
            break;
        case 0xE46BF7A2u:
            m_soundCrack.SetID(data.GetStringArg(0), false, false);
            break;
        case 0x882E50B8u:
            m_soundSubCrack.SetID(data.GetStringArg(0), false, false);
            break;
        }
    }
    m_fTimer = 0.0f;
    m_fTimeNextFlash = m_fTimeBetweenFlashesMax;
    m_iNumSubFlashesRemaining = 0;
    m_bInitRestore = false;
}

// FUNCTION: SWBF2 0x00572d40
void LightningEffect::PlaySound(GameSound& sound)
{
    bool bOutdoors = false;
    for (int i = 0; i < 4; ++i)
    {
        const Snd::Listener& listener = Snd::EngineBase::smListeners[i];
        bOutdoors |= listener.Enabled() && listener.GetSpace() == &Snd::Space::smDefaultSpace;
    }
    if (bOutdoors)
    {
        sound.Play(0, 0, false, true);
    }
}

// FUNCTION: SWBF2 0x00572de0
void LightningEffect::Update(float fDeltaTime)
{
    if (!m_bInitRestore)
    {
        m_colorSunlight_Restore = RedLight::GetGlobalDirectionalLight(0)->m_color;
        m_colorAmbient_Restore = RedLight::GetGlobalAmbientLight()->m_color;
        RedLight::GetGlobalDirectionalAmbientColors(m_colorDirectionalAmbient_Restore[0],
                                                    m_colorDirectionalAmbient_Restore[1]);
        m_vSunlightDir_Restore = RedLight::GetGlobalDirectionalLight(0)->m_vDirection;
        m_fSunlightBrightness = (float)((unsigned int)m_colorSunlight_Restore.r +
                                        (unsigned int)m_colorSunlight_Restore.g +
                                        (unsigned int)m_colorSunlight_Restore.b) / 765.0f;
        m_colorFog_Restore = RedRenderer::GetFogColor();
        m_bInitRestore = true;
    }

    float fBoltDist = 800.0f;
    float fBoltHeight = 300.0f;

    m_fTimer += fDeltaTime;
    if (m_fTimer > m_fTimeNextFlash)
    {
        m_fTimer = 0.0f;
        if (m_iNumSubFlashesRemaining <= 0)
        {
            float fElevation = (m_fHorizonAngleMax - m_fHorizonAngleMin) * PblRandom::GetGlobalFloat() +
                               m_fHorizonAngleMin;
            float fHeading = PblRandom::GetGlobalFloat() * PblMath::fTwoPi;
            float fSinElevation, fCosElevation, fSinHeading, fCosHeading;
            PblMath::SinCos(&fSinElevation, &fCosElevation, fElevation);
            PblMath::SinCos(&fSinHeading, &fCosHeading, fHeading);
            m_vDir.x = -(fSinHeading * fCosElevation);
            m_vDir.y = -fSinElevation;
            m_vDir.z = -(fCosHeading * fCosElevation);

            m_iNumSubFlashesRemaining =
                (int)(((float)(m_iNumSubFlashesMax - m_iNumSubFlashesMin) + 0.9999f) *
                          PblRandom::GetGlobalFloat() +
                      (float)m_iNumSubFlashesMin);
            PlaySound(m_soundCrack);
            fBoltDist = 200.0f;
            fBoltHeight = 200.0f;
            mBoltTimer = 0.0f;
        }
        --m_iNumSubFlashesRemaining;
        m_fBrightness = (1.0f - m_fBrightnessMin) * PblRandom::GetGlobalFloat() + m_fBrightnessMin;
        if (m_iNumSubFlashesRemaining > 0)
        {
            m_fTimeNextFlash = (m_fTimeBetweenSubFlashesMax - m_fTimeBetweenSubFlashesMin) *
                                   PblRandom::GetGlobalFloat() +
                               m_fTimeBetweenSubFlashesMin;
            PlaySound(m_soundSubCrack);
        }
        else
        {
            m_fTimeNextFlash = (m_fTimeBetweenFlashesMax - m_fTimeBetweenFlashesMin) *
                                   PblRandom::GetGlobalFloat() +
                               m_fTimeBetweenFlashesMin;
        }
    }

    if (m_fTimer < m_fTimeToFade)
    {
        float fFade = m_fTimer / m_fTimeToFade;
        float fSunlight = fFade * m_fSunlightBrightness;
        float fFlash = (1.0f - fFade) * m_fBrightness;
        float fBrightness = (PblRandom::GetGlobalFloat() - 0.5f) * (1.0f - fFlash) * fFlash + fFlash;
        if (fBrightness > fSunlight)
        {
            float fLightning = fBrightness - fSunlight;
            float fDarken = 1.0f - m_fSunlightFadeMax;

            RedColorValue colorAmbient;
            colorAmbient.r = m_colorAmbient_Restore.r * fDarken + m_colorSunlight_Restore.r * 0.333f * fDarken;
            colorAmbient.g = m_colorAmbient_Restore.g * fDarken + m_colorSunlight_Restore.g * 0.333f * fDarken;
            colorAmbient.b = m_colorAmbient_Restore.b * fDarken + m_colorSunlight_Restore.b * 0.333f * fDarken;
            colorAmbient.a = 1.0f;

            RedColorValue colorTop;
            colorTop.r = m_colorDirectionalAmbient_Restore[0].r * fDarken + m_colorSunlight_Restore.r * 0.333f * fDarken;
            colorTop.g = m_colorDirectionalAmbient_Restore[0].g * fDarken + m_colorSunlight_Restore.g * 0.333f * fDarken;
            colorTop.b = m_colorDirectionalAmbient_Restore[0].b * fDarken + m_colorSunlight_Restore.b * 0.333f * fDarken;
            colorTop.a = 1.0f;

            RedColorValue colorBottom;
            colorBottom.r = m_colorDirectionalAmbient_Restore[1].r * fDarken + m_colorSunlight_Restore.r * 0.333f * fDarken;
            colorBottom.g = m_colorDirectionalAmbient_Restore[1].g * fDarken + m_colorSunlight_Restore.g * 0.333f * fDarken;
            colorBottom.b = m_colorDirectionalAmbient_Restore[1].b * fDarken + m_colorSunlight_Restore.b * 0.333f * fDarken;
            colorBottom.a = 1.0f;

            float fScale = 1.0f - fLightning;
            RedColorValue colorSunlight;
            colorSunlight.r = m_colorLightning.r * fScale;
            colorSunlight.g = m_colorLightning.g * fScale;
            colorSunlight.b = m_colorLightning.b * fScale;
            colorSunlight.a = m_colorLightning.a * fScale;

            RedLight::SetGlobalAmbientLightColor(colorAmbient);
            RedLight::SetGlobalDirectionalAmbientColors(colorTop, colorBottom);
            RedLight::SetGlobalDirectionalLightColor(colorSunlight, 0);
            pcTerrainShader::_SetLightningValue(fLightning * 0.15f);
        }
        else
        {
            float fScale = 1.0f - (fSunlight - fBrightness) / m_fSunlightBrightness;
            float fDarken = 1.0f - m_fSunlightFadeMax;

            RedColorValue colorSunlightAdd;
            colorSunlightAdd.r = m_colorSunlight_Restore.r * 0.333f * fScale;
            colorSunlightAdd.g = m_colorSunlight_Restore.g * 0.333f * fScale;
            colorSunlightAdd.b = m_colorSunlight_Restore.b * 0.333f * fScale;
            colorSunlightAdd.a = m_colorSunlight_Restore.a * 0.333f * fScale;

            RedColorValue colorAmbient;
            colorAmbient.r = m_colorAmbient_Restore.r * fDarken + fDarken * colorSunlightAdd.r;
            colorAmbient.g = m_colorAmbient_Restore.g * fDarken + fDarken * colorSunlightAdd.g;
            colorAmbient.b = m_colorAmbient_Restore.b * fDarken + fDarken * colorSunlightAdd.b;
            colorAmbient.a = m_colorAmbient_Restore.a * fDarken + fDarken * colorSunlightAdd.a;

            RedColorValue colorTop;
            colorTop.r = m_colorDirectionalAmbient_Restore[0].r * fDarken + fDarken * colorSunlightAdd.r;
            colorTop.g = m_colorDirectionalAmbient_Restore[0].g * fDarken + fDarken * colorSunlightAdd.g;
            colorTop.b = m_colorDirectionalAmbient_Restore[0].b * fDarken + fDarken * colorSunlightAdd.b;
            colorTop.a = m_colorDirectionalAmbient_Restore[0].a * fDarken + fDarken * colorSunlightAdd.a;

            RedColorValue colorBottom;
            colorBottom.r = m_colorDirectionalAmbient_Restore[1].r * fDarken + fDarken * colorSunlightAdd.r;
            colorBottom.g = m_colorDirectionalAmbient_Restore[1].g * fDarken + fDarken * colorSunlightAdd.g;
            colorBottom.b = m_colorDirectionalAmbient_Restore[1].b * fDarken + fDarken * colorSunlightAdd.b;
            colorBottom.a = m_colorDirectionalAmbient_Restore[1].a * fDarken + fDarken * colorSunlightAdd.a;

            RedColorValue colorSunlight;
            colorSunlight.r = m_colorSunlight_Restore.r * fScale;
            colorSunlight.g = m_colorSunlight_Restore.g * fScale;
            colorSunlight.b = m_colorSunlight_Restore.b * fScale;
            colorSunlight.a = m_colorSunlight_Restore.a * fScale;

            RedLight::SetGlobalAmbientLightColor(colorAmbient);
            RedLight::SetGlobalDirectionalAmbientColors(colorTop, colorBottom);
            RedLight::SetGlobalDirectionalLightColor(colorSunlight, 0);
            pcTerrainShader::_SetLightningValue(0.0f);
        }

        SkyDome::sInstance->SetSkyDomeColor(
            Interpolate(RedColor::WHITE, RedColor::BLACK, m_fSkyDomeDarken * (1.0f - fBrightness)));
        RedRenderer::SetFogColor(
            Interpolate(RedColor::WHITE, RedColor::BLACK, m_fSkyDomeDarken * (1.0f - fBrightness)));
    }
    else if (m_fTimer - fDeltaTime < m_fTimeToFade)
    {
        RedLight::SetGlobalDirectionalLightColor(m_colorSunlight_Restore, 0);
        RedLight::SetGlobalAmbientLightColor(m_colorAmbient_Restore);
        RedLight::SetGlobalDirectionalAmbientColors(m_colorDirectionalAmbient_Restore[0],
                                                    m_colorDirectionalAmbient_Restore[1]);
        SkyDome::sInstance->SetSkyDomeColor(Interpolate(RedColor::WHITE, RedColor::BLACK, m_fSkyDomeDarken));
        RedRenderer::SetFogColor(m_colorFog_Restore);
        pcTerrainShader::_SetLightningValue(0.0f);
    }

    mBoltTimer -= fDeltaTime;
    if (mBoltTimer < 0.0f)
    {
        FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(0x559A2771u);
        if (pClass)
        {
            FLEffectObject* pEffect = pClass->BuildEffect();
            if (pEffect)
            {
                const PblMatrix& mCamera = CameraManager::sInstance->GetCamera(0)->GetMatrix();
                PblVector3 vDir;
                vDir.x = PblRandom::GetGlobalFloat() - 0.5f;
                vDir.z = PblRandom::GetGlobalFloat() - 0.5f;
                vDir.y = 0.01f;
                vDir.x -= mCamera.m[2][0];
                vDir.z -= mCamera.m[2][2];
                PblVector3 vNormal;
                D3DXVec3Normalize(&vNormal, &vDir);

                PblVector3 vPos;
                vPos.x = mCamera.m[3][0] + vNormal.x * fBoltDist;
                vPos.y = fBoltHeight;
                vPos.z = mCamera.m[3][2] + vNormal.z * fBoltDist;
                pEffect->AttachEffectToVector(&vPos, false);

                float fOffsetX = (PblRandom::GetGlobalFloat() - 0.5f) * 20.0f;
                float fOffsetZ = (PblRandom::GetGlobalFloat() - 0.5f) * 20.0f;
                LightningBoltEffectObject* pBolt = static_cast<LightningBoltEffectObject*>(pEffect);
                pBolt->mTargetPos.x = vPos.x + fOffsetX;
                pBolt->mTargetPos.y = vPos.y - 200.0f;
                pBolt->mTargetPos.z = vPos.z + fOffsetZ;
            }
        }
        mBoltTimer = (mBoltFrequencyMax - mBoltFrequencyMin) * PblRandom::GetGlobalFloat() + mBoltFrequencyMin;
    }
}
