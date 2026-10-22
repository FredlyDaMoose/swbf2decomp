#include "SkyDome.h"
#include "PebbleFL/Common/PblVector.h"

#include "Entity.h"
#include "FLEffect.h"
#include "ParticleEmitterObject.h"
#include "SkyStars.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

#include "RedEngineFL/Application/RedTerrain.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

extern PblVector3 gVectorZero;

typedef void(__fastcall* SceneObjectFn)(void* pThis, void*);

// GLOBAL: SWBF2 0x01eaf064
SkyDome* SkyDome::sInstance;

// FUNCTION: SWBF2 0x006371e0
void SkyDome::Create()
{
    sInstance = new SkyDome();
}

// FUNCTION: SWBF2 0x00637270
SkyDome::SkyDome()
    : Renderable(0)
{
    mForceHiDetail = false;
    Renderable::Activate(-9999);
    Thread::ActivateThread(0, PER_FRAME);
    RedRenderer::SetSky(1);

    mNumDomeModels = 0;
    for (int i = 0; i < 10; ++i)
    {
        DomeModel& model = mDomeModels[i];
        model.mModel = 0;
        model.mOffset = 0.0f;
        model.mMovementScale = 1.0f;
        model.mType = TRACK_CAMERA;
        model.mReflectionFlag = false;
        model.mRotationSpeed = 0.0f;
        model.mRotationAxis = PblVector3(0.0f, 1.0f, 0.0f);
        model.mNumEffectData = 0;
    }

    mTerrainTextureHash = 0;
    mTerrainTextureScale = 1.0f;
    mTerrainDetailTextureHash = 0;
    mTerrainDetailTextureScale = 1.0f;
    mTerrainMaxDist = 2000.0f;
    mTerrainPatchResolution = 5;
    m_colorSkyDome = RedColor::WHITE;
    m_pStars = 0;
    mDomeHeight = 0.0f;

    mReflectionColor.g = 0xff;
    mReflectionColor.r = 0xff;
    mReflectionColor.b = 0xff;
    mReflectionColor.a = 0xff;

    Init();
}

// FUNCTION: SWBF2 0x00637780
SkyDome::DomeModel::DomeModel()
{
}

// FUNCTION: SWBF2 0x006377c0
SkyDome::EffectData::EffectData()
{
    effect.mObject = 0;
    effect.mSavedHandleId = 0;
}

