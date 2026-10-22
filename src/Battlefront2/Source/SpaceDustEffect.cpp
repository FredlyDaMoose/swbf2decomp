#include <math.h>

#include "SpaceDustEffect.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

typedef SpaceDustEffect::DustEmitterData DustEmitterData;

// FUNCTION: SWBF2 0x0064cef0
SpaceDustEffect::DustEmitterData::DustEmitterData()
{
    mEnabled = false;
    mTimeSinceLastSpawned = 0.0f;
    mParticleLife = 30000.0f;
    mMaxParticles = 80.0f;
    mParticle = 0;
}

// FUNCTION: SWBF2 0x0064cf20
SpaceDustEffect::DustEmitterData::~DustEmitterData()
{
    if (mParticle)
    {
        delete[] mParticle;
    }
}

// GLOBAL: SWBF2 0x01f6dea0
// SYNTHETIC: SWBF2 0x004063d0
// `dynamic initializer for 'SpaceDustEffect::m_emitter''
// SYNTHETIC: SWBF2 0x007694d0
// `dynamic atexit destructor for 'SpaceDustEffect::m_emitter''
SpaceDustEffect::DustEmitterData SpaceDustEffect::m_emitter[1];

// GLOBAL: SWBF2 0x01eafb30
SpaceDustEffect* SpaceDustEffect::s_pInstance;

// GLOBAL: SWBF2 0x007e9a40
float SpaceDustEffect::mSpawnDistance = 150.0f;
// GLOBAL: SWBF2 0x007e9438
float SpaceDustEffect::mMaxRandomSideOffset = 70.0f;
// GLOBAL: SWBF2 0x007e9a48
float SpaceDustEffect::mCenterDeadZoneRadius = 40.0f;
// GLOBAL: SWBF2 0x007e9a54
float SpaceDustEffect::mSpawnDelay = 0.03f;
// GLOBAL: SWBF2 0x007e9a5c
float SpaceDustEffect::mReferenceSpeed = 70.0f;
// GLOBAL: SWBF2 0x007e943c
float SpaceDustEffect::mDustParticleSpeed = 50.0f;

// GLOBAL: SWBF2 0x007e9a4c
unsigned int SpaceDustEffect::mTextureHash = 0x72F9433Cu;
// GLOBAL: SWBF2 0x007e9434
float SpaceDustEffect::mMinParticleScale = 0.5f;
// GLOBAL: SWBF2 0x007e9a44
float SpaceDustEffect::mMaxParticleScale = 1.0f;
// GLOBAL: SWBF2 0x007e9a18
float SpaceDustEffect::mSpeedParticleMinLength = 2.0f;
// GLOBAL: SWBF2 0x007e9a1c
float SpaceDustEffect::mSpeedParticleMaxLength = 6.0f;
// GLOBAL: SWBF2 0x007e9a50
float SpaceDustEffect::mParticleLengthMinSpeed = 70.0f;
// GLOBAL: SWBF2 0x007e9a58
float SpaceDustEffect::mParticleLengthMaxSpeed = 160.0f;

// FUNCTION: SWBF2 0x0064cee0
bool SpaceDustEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x0064cf30
void SpaceDustEffect::DustEmitterData::Init(bool bFreeParticles, bool bAllocParticles)
{
    mEnabled = false;
    mTimeSinceLastSpawned = 0.0f;
    if (bFreeParticles && mParticle)
    {
        delete[] mParticle;
        mParticle = 0;
    }
    if (bAllocParticles && !mParticle)
    {
        mParticle = new ParticleData[80];
    }
    if (mParticle)
    {
        for (int i = 0; i < 80; i++)
        {
            mParticle[i].mLifeLeft = -1.0f;
        }
    }
}

// FUNCTION: SWBF2 0x0064d040
void SpaceDustEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new SpaceDustEffect();
    }
}

// FUNCTION: SWBF2 0x0064d0b0
void SpaceDustEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x0064d0d0
SpaceDustEffect::SpaceDustEffect()
    : FLEffect(0x3757A565u, 3)
{
    Enable(false);
}

// SYNTHETIC: SWBF2 0x0064d140
// SpaceDustEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0064d170
SpaceDustEffect::~SpaceDustEffect()
{
}

// FUNCTION: SWBF2 0x0064d1c0
void SpaceDustEffect::Init()
{
    Init(false);
}

