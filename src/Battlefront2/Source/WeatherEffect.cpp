#include "WeatherEffect.h"
#include "GameState.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"

#include <math.h>

// FUNCTION: SWBF2 0x00694f90
void PrecipitationEffect::RainShadowRegion::UpdateVisibility(const PblBox& box)
{
    PblVector3 vCenter((box._vMin.x + box._vMax.x) * 0.5f,
                       (box._vMin.y + box._vMax.y) * 0.5f,
                       (box._vMin.z + box._vMax.z) * 0.5f);
    PblVector3 vExtents(box._vMax.x - vCenter.x,
                        box._vMax.y - vCenter.y,
                        box._vMax.z - vCenter.z);
    m_bVisible = DoesAABoxIntersect(vCenter, vExtents);
}

// GLOBAL: SWBF2 0x01eafd74
PrecipitationEffect* PrecipitationEffect::s_pInstance;

// GLOBAL: SWBF2 0x01eafd78
FogCloudEffect* FogCloudEffect::s_pInstance;

// GLOBAL: SWBF2 0x01eafd7c
WindEffect* WindEffect::s_pInstance;

// FUNCTION: SWBF2 0x00692cb0
void WindEffect::Render(int)
{
}

// FUNCTION: SWBF2 0x00692cc0
void WindEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new WindEffect();
    }
}

// FUNCTION: SWBF2 0x00692d30
void WindEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x00692d50
WindEffect::WindEffect()
    : FLEffect(0x1414F107u, 3)
{
    Enable(false);
    m_vBaseVelocity = PblVector3(0, 0, 0);
    m_vVelocityTarget = PblVector3(0, 0, 0);
    m_vVelocityPrevTarget = PblVector3(0, 0, 0);
    m_vVelocity = PblVector3(0, 0, 0);
    m_fVelocityRange = 0.0f;
    m_fVelocityChangeRate = 0.0f;
    m_fTimer = 0.0f;
}

// SYNTHETIC: SWBF2 0x00692e10
// WindEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00692e40
WindEffect::~WindEffect()
{
}

// FUNCTION: SWBF2 0x00692e90
void WindEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x32741C32u:
            m_vBaseVelocity = PblVector3(data.GetFloatArg(0), 0.0f, data.GetFloatArg(1));
            break;
        case 0x5469303Fu:
            m_fVelocityRange = data.GetFloatArg(0);
            break;
        case 0xECEC52A4u:
            m_fVelocityChangeRate = data.GetFloatArg(0);
            break;
        case 0xAF8BB8CEu:
            Enable(data.GetUint32Arg(0) != 0);
            break;
        }
    }
    m_vVelocityPrevTarget = m_vVelocityTarget = m_vBaseVelocity;
    m_vVelocity = m_vBaseVelocity;
    m_fTimer = 0.0f;
}

// FUNCTION: SWBF2 0x00692fa0
void WindEffect::Update(float fDeltaTime)
{
    m_fTimer += m_fVelocityChangeRate * fDeltaTime;
    if (m_fTimer < 1.0f)
    {
        m_vVelocity.x = (m_vVelocityTarget.x - m_vVelocityPrevTarget.x) * m_fTimer + m_vVelocityPrevTarget.x;
        m_vVelocity.y = (m_vVelocityTarget.y - m_vVelocityPrevTarget.y) * m_fTimer + m_vVelocityPrevTarget.y;
        m_vVelocity.z = (m_vVelocityTarget.z - m_vVelocityPrevTarget.z) * m_fTimer + m_vVelocityPrevTarget.z;
        return;
    }

    m_vVelocityPrevTarget = m_vVelocityTarget;
    m_vVelocity = m_vVelocityTarget;
    if (m_fTimer > 1.5f)
    {
        float fLength = sqrtf(m_vBaseVelocity.x * m_vBaseVelocity.x +
                              m_vBaseVelocity.y * m_vBaseVelocity.y +
                              m_vBaseVelocity.z * m_vBaseVelocity.z);
        if (fLength <= m_fVelocityRange * 0.01f)
        {
            float fRandX = PblRandom::GetGlobalFloat() * 2.0f - 1.0f;
            float fRandZ = PblRandom::GetGlobalFloat() * 2.0f - 1.0f;
            m_vVelocityTarget.x = m_vBaseVelocity.x + fRandX * m_fVelocityRange;
            m_vVelocityTarget.y = m_vBaseVelocity.y + m_fVelocityRange * 0.0f;
            m_vVelocityTarget.z = m_vBaseVelocity.z + fRandZ * m_fVelocityRange;
        }
        else
        {
            float fScale = m_fVelocityRange / fLength;
            PblVector3 vAlong(m_vBaseVelocity.x * fScale,
                              m_vBaseVelocity.y * fScale,
                              m_vBaseVelocity.z * fScale);
            float fSide = m_fVelocityRange * m_fVelocityRange / (m_fVelocityRange + fLength) / fLength;
            PblVector3 vSide(m_vBaseVelocity.z * fSide, fSide * 0.0f, -m_vBaseVelocity.x * fSide);

            float fRandSide = PblRandom::GetGlobalFloat() * 2.0f - 1.0f;
            vSide.x *= fRandSide;
            vSide.y *= fRandSide;
            vSide.z *= fRandSide;
            float fRandAlong = PblRandom::GetGlobalFloat() * 2.0f - 1.0f;
            m_vVelocityTarget.x = m_vBaseVelocity.x + fRandAlong * vAlong.x + vSide.x;
            m_vVelocityTarget.y = m_vBaseVelocity.y + vAlong.y * fRandAlong + vSide.y;
            m_vVelocityTarget.z = m_vBaseVelocity.z + vAlong.z * fRandAlong + vSide.z;
        }
        m_fTimer = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00693240
void FogCloudEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new FogCloudEffect();
    }
}