// FUNCTION: SWBF2 0x006378a0
void SkyDome::Read(PblConfig& config)
{
    RedTerrain::mTerrainColorDarkeningFlag = false;
    EntityClass* pCloudClass = 0;
    float fCloudHeight = 0.0f;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3CE6A053u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            while (!scope.IsDone())
            {
                PblConfig::Data cloud = scope.ReadNextData();
                switch (cloud.GetId())
                {
                case 0xAB409A38u:
                    pCloudClass = EntityClass::Find(PblHash(cloud.GetStringArg(0)));
                    break;
                case 0xD5BDBB42u:
                    fCloudHeight = cloud.GetFloatArg(0);
                    break;
                }
            }
            break;
        }
        case 0x4939F3F8u:
            RedRenderer::SetAmbientThreshold(data.GetInt32Arg(0));
            break;
        case 0x8563E50Au:
            RedRenderer::SetAmbientIntensity(data.GetInt32Arg(0));
            break;
        case 0x82681057u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            while (!scope.IsDone())
            {
                PblConfig::Data model = scope.ReadNextData();
                switch (model.GetId())
                {
                case 0x32048FBAu:
                {
                    mDomeModels[mNumDomeModels].mRotationSpeed = model.GetFloatArg(0);
                    PblVector3 vAxis(model.GetFloatArg(1), model.GetFloatArg(2), model.GetFloatArg(3));

                    PblVector3 vNormal;
                    D3DXVec3Normalize(&vNormal, &vAxis);
                    mDomeModels[mNumDomeModels].mRotationAxis = vNormal;
                    break;
                }
                case 0x2A6D8591u:
                    mDomeModels[mNumDomeModels].mModel =
                        const_cast<RedModel*>(RedModel::Find(PblHash(model.GetStringArg(0))));
                    break;
                case 0x14C8D3CAu:
                    mDomeModels[mNumDomeModels].mOffset = model.GetFloatArg(0);
                    mDomeModels[mNumDomeModels].mType = STATIC_HEIGHT;
                    break;
                case 0x6E6E8D54u:
                {
                    const int iEffect = mDomeModels[mNumDomeModels].mNumEffectData;
                    if (iEffect < 20)
                    {
                        mDomeModels[mNumDomeModels].mNumEffectData = iEffect + 1;
                        EffectData* pData = mDomeModels[mNumDomeModels].mEffectData;
                        pData[iEffect].effectHash = PblHash(model.GetStringArg(0));
                        pData[iEffect].hardPoint = PblTEMPHash(model.GetStringArg(1));
                        if (model.GetNumArgs() > 2)
                        {
                            pData[iEffect].fRestartTimeMin = model.GetFloatArg(2);
                            if (pData[iEffect].fRestartTimeMin < 0.0f)
                            {
                                pData[iEffect].fRestartTimeMin = 0.0f;
                            }
                            if (model.GetNumArgs() > 3)
                            {
                                pData[iEffect].fRestartTimeMax = model.GetFloatArg(3);
                                if (pData[iEffect].fRestartTimeMin > pData[iEffect].fRestartTimeMax)
                                {
                                    pData[iEffect].fRestartTimeMax = pData[iEffect].fRestartTimeMin;
                                }
                            }
                            else
                            {
                                pData[iEffect].fRestartTimeMax = pData[iEffect].fRestartTimeMin;
                            }
                        }
                        else
                        {
                            pData[iEffect].fRestartTimeMax = 0.0f;
                            pData[iEffect].fRestartTimeMin = 0.0f;
                        }
                        pData[iEffect].timer =
                            -(PblRandom::GetGlobalFloat() * pData[iEffect].fRestartTimeMax);
                    }
                    break;
                }
                case 0x92C778AAu:
                    mDomeModels[mNumDomeModels].mReflectionFlag = true;
                    break;
                case 0xFD58FBFAu:
                    mDomeModels[mNumDomeModels].mMovementScale = model.GetFloatArg(0);
                    mDomeModels[mNumDomeModels].mType = STATIC_HEIGHT;
                    break;
                }
            }
            ++mNumDomeModels;
            break;
        }
        case 0x622B0026u:
        {
            delete m_pStars;
            m_pStars = new SkyStars();
            PblConfig scope(config.ReadNextScope(), true);
            m_pStars->Read(scope);
            break;
        }
        case 0x8958BCDAu:
            RedRenderer::SetSoftnessEnable(data.GetInt32Arg(0) != 0);
            break;
        case 0x8A81DE8Bu:
            mReflectionColor.r = data.GetUint8Arg(0);
            mReflectionColor.g = data.GetUint8Arg(1);
            mReflectionColor.b = data.GetUint8Arg(2);
            break;
        case 0x9010BF4Au:
            if (data.GetNumArgs() >= 2)
            {
                RedTerrain::SetBumpSpecularTexture(PblHash(data.GetStringArg(0)), data.GetFloatArg(1));
            }
            break;
        case 0xC7E16877u:
            RedRenderer::SetAmbientFilter(data.GetUint32Arg(0) != 0);
            break;
        case 0xDDF0C29Eu:
        {
            PblConfig scope(config.ReadNextScope(), true);
            while (!scope.IsDone())
            {
                PblConfig::Data terrain = scope.ReadNextData();
                switch (terrain.GetId())
                {
                case 0x6021B96Fu:
                    mTerrainDetailTextureScale = terrain.GetFloatArg(0);
                    break;
                case 0x3C6468F4u:
                    mTerrainTextureHash = PblHash(terrain.GetStringArg(0));
                    break;
                case 0x3F2E0EAFu:
                    mTerrainDetailTextureHash = PblHash(terrain.GetStringArg(0));
                    break;
                case 0x41CA1C56u:
                    mForceHiDetail = terrain.GetBoolArg(0);
                    break;
                case 0x6D9B436Eu:
                    mTerrainTextureScale = terrain.GetFloatArg(0);
                    break;
                case 0x6EF738FBu:
                    mTerrainPatchResolution = terrain.GetInt32Arg(0);
                    break;
                case 0xE6915CF6u:
                    mTerrainMaxDist = terrain.GetFloatArg(0);
                    break;
                }
            }
            RedTerrain::SetFarTerrainTexture(mTerrainTextureHash, mTerrainTextureScale,
                                             mTerrainDetailTextureHash, mTerrainDetailTextureScale);
            RedTerrain::SetFarTerrainMaxDistance(mTerrainMaxDist);
            RedTerrain::SetForceHiDetail(mForceHiDetail);
            break;
        }
        case 0xD8A93A22u:
            RedTerrain::mTerrainColorDarkeningFlag = true;
            break;
        case 0xE1BCA028u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            int iColor = 0;
            while (!scope.IsDone())
            {
                PblConfig::Data atmosphere = scope.ReadNextData();
                switch (atmosphere.GetId())
                {
                case 0x3D7E6258u:
                {
                    RedColor color;
                    color.b = atmosphere.GetUint8Arg(2);
                    color.g = atmosphere.GetUint8Arg(1);
                    color.r = atmosphere.GetUint8Arg(0);
                    color.a = atmosphere.GetNumArgs() > 3 ? atmosphere.GetUint8Arg(3) : 0x20;
                    RedRenderer::SetFilterColor(color, iColor++);
                    break;
                }
                case 0x61342FD0u:
                    RedRenderer::SetColorFilter(atmosphere.GetInt32Arg(0) != 0);
                    break;
                }
            }
            break;
        }
        case 0xEEEB3A51u:
            if (data.GetNumArgs() >= 3)
            {
                RedTerrain::SetBumpDetailTexture(PblHash(data.GetStringArg(0)), data.GetFloatArg(1),
                                                 data.GetFloatArg(2));
            }
            else if (data.GetNumArgs() >= 2)
            {
                RedTerrain::SetBumpDetailTexture(PblHash(data.GetStringArg(0)), data.GetFloatArg(1), 0.5f);
            }
            break;
        case 0xFEF82E0Bu:
            RedRenderer::SetSoftnessValue(data.GetInt32Arg(0));
            break;
        }
    }

    if (pCloudClass)
    {
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                EntityDesc desc;
                desc.mMatrix = _IdentityMatrix;

                const float z = ((float)j + PblRandom::_Global.GetFloat() * 0.8f - 4.0f) * 250.0f;
                const float y = PblRandom::_Global.GetFloat() * 35.0f + fCloudHeight;
                const float x = ((float)i + PblRandom::_Global.GetFloat() * 0.8f - 4.0f) * 250.0f;
                desc.mMatrix.m[3][0] = x;
                desc.mMatrix.m[3][1] = y;
                desc.mMatrix.m[3][2] = z;
                desc.mMatrix.m[3][3] = 1.0f;
                pCloudClass->Build(desc);
            }
        }
    }
}