// FUNCTION: SWBF2 0x0064d1d0
void SpaceDustEffect::Init(bool bAllocate)
{
    DustEmitterData& e = SpaceDustEffect::m_emitter[0];
    e.mEnabled = false;
    e.mTimeSinceLastSpawned = 0.0f;
    if (!bAllocate)
    {
        if (e.mParticle)
        {
            delete[] e.mParticle;
            e.mParticle = 0;
        }
    }
    else if (!e.mParticle)
    {
        e.mParticle = new SpaceDustEffect::ParticleData[80];
    }
    if (e.mParticle)
    {
        for (int i = 0; i < 80; i++)
        {
            e.mParticle[i].mLifeLeft = -1.0f;
        }
    }
}

// FUNCTION: SWBF2 0x0064d300
void SpaceDustEffect::Cleanup()
{
    Init();
}

// FUNCTION: SWBF2 0x0064d310
void SpaceDustEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0xAF8BB8CEu:
        {
            bool bEnable = data.GetUint32Arg(0) != 0;
            Enable(bEnable);
            Init(bEnable);
            break;
        }
        case 0x5C980BB3u:
            mSpawnDistance = data.GetFloatArg(0);
            break;
        case 0x0DC9368Au:
            mMaxRandomSideOffset = data.GetFloatArg(0);
            break;
        case 0x1C5D2DDAu:
            mCenterDeadZoneRadius = data.GetFloatArg(0);
            break;
        case 0xA4C9A8D7u:
            mSpawnDelay = data.GetFloatArg(0);
            break;
        case 0x23B8373Du:
            mReferenceSpeed = data.GetFloatArg(0);
            break;
        case 0x0E0C9EFAu:
            mDustParticleSpeed = data.GetFloatArg(0);
            break;
        case 0x3C6468F4u:
            mTextureHash = PblHash(data.GetStringArg(0));
            break;
        case 0x7EFE380Du:
            mMinParticleScale = data.GetFloatArg(0);
            break;
        case 0xDE77E423u:
            mMaxParticleScale = data.GetFloatArg(0);
            break;
        case 0xE26BA52Au:
            mSpeedParticleMinLength = data.GetFloatArg(0);
            break;
        case 0x006E0A20u:
            mSpeedParticleMaxLength = data.GetFloatArg(0);
            break;
        case 0xD9F25D54u:
            mParticleLengthMinSpeed = data.GetFloatArg(0);
            break;
        case 0x7BF23692u:
            mParticleLengthMaxSpeed = data.GetFloatArg(0);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0064d510
void SpaceDustEffect::Start(int iEmitter)
{
    if (iEmitter >= 0 && !m_emitter[iEmitter].mEnabled)
    {
        m_emitter[iEmitter].Init(false, true);
        m_emitter[iEmitter].mEnabled = true;
    }
}

// FUNCTION: SWBF2 0x0064d540
void SpaceDustEffect::Stop(int iEmitter)
{
    if (iEmitter >= 0 && m_emitter[iEmitter].mEnabled)
    {
        m_emitter[iEmitter].Init(false, false);
    }
}