// FUNCTION: SWBF2 0x006932b0
void FogCloudEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x006932d0
FogCloudEffect::FogCloudEffect()
    : FLEffect(0xC211031Cu, 3)
{
    Enable(false);
    VELOCITY = PblVector3(0, 0, 0);
    COLOR.b = 0xC0;
    COLOR.g = 0xC0;
    COLOR.r = 0xC0;
    COLOR.a = 0xA0;
    TEXTURE_HASH = 0xA7C73459u;
    ROTATION_RATE = 0.1f;
    BOX_SIZE = 256.0f;
    BOX_SIZEINV = 1.0f / 256.0f;
    PARTICLES_PER_BOX = 30;
    PARTICLE_SIZE = 28.0f;
    HEIGHT_ABOVE_GROUND = 16.0f;
    DISTANCE_NEAR = 40.0f;
    DISTANCE_FAR = 90.0f;
    m_pShader = 0;
}

// SYNTHETIC: SWBF2 0x006933b0
// FogCloudEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006933e0
FogCloudEffect::~FogCloudEffect()
{
}

// FUNCTION: SWBF2 0x00693430
void FogCloudEffect::Init()
{
    _TimeStamp.Mark(&GameState::_TimeBase);
}

// FUNCTION: SWBF2 0x00693450
void FogCloudEffect::Cleanup()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x00693460
void FogCloudEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3C6468F4u:
            SetTexture(PblHash(data.GetStringArg(0)));
            break;
        case 0x21AC415Fu:
            SetRotationRate(data.GetFloatArg(0));
            break;
        case 0x32741C32u:
            SetVelocity(PblVector3(data.GetFloatArg(0), 0.0f, data.GetFloatArg(1)));
            break;
        case 0xC393FFF1u:
            SetParticleDensity(data.GetUint32Arg(0));
            break;
        case 0x55E41B16u:
            SetParticleSize(data.GetFloatArg(0));
            break;
        case 0xD5BDBB42u:
            SetParticleHeight(data.GetFloatArg(0));
            break;
        case 0xFADC0CD2u:
            SetRange(data.GetFloatArg(0), data.GetFloatArg(1));
            break;
        case 0x3D7E6258u:
        {
            RedColor color;
            float f = data.GetFloatArg(0);
            if (f < 0.0f)
            {
                f = 0.0f;
            }
            else if (f > 255.0f)
            {
                f = 255.0f;
            }
            color.r = (unsigned char)(int)f;
            f = data.GetFloatArg(1);
            if (f < 0.0f)
            {
                f = 0.0f;
            }
            else if (f > 255.0f)
            {
                f = 255.0f;
            }
            color.g = (unsigned char)(int)f;
            f = data.GetFloatArg(2);
            if (f < 0.0f)
            {
                f = 0.0f;
            }
            else if (f > 255.0f)
            {
                f = 255.0f;
            }
            color.b = (unsigned char)(int)f;
            if (data.GetNumArgs() > 3)
            {
                f = data.GetFloatArg(3);
                if (f < 0.0f)
                {
                    f = 0.0f;
                }
                else if (f > 255.0f)
                {
                    f = 255.0f;
                }
                color.a = (unsigned char)(int)f;
            }
            else
            {
                color.a = 0xFF;
            }
            SetColor(color);
            break;
        }
        case 0xAF8BB8CEu:
            Enable(data.GetUint32Arg(0) != 0);
            break;
        }
    }
    PlatformInit();
}