// FUNCTION: SWBF2 0x006384f0
bool SkyDome::Update(float fDeltaTime)
{
    for (int iDome = 0; iDome < mNumDomeModels; ++iDome)
    {
        DomeModel& rDome = mDomeModels[iDome];
        for (int i = 0; i < rDome.mNumEffectData; ++i)
        {
            EffectData& rData = rDome.mEffectData[i];
            rData.timer += fDeltaTime;
            if (rData.effect)
            {
                continue;
            }

            if (rData.timer - fDeltaTime > 0.0f)
            {
                rData.timer = -rData.fRestartTimeMin;
                if (rData.fRestartTimeMax > rData.fRestartTimeMin)
                {
                    rData.timer -= (rData.fRestartTimeMax - rData.fRestartTimeMin) *
                                   PblRandom::_Global.GetFloat();
                }
            }
            else if (rData.timer > 0.0f)
            {
                FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(rData.effectHash);
                if (!pClass)
                {
                    continue;
                }
                rData.effect = static_cast<ParticleEmitterObject*>(pClass->BuildEffect());
                if (!rData.effect)
                {
                    continue;
                }

                rData.pos = gVectorZero;
                rData.effect->AttachEffectToVector(&rData.pos, true);

                unsigned char* pEmitter = (unsigned char*)(ParticleEmitterObject*)rData.effect;
                void* pSceneObject = pEmitter + 0x80;
                static_cast<RedSceneObject*>(rData.effect)->Deactivate();
                pEmitter[0x106] = 0;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00637250
void SkyDome::Destroy()
{
    delete sInstance;
    sInstance = 0;
}

// FUNCTION: SWBF2 0x006377f0
SkyDome::~SkyDome()
{
    if (m_pStars)
    {
        delete m_pStars;
        m_pStars = 0;
    }
    Cleanup();
    Deactivate();
    DeactivateThread();
}