// FUNCTION: SWBF2 0x0064d570
void SpaceDustEffect::Update(float fDeltaTime)
{
    DustEmitterData& emitter = m_emitter[0];
    if (!emitter.mEnabled || !emitter.mParticle)
    {
        return;
    }

    int iNumFree = 0;
    int iNumAlive = 0;
    int aiFree[20];
    const PblMatrix& mCamera = CameraManager::sInstance->mRedCamera[0]->GetMatrix();
    emitter.mTimeSinceLastSpawned += fDeltaTime;

    for (int i = 0; i < 80; i++)
    {
        ParticleData& particle = emitter.mParticle[i];
        if (particle.mLifeLeft >= 0.0f)
        {
            particle.mLifeLeft -= fDeltaTime;
        }
        if (particle.mLifeLeft >= 0.0f)
        {
            const float fDot = -mCamera.m[2][0] * (particle.mCoord.x - mCamera.m[3][0]) +
                               -mCamera.m[2][1] * (particle.mCoord.y - mCamera.m[3][1]) +
                               -mCamera.m[2][2] * (particle.mCoord.z - mCamera.m[3][2]);
            if (fDot < 0.0f)
            {
                particle.mLifeLeft = -1.0f;
            }
        }
        if (particle.mLifeLeft >= 0.0f)
        {
            ++iNumAlive;
            const float fMove = mDustParticleSpeed * fDeltaTime;
            particle.mCoord.x += mCamera.m[2][0] * fMove;
            particle.mCoord.y += mCamera.m[2][1] * fMove;
            particle.mCoord.z += mCamera.m[2][2] * fMove;
        }
        else if (iNumFree < 20)
        {
            aiFree[iNumFree++] = i;
        }
    }

    PblVector3 vDir;
    float fSpeed = 0.0f;
    ChaseCamera* pChaseCamera = CameraManager::sInstance->mChaseCamera[0];
    if (pChaseCamera)
    {
        PblVector3 vVelocity = pChaseCamera->GetVelocity();
        const float fLengthSq = vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y +
                                vVelocity.z * vVelocity.z;
        if (fLengthSq < 0.0001f)
        {
            vDir = PblVector3(-mCamera.m[2][0], -mCamera.m[2][1], -mCamera.m[2][2]);
        }
        else
        {
            fSpeed = sqrtf(fLengthSq);
            D3DXVec3Normalize(&vDir, &vVelocity);
        }
    }
    else
    {
        vDir = PblVector3(-mCamera.m[2][0], -mCamera.m[2][1], -mCamera.m[2][2]);
    }

    float fSpawnDelay;
    if (fSpeed != 0.0f)
    {
        fSpawnDelay = mReferenceSpeed * mSpawnDelay / fSpeed;
    }
    else
    {
        fSpawnDelay = 99999.0f;
        emitter.mTimeSinceLastSpawned = 0.0f;
    }

    if (emitter.mTimeSinceLastSpawned > fSpawnDelay && emitter.mMaxParticles > (float)iNumAlive &&
        iNumFree > 0)
    {
        int iNumSpawn = (int)(emitter.mTimeSinceLastSpawned / fSpawnDelay);
        emitter.mTimeSinceLastSpawned -= (float)iNumSpawn * fSpawnDelay;
        if (iNumSpawn > iNumFree)
        {
            iNumSpawn = iNumFree;
        }

        const float fBackX = -mCamera.m[2][0];
        const float fBackY = -mCamera.m[2][1];
        const float fBackZ = -mCamera.m[2][2];
        PblVector3 vSide(fBackY * vDir.z - fBackZ * vDir.y, fBackZ * vDir.x - vDir.z * fBackX,
                         vDir.y * fBackX - fBackY * vDir.x);
        PblVector3 vRight;
        if (vSide.x * vSide.x + vSide.y * vSide.y + vSide.z * vSide.z < 0.0001f)
        {
            vRight = PblVector3(mCamera.m[1][0], mCamera.m[1][1], mCamera.m[1][2]);
        }
        else
        {
            D3DXVec3Normalize(&vRight, &vSide);
        }

        PblVector3 vUp(vDir.z * vRight.y - vDir.y * vRight.z, vDir.x * vRight.z - vDir.z * vRight.x,
                       vDir.y * vRight.x - vDir.x * vRight.y);
        PblVector3 vUpNorm;
        D3DXVec3Normalize(&vUpNorm, &vUp);
        PblMatrix mSpawn;
        mSpawn.m[0][0] = vUpNorm.x;
        mSpawn.m[0][1] = vUpNorm.y;
        mSpawn.m[0][2] = vUpNorm.z;
        mSpawn.m[0][3] = 0.0f;
        mSpawn.m[1][0] = vRight.x;
        mSpawn.m[1][1] = vRight.y;
        mSpawn.m[1][2] = vRight.z;
        mSpawn.m[1][3] = 0.0f;
        mSpawn.m[2][0] = vDir.x;
        mSpawn.m[2][1] = vDir.y;
        mSpawn.m[2][2] = vDir.z;
        mSpawn.m[2][3] = 0.0f;
        mSpawn.m[3][0] = vDir.x * mSpawnDistance + mCamera.m[3][0];
        mSpawn.m[3][1] = mCamera.m[3][1] + vDir.y * mSpawnDistance;
        mSpawn.m[3][2] = mCamera.m[3][2] + vDir.z * mSpawnDistance;
        mSpawn.m[3][3] = 1.0f;

        for (int i = 0; i < iNumSpawn; i++)
        {
            ParticleData& particle = emitter.mParticle[aiFree[i]];
            particle.mLifeLeft = emitter.mParticleLife;
            particle.mScale = 0.5f;

            const float fX = PblRandom::GetGlobalFloat();
            const float fY = PblRandom::GetGlobalFloat();
            PblVector3 vOffset((1.0f - fX) * mMaxRandomSideOffset - fX * mMaxRandomSideOffset,
                               (1.0f - fY) * mMaxRandomSideOffset - fY * mMaxRandomSideOffset, 0.0f);
            if (mCenterDeadZoneRadius > fabs(vOffset.x) && mCenterDeadZoneRadius > fabs(vOffset.y))
            {
                if (vOffset.x < 0.0f)
                {
                    vOffset.x -= mCenterDeadZoneRadius;
                }
                else
                {
                    vOffset.x += mCenterDeadZoneRadius;
                }
                if (vOffset.y < 0.0f)
                {
                    vOffset.y -= mCenterDeadZoneRadius;
                }
                else
                {
                    vOffset.y += mCenterDeadZoneRadius;
                }
            }
            D3DXVec3TransformCoord(&particle.mCoord, &vOffset, &mSpawn);

            const float fScale = PblRandom::GetGlobalFloat();
            particle.mScale = (1.0f - fScale) * mMinParticleScale + fScale * mMaxParticleScale;
        }
    }
}