// FUNCTION: SWBF2 0x00693690
void PrecipitationEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new PrecipitationEffect();
    }
}

// FUNCTION: SWBF2 0x00693720
PrecipitationEffect::PrecipitationEffect()
    : FLEffect(0x3F5E4CD0u, 3)
    , RedRegionFactory("rainshadow")
{
    Enable(false);
    COLOR.b = 0xC0;
    COLOR.g = 0xC0;
    COLOR.r = 0xC0;
    COLOR.a = 0xA0;
    BOX_SIZE = 5.0f;
    BOX_SIZEINV = 0.2f;
    RENDER_DISTANCE = 10.0f;
    TEXTURE_HASH = 0;
    PARTICLE_SIZE = 0.02f;
    PARTICLE_TYPE = PRECIPITATION_STREAKS;
    VELOCITY = 10.0f;
    VELOCITY_RANGE = 1.0f;
    PARTICLES_PER_BOX = 30.0f;
    PARTICLES_PER_BOX_RANGE = 0.0f;
    ALPHA_MAX = 0.5f;
    ALPHA_MIN = 0.5f;
    STREAK_TIME = 1.0f / 30.0f;
    ROTATION_RATE_MEAN = 0.0f;
    ROTATION_RATE_RANGE = 2.0f;
    MAX_PARTICLES_PER_BOX = 30;
    PROPERTY_DRIFT_RATE = 0.4f;
    m_fDriftTime = 0.0f;
    CAMERA_CROSSVELOCITY_SCALE = 1.0f;
    CAMERA_AXIALVELOCITY_SCALE = 1.0f;
    m_pData = 0;
    mGroundEffect = 0;
    mGroundEffectSpread = 20.0f;
    mNumGroundEffectsPerSec = 10.0f;
    mGroundEffectTimer[0] = 0.0f;
    m_pShader = 0;
}

// FUNCTION: SWBF2 0x00693980
void PrecipitationEffect::Init()
{
}

// FUNCTION: SWBF2 0x00693990
void PrecipitationEffect::Cleanup()
{
    PlatformCleanup();
    delete[] m_pData;
    m_pData = 0;
    m_RainShadows.RemoveAll();
}

RedColor ReadColor(const PblConfig::Data& data);

// FUNCTION: SWBF2 0x006939c0
void PrecipitationEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3C6468F4u:
            TEXTURE_HASH = PblHash(data.GetStringArg(0));
            break;
        case 0x5127F14Du:
            PARTICLE_TYPE = (PblHash(data.GetStringArg(0)) == 0x4A59A280u)
                                ? PRECIPITATION_STREAKS
                                : PRECIPITATION_QUADS;
            break;
        case 0x55E41B16u:
            PARTICLE_SIZE = data.GetFloatArg(0);
            break;
        case 0xFADC0CD2u:
            RENDER_DISTANCE = data.GetFloatArg(0);
            break;
        case 0x3D7E6258u:
            COLOR = ReadColor(data);
            break;
        case 0x220C0C21u:
            ALPHA_MIN = data.GetFloatArg(0);
            ALPHA_MAX = data.GetNumArgs() > 1 ? data.GetFloatArg(1) : data.GetFloatArg(0);
            break;
        case 0xC393FFF1u:
            PARTICLES_PER_BOX = data.GetFloatArg(0);
            break;
        case 0x2214EF7Eu:
            PARTICLES_PER_BOX_RANGE = data.GetFloatArg(0);
            break;
        case 0x32741C32u:
            VELOCITY = data.GetFloatArg(0);
            break;
        case 0x5469303Fu:
            VELOCITY_RANGE = data.GetFloatArg(0);
            break;
        case 0xB16A1ADBu:
            STREAK_TIME = data.GetFloatArg(0) * (1.0f / 30.0f);
            break;
        case 0x8F369C79u:
            CAMERA_CROSSVELOCITY_SCALE = data.GetFloatArg(0);
            break;
        case 0x1C4564CAu:
            CAMERA_AXIALVELOCITY_SCALE = data.GetFloatArg(0);
            break;
        case 0x21AC415Fu:
            ROTATION_RATE_MEAN = data.GetFloatArg(0);
            break;
        case 0xF2617B74u:
            ROTATION_RATE_RANGE = data.GetFloatArg(0);
            break;
        case 0x51689473u:
            mGroundEffect = FLEffect::s_EffectClasses.Find(PblHash(data.GetStringArg(0)));
            break;
        case 0x885AC868u:
            mNumGroundEffectsPerSec =
                data.GetFloatArg(0) / (float)CameraManager::Instance()->GetNumCameras();
            break;
        case 0xD23D97A4u:
            mGroundEffectSpread = data.GetFloatArg(0);
            break;
        case 0xAF8BB8CEu:
            Enable(data.GetUint32Arg(0) != 0);
            break;
        }
    }
    if (!m_pData)
    {
        CreatePrecipitationData();
    }
    PlatformInit();
}