// FUNCTION: SWBF2 0x0064dc60
void SpaceDustEffect::Render(int value)
{
    if (value & 0x20000)
    {
        return;
    }

    const RedCamera* pCamera = RedRenderer::GetCamera();
    int iEmitter;
    for (iEmitter = 0; iEmitter < 1; iEmitter++)
    {
        if (CameraManager::sInstance->mRedCamera[iEmitter] == pCamera && m_emitter[iEmitter].mEnabled &&
            m_emitter[iEmitter].mParticle)
        {
            break;
        }
    }
    if (iEmitter >= 1)
    {
        return;
    }
    const DustEmitterData& emitter = m_emitter[iEmitter];

    const PblMatrix& mCamera = pCamera->GetMatrix();
    const PblVector3 vPos(mCamera.m[3][0], mCamera.m[3][1], mCamera.m[3][2]);
    const PblVector3 vFwd(mCamera.m[2][0], mCamera.m[2][1], mCamera.m[2][2]);
    const PblVector3 vMax(vPos.x + 300.0f, vPos.y + 300.0f, vPos.z + 300.0f);
    const PblVector3 vMin(vPos.x - 300.0f, vPos.y - 300.0f, vPos.z - 300.0f);
    RedParticleRenderer::SubmitBounds(vMin, vMax);
    RedParticleRenderer::SubmitTexture(mTextureHash, BLEND_NORMAL, 0);

    float fSpeed = 0.0f;
    ChaseCamera* pChaseCamera = CameraManager::sInstance->mChaseCamera[iEmitter];
    if (pChaseCamera)
    {
        PblVector3 vVelocity = pChaseCamera->GetVelocity();
        const float fLengthSq = vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y +
                                vVelocity.z * vVelocity.z;
        if (fLengthSq >= 0.0001f)
        {
            fSpeed = sqrtf(fLengthSq);
        }
    }
    float fT = (fSpeed - mParticleLengthMinSpeed) / (mParticleLengthMaxSpeed - mParticleLengthMinSpeed);
    if (fT < 0.0f)
    {
        fT = 0.0f;
    }
    else if (fT > 1.0f)
    {
        fT = 1.0f;
    }
    mTextureHash = 0x72F9433Cu;
    const float fLength = (1.0f - fT) * mSpeedParticleMinLength + mSpeedParticleMaxLength * fT;

    RedParticleRenderer::TexCoords texCoords;
    texCoords.u0 = 0;
    texCoords.v0 = 0;
    texCoords.u1 = 0xff;
    texCoords.v1 = 0xff;
    for (int i = 0; i < 80; i++)
    {
        const ParticleData& particle = emitter.mParticle[i];
        if (particle.mLifeLeft < 0.0f)
        {
            continue;
        }
        RedColor color = RedColor::WHITE;
        float fFade = ((particle.mCoord.x - vPos.x) * -vFwd.x + (particle.mCoord.y - vPos.y) * -vFwd.y +
                       (particle.mCoord.z - vPos.z) * -vFwd.z - 50.0f) / 30.0f;
        if (fFade < 0.0f)
        {
            fFade = 0.0f;
        }
        else if (fFade > 1.0f)
        {
            fFade = 1.0f;
        }
        float fAlpha = (1.0f - fFade) + fFade * 255.0f;
        if (fAlpha < 0.0f)
        {
            fAlpha = 0.0f;
        }
        else if (fAlpha > 255.0f)
        {
            fAlpha = 255.0f;
        }
        color.a = (unsigned char)(int)fAlpha;
        RedParticleRenderer::SubmitParticle(PIT_LASER_START, particle.mCoord, color, particle.mScale * 0.5f,
                                            0.0f, texCoords, gVectorUnitX, gVectorUnitY);
        const PblVector3 vTail(particle.mCoord.x - vFwd.x * fLength, particle.mCoord.y - vFwd.y * fLength,
                               particle.mCoord.z - vFwd.z * fLength);
        RedParticleRenderer::SubmitParticle(PIT_LASER_END, vTail, color, particle.mScale * 0.5f, 0.0f,
                                            texCoords, gVectorUnitX, gVectorUnitY);
    }
}