// FUNCTION: SWBF2 0x00693cc0
void PrecipitationEffect::CreatePrecipitationData()
{
    MAX_PARTICLES_PER_BOX = (int)(PARTICLES_PER_BOX + PARTICLES_PER_BOX_RANGE);
    m_pData = new PrecipitationData[MAX_PARTICLES_PER_BOX];
    for (unsigned int i = 0; i < MAX_PARTICLES_PER_BOX; ++i)
    {
        PrecipitationData& data = m_pData[i];
        data.vPosRelative = PblVector3(PblRandom::_Global.GetFloat(),
                                       PblRandom::_Global.GetFloat(),
                                       PblRandom::_Global.GetFloat());
        if (VELOCITY_RANGE > 0.0f)
        {
            const PblVector3 v = PblRandom::GetGlobalVector3();
            data.vVelocityLocal = PblVector3(v.x * VELOCITY_RANGE, v.y * VELOCITY_RANGE,
                                             v.z * VELOCITY_RANGE);
        }
        else
        {
            data.vVelocityLocal = PblVector3(0.0f, 0.0f, 0.0f);
        }
        if (ALPHA_MAX > ALPHA_MIN)
        {
            data.fAlpha = (ALPHA_MAX - ALPHA_MIN) * PblRandom::_Global.GetFloat() + ALPHA_MIN;
        }
        else
        {
            data.fAlpha = ALPHA_MIN;
        }
        if (PARTICLE_TYPE == PRECIPITATION_QUADS)
        {
            if (ROTATION_RATE_RANGE > 0.0f)
            {
                data.fRotationRate =
                    (PblRandom::_Global.GetFloat() - 0.5f) * ROTATION_RATE_RANGE + ROTATION_RATE_MEAN;
            }
            else
            {
                data.fRotationRate = ROTATION_RATE_MEAN;
            }
            data.fRotation = PblRandom::_Global.GetFloat();
        }
        else
        {
            data.fRotationRate = 0.0f;
            data.fRotation = 0.0f;
        }
    }
    m_fParticleDensity = PARTICLES_PER_BOX;
    m_fDriftTime = 0.0f;
}

// FUNCTION: SWBF2 0x00694000
void PrecipitationEffect::Update(float fDeltaTime)
{
    if (!m_pData)
    {
        CreatePrecipitationData();
    }

    m_fDriftTime += PROPERTY_DRIFT_RATE * fDeltaTime;
    if (m_fDriftTime > 0.2f)
    {
        if (m_fDriftTime > 1.0f)
        {
            m_fDriftTime = 1.0f;
        }

        if (PARTICLES_PER_BOX_RANGE > 0.0f)
        {
            const float fTarget =
                ((float)(PblRandom::GetGlobalInt() & 1) * 2.0f - 1.0f) * PARTICLES_PER_BOX_RANGE + PARTICLES_PER_BOX;
            m_fParticleDensity = (fTarget - m_fParticleDensity) * m_fDriftTime + m_fParticleDensity;
        }

        if (VELOCITY_RANGE > 0.0f)
        {
            for (unsigned int i = 0; i < MAX_PARTICLES_PER_BOX; ++i)
            {
                PrecipitationData& data = m_pData[i];

                const PblVector3 vTarget((float)(PblRandom::GetGlobalInt() & 1) * 2.0f - 1.0f,
                                         (float)(PblRandom::GetGlobalInt() & 1) * 2.0f - 1.0f,
                                         (float)(PblRandom::GetGlobalInt() & 1) * 2.0f - 1.0f);
                data.vVelocityLocal.x = (VELOCITY_RANGE * vTarget.x - data.vVelocityLocal.x) * m_fDriftTime
                                      + data.vVelocityLocal.x;
                data.vVelocityLocal.y = (VELOCITY_RANGE * vTarget.y - data.vVelocityLocal.y) * m_fDriftTime
                                      + data.vVelocityLocal.y;
                data.vVelocityLocal.z = (VELOCITY_RANGE * vTarget.z - data.vVelocityLocal.z) * m_fDriftTime
                                      + data.vVelocityLocal.z;
            }
        }

        if (ALPHA_MAX > ALPHA_MIN)
        {
            for (unsigned int i = 0; i < MAX_PARTICLES_PER_BOX; ++i)
            {
                PrecipitationData& data = m_pData[i];
                const float fTarget = (ALPHA_MAX - ALPHA_MIN) * (float)(PblRandom::GetGlobalInt() & 1) + ALPHA_MIN;
                data.fAlpha = (fTarget - data.fAlpha) * m_fDriftTime + data.fAlpha;
            }
        }

        if (PARTICLE_TYPE == PRECIPITATION_QUADS && ROTATION_RATE_RANGE > 0.0f)
        {
            for (unsigned int i = 0; i < MAX_PARTICLES_PER_BOX; ++i)
            {
                PrecipitationData& data = m_pData[i];
                const float fTarget =
                    ((float)(PblRandom::GetGlobalInt() & 1) - 0.5f) * ROTATION_RATE_RANGE + ROTATION_RATE_MEAN;
                data.fRotationRate = (fTarget - data.fRotationRate) * m_fDriftTime + data.fRotationRate;
            }
        }

        m_fDriftTime -= 0.2f;
    }

    const PblVector3 vWind = WindEffect::Instance()->GetVelocity();
    const float fScale = BOX_SIZEINV * fDeltaTime;
    const PblVector3 vOffset(vWind.x * fScale + 1000.0f, -VELOCITY * fScale + 1000.0f,
                             vWind.z * fScale + 1000.0f);
    for (unsigned int i = 0; i < MAX_PARTICLES_PER_BOX; ++i)
    {
        PrecipitationData& data = m_pData[i];
        const float fStep = BOX_SIZEINV * fDeltaTime;
        data.vPosRelative.x += data.vVelocityLocal.x * fStep + vOffset.x;
        data.vPosRelative.y += data.vVelocityLocal.y * fStep + vOffset.y;
        data.vPosRelative.z += data.vVelocityLocal.z * fStep + vOffset.z;
        data.vPosRelative.x -= (float)(int)data.vPosRelative.x;
        data.vPosRelative.y -= (float)(int)data.vPosRelative.y;
        data.vPosRelative.z -= (float)(int)data.vPosRelative.z;
    }

    if (PARTICLE_TYPE == PRECIPITATION_QUADS)
    {
        for (unsigned int i = 0; i < MAX_PARTICLES_PER_BOX; ++i)
        {
            PrecipitationData& data = m_pData[i];
            data.fRotation += data.fRotationRate * fDeltaTime;
            data.fRotation -= (float)(int)data.fRotation;
        }
    }

    if (!mGroundEffect)
    {
        return;
    }

    const float fDensity = mNumGroundEffectsPerSec / (mGroundEffectSpread * mGroundEffectSpread * 0.65f);
    const unsigned int uNumCameras = CameraManager::sInstance->GetNumCameras();

    PblBox aBoxes[_countof(mGroundEffectTimer)];
    unsigned int uCam;
    for (uCam = 0; uCam < uNumCameras; ++uCam)
    {
        const float fReach = mGroundEffectSpread + 3.0f;
        const PblMatrix& mCamera = CameraManager::sInstance->GetCamera(uCam)->GetMatrix();
        PblBox& box = aBoxes[uCam];
        box._vMin = *(const PblVector3*)mCamera.m[3];
        box._vMax = *(const PblVector3*)mCamera.m[3];

        const PblVector3 vCentre(mCamera.m[3][0] - fReach * mCamera.m[2][0],
                                 mCamera.m[3][1] - mCamera.m[2][1] * fReach,
                                 mCamera.m[3][2] - mCamera.m[2][2] * fReach);
        const float fSide = fReach * 0.577f;
        const float fUp = fReach * 0.433f;
        const PblVector3 vSide(fSide * mCamera.m[0][0], mCamera.m[0][1] * fSide, mCamera.m[0][2] * fSide);
        const PblVector3 vUp(fUp * mCamera.m[1][0], mCamera.m[1][1] * fUp, mCamera.m[1][2] * fUp);
        box.AddPoint(PblVector3(vSide.x + vCentre.x + vUp.x, vSide.y + vCentre.y + vUp.y,
                                vSide.z + vCentre.z + vUp.z));
        box.AddPoint(PblVector3(vSide.x + vCentre.x - vUp.x, vSide.y + vCentre.y - vUp.y,
                                vSide.z + vCentre.z - vUp.z));
        box.AddPoint(PblVector3(vCentre.x - vSide.x + vUp.x, vCentre.y - vSide.y + vUp.y,
                                vCentre.z - vSide.z + vUp.z));
        box.AddPoint(PblVector3(vCentre.x - vSide.x - vUp.x, vCentre.y - vSide.y - vUp.y,
                                vCentre.z - vSide.z - vUp.z));

        const float fInterval =
            1.0f / ((box._vMax.x - box._vMin.x) * fDensity * (box._vMax.z - box._vMin.z));
        float& fTimer = mGroundEffectTimer[uCam];
        if (fTimer > fInterval)
        {
            for (PblList<RainShadowRegion>::Itor it = m_RainShadows.Begin(); it != m_RainShadows.End(); ++it)
            {
                it->UpdateVisibility(box);
            }
        }

        const float fSizeX = box._vMax.x - box._vMin.x;
        const float fSizeZ = box._vMax.z - box._vMin.z;
        for (fTimer += fDeltaTime; fTimer > fInterval; fTimer -= fInterval)
        {

            PblVector3 vPos(PblRandom::_Global.GetFloat() * fSizeX + box._vMin.x, mCamera.m[3][1],
                            PblRandom::_Global.GetFloat() * fSizeZ + box._vMin.z);

            unsigned int uOther;
            for (uOther = 0; uOther < uCam; ++uOther)
            {
                const PblBox& other = aBoxes[uOther];
                if (vPos.x > other._vMin.x && other._vMax.x > vPos.x && vPos.z > other._vMin.z &&
                    other._vMax.z > vPos.z &&
                    50.0f > (float)fabs(vPos.y - CameraManager::sInstance->GetCamera(uOther)->GetMatrix().m[3][1]))
                {
                    break;
                }
            }
            if (uOther < uCam)
            {
                continue;
            }

            vPos.y += 50.0f;
            CollisionObject* pHit;
            const float fHit = CollisionManager::RayHit(vPos, PblVector3(0.0f, -1.0f, 0.0f), 100.0f, pHit, 0, 0, 0,
                                                        0x90, true);
            vPos.y -= fHit * 100.0f;

            PblList<RainShadowRegion>::Itor it;
            for (it = m_RainShadows.Begin(); it != m_RainShadows.End(); ++it)
            {
                if (it->IsVisible() && it->DoesSphereIntersect(vPos, 1.0f))
                {
                    break;
                }
            }
            if (it != m_RainShadows.End())
            {
                continue;
            }

            FLEffectObject* pEffect = mGroundEffect->BuildEffect();
            if (!pEffect)
            {
                fTimer = (float)fmod((double)fTimer, (double)fInterval);
                break;
            }
            pEffect->AttachEffectToVector(&vPos, false);
        }
    }
    for (; uCam < _countof(mGroundEffectTimer); ++uCam)
    {
        mGroundEffectTimer[uCam] = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00694d70
void* PrecipitationEffect::CreateRegion(RedRegionDesc& desc, const char*) const
{
    RainShadowRegion* pRegion = new RainShadowRegion(desc);
    const_cast<PrecipitationEffect*>(this)->m_RainShadows.Add(&pRegion->m_RainShadowNode, pRegion);
    return pRegion;
}

// FUNCTION: SWBF2 0x00693700
void PrecipitationEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x006938d0
PrecipitationEffect::~PrecipitationEffect()
{
    for (PblList<RainShadowRegion>::Itor it = m_RainShadows.Begin(); it != m_RainShadows.End(); ++it)
    {
        RainShadowRegion* pRegion = it;
        delete pRegion;
    }
    Cleanup();
}

// SYNTHETIC: SWBF2 0x006938a0
// PrecipitationEffect::`scalar deleting destructor'
