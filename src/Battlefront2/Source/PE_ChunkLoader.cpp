#include "PE_ChunkLoader.h"

#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Util/ext/types.h"
#include "Battlefront2Lod.h"
#include "CameraManager.h"
#include "ParticleEmitter.h"
#include "ParticleGeometry.h"

extern SmallEffectLodData smallEffectLodData;
extern MediumEffectLodData mediumEffectLodData;
extern LargeEffectLodData largeEffectLodData;

// GLOBAL: SWBF2 0x01eaedf0
ParticleEmitterEffect* ParticleEmitterEffect::s_pInstance;

// FUNCTION: SWBF2 0x00618690
void ParticleEmitterEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new ParticleEmitterEffect();
    }
}

// FUNCTION: SWBF2 0x00618620
ParticleEmitterEffect::~ParticleEmitterEffect()
{
}

// FUNCTION: SWBF2 0x00618610
void ParticleEmitterEffect::Render(int)
{
}

// FUNCTION: SWBF2 0x00618600
FLEffectClass* ParticleEmitterEffect::BuildClass(PblConfig& config) const
{
    return (FLEffectClass*)PE_ChunkLoader::Read(config);
}

// FUNCTION: SWBF2 0x00618740
static void ReadVarianceValueWithArg(VarianceValue& value, PblConfig::Data& data, unsigned int uiArg, float fScale)
{
    float fFirst = data.GetFloatArg(uiArg) * fScale;
    if (uiArg + 1 < data.GetNumArgs())
    {
        value.Setup(fFirst, data.GetFloatArg(uiArg + 1) * fScale);
    }
    else
    {
        value.Setup(fFirst);
    }
}

// FUNCTION: SWBF2 0x006187a0
static void ReadVarianceValue(VarianceValue& value, PblConfig::Data& data, float fScale)
{
    ReadVarianceValueWithArg(value, data, 0, fScale);
}

// FUNCTION: SWBF2 0x006187b0
static void ReadVarianceValue(VarianceValue* pValues, PblConfig::Data& data, float fScale)
{
    ReadVarianceValueWithArg(pValues[data.GetInt32Arg(0)], data, 1, fScale);
}

// FUNCTION: SWBF2 0x00619480
void PE_ChunkLoader::LoadSpreadData(ParticleSpawner::SpreadData& data, PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data line = config.ReadNextData();
        switch (line.GetId())
        {
        case 0x57d87d76u:
            ReadVarianceValue(data.mPositionX, line, 1.0f);
            break;
        case 0x58d87f09u:
            ReadVarianceValue(data.mPositionY, line, 1.0f);
            break;
        case 0x55d87a50u:
            ReadVarianceValue(data.mPositionZ, line, 1.0f);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00619510
void PE_ChunkLoader::LoadRotationData(ParticleSpawner::RotationData& data, PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data line = config.ReadNextData();
        VarianceValue* pValue = 0;
        switch (line.GetId())
        {
        case 0xe9b852e5u: pValue = &data.mStartX; break;
        case 0xf102de8au: pValue = &data.mStartMoveX; break;
        case 0xaa11aa04u: pValue = &data.mMoveX; break;
        case 0xe8b85152u: pValue = &data.mStartY; break;
        case 0xab11ab97u: pValue = &data.mMoveY; break;
        case 0xf202e01du: pValue = &data.mStartMoveY; break;
        case 0xe7b84fbfu: pValue = &data.mStartZ; break;
        case 0xac11ad2au: pValue = &data.mMoveZ; break;
        case 0xef02db64u: pValue = &data.mStartMoveZ; break;
        }
        if (pValue)
        {
            ReadVarianceValue(*pValue, line, 1.0f);
        }
    }
}

// FUNCTION: SWBF2 0x00618d00
void PE_ChunkLoader::LoadSpawner(ParticleSpawner& spawner, PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x14c8d3cau:
        {
            PblConfig scope(config.ReadNextScope(), true);
            LoadSpreadData(spawner.mOffset, scope);
            break;
        }
        case 0x494a00ceu:
        {
            spawner.mDirectionType = ParticleSpawner::SPAWN_SPREAD;
            PblConfig scope(config.ReadNextScope(), true);
            LoadSpreadData(spawner.mDirectionData.mSpreadData, scope);
            break;
        }
        case 0xd8760e19u:
        {
            spawner.mDirectionType = ParticleSpawner::SPAWN_VORTEX;
            PblConfig scope(config.ReadNextScope(), true);
            LoadRotationData(spawner.mDirectionData.mRotationData, scope);
            break;
        }
        case 0x28217089u:
        {
            spawner.mDirectionType = ParticleSpawner::SPAWN_CIRCLE;
            PblConfig scope(config.ReadNextScope(), true);
            LoadSpreadData(spawner.mDirectionData.mSpreadData, scope);
            break;
        }
        case 0xc091789au:
        {
            spawner.mDirectionType = ParticleSpawner::SPAWN_ROTATOR;
            PblConfig scope(config.ReadNextScope(), true);
            LoadRotationData(spawner.mDirectionData.mRotationData, scope);
            break;
        }
        case 0x9ff363c0u:
            ReadVarianceValue(spawner.mPositionDirectionScale, data, 1.0f);
            break;
        case 0x983c4168u:
            ReadVarianceValue(spawner.mVelocityDirectionScale, data, 1.0f);
            break;
        case 0xb9a4157cu:
            ReadVarianceValue(spawner.mInheritVelocityFactor, data, 1.0f);
            break;
        case 0x23a0d95cu:
            ReadVarianceValue(spawner.mSize, data, 1.0f);
            break;
        case 0x40f480dcu:
            spawner.mUseHSV = false;
            ReadVarianceValue(spawner.mRed, data, 1.0f / 255.0f);
            break;
        case 0xe38d6359u:
            spawner.mUseHSV = true;
            ReadVarianceValue(spawner.mRed, data, 1.0f / 255.0f);
            break;
        case 0x011decbcu:
        case 0xf5a2e289u:
            ReadVarianceValue(spawner.mGreen, data, 1.0f / 255.0f);
            break;
        case 0x82fbf5cdu:
        case 0x425ed3cau:
            ReadVarianceValue(spawner.mBlue, data, 1.0f / 255.0f);
            break;
        case 0x5d8b6dabu:
            ReadVarianceValue(spawner.mAlpha, data, 1.0f / 255.0f);
            break;
        case 0x5083ecd9u:
            spawner.mFadeInTime = data.GetFloatArg(0);
            break;
        case 0x3d4cef81u:
            ReadVarianceValue(spawner.mStartRotation, data, 1.0f);
            break;
        case 0x164b5138u:
            ReadVarianceValue(spawner.mRotVelocity, data, 1.0f);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00618730
ParticleEmitterClass* PE_ChunkLoader::Read(PblConfig& config)
{
    return LoadEmitter(config);
}

// FUNCTION: SWBF2 0x006187d0
ParticleEmitterClass* PE_ChunkLoader::LoadEmitter(PblConfig& config)
{
    ParticleEmitterClass* pClass = new ParticleEmitterClass();

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x826cd95eu:
            ReadVarianceValue(pClass->mStartDelay, data, 1.0f);
            break;
        case 0x78fc3314u:
            ReadVarianceValue(pClass->mBurstDelay, data, 1.0f);
            break;
        case 0x33cbb418u:
            ReadVarianceValue(pClass->mBurstCount, data, 1.0f);
            break;
        case 0x865d5c88u:
            ReadVarianceValue(pClass->mMaxParticles, data, 1.0f);
            if (CameraManager::Instance()->GetNumCameras() > 1 && pClass->mMaxParticles.GetMin() > 0.0f &&
                pClass->mMaxParticles.GetMax() != 0.0f)
            {
                float fOne = 1.0f;
                float fHalfMin = pClass->mMaxParticles.GetMin() * 0.5f;
                float fMin = util::Max_(fHalfMin, fOne);
                float fOne2 = 1.0f;
                float fHalfMax = pClass->mMaxParticles.mVariance * 0.5f + pClass->mMaxParticles.mValue * 0.5f;
                pClass->mMaxParticles.Setup(fMin, util::Max_(fHalfMax, fOne2));
            }
            break;
        case 0x40f480dcu:
            pClass->mUseHSV = false;
            ReadVarianceValue(pClass->mRed, data, 1.0f / 255.0f);
            break;
        case 0xe38d6359u:
            pClass->mUseHSV = true;
            ReadVarianceValue(pClass->mRed, data, 1.0f / 255.0f);
            break;
        case 0x011decbcu:
        case 0xf5a2e289u:
            ReadVarianceValue(pClass->mGreen, data, 1.0f / 255.0f);
            break;
        case 0x82fbf5cdu:
        case 0x425ed3cau:
            ReadVarianceValue(pClass->mBlue, data, 1.0f / 255.0f);
            break;
        case 0x5d8b6dabu:
            ReadVarianceValue(pClass->mAlpha, data, 1.0f / 255.0f);
            break;
        case 0x23a0d95cu:
            ReadVarianceValue(pClass->mSize, data, 1.0f);
            break;
        case 0x4936726fu:
        {
            PblConfig scope(config.ReadNextScope(), true);
            LoadSpawner(pClass->mSpawner, scope);
            break;
        }
        case 0xf1773bf8u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            LoadTransformer(pClass->mTransformer, scope);
            break;
        }
        case 0x2a6d8591u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            LoadGeometry(pClass->mGeometry, scope);
            break;
        }
        case 0xcebb215fu:
        {
            PblConfig scope(config.ReadNextScope(), true);
            pClass->mNext = LoadEmitter(scope);
            break;
        }
        case 0x6245ae9eu:
            pClass->mMinDistLOD = data.GetFloatArg(0);
            break;
        case 0xb9d61dccu:
            pClass->mMaxDistLOD = data.GetFloatArg(0);
            break;
        case 0xcbbfdf2bu:
        {
            float fRadius = data.GetFloatArg(0);
            pClass->mBoundingRadius = fRadius;
            if (fRadius > 20.0f)
            {
                pClass->mLodData = &largeEffectLodData;
            }
            else if (fRadius > 5.0f)
            {
                pClass->mLodData = &mediumEffectLodData;
            }
            else
            {
                pClass->mLodData = &smallEffectLodData;
            }
            break;
        }
        case 0xa7fc4de1u:
            pClass->mNoRegisterStep = true;
            break;
        case 0x33bbef79u:
            pClass->mWorldAlignedFlag = true;
            break;
        case 0x7a5ff18fu:
            if (data.GetNumArgs() == 1)
            {
                pClass->mAmbientSound.SetID(data.GetStringArg(0), false, false);
            }
            break;
        }
    }

    if (pClass->mMaxParticles.GetMin() == -1.0f && pClass->mMaxParticles.GetMax() == -1.0f &&
        pClass->mBurstDelay.GetMax() == 0.0f && pClass->mBurstDelay.GetMin() == 0.0f)
    {
        pClass->mBurstDelay.mValue = 0.1f;
        pClass->mBurstDelay.mVariance = 0.1f;
    }
    return pClass;
}

// FUNCTION: SWBF2 0x00618fc0
void PE_ChunkLoader::LoadTransformer(ParticleTransformer& transformer, PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        float fArg = 0.0f;
        if (data.GetNumArgs() > 0)
        {
            fArg = data.GetFloatArg(0);
        }

        switch (data.GetId())
        {
        case 0x23a0d95cu:
        {
            PblConfig scope(config.ReadNextScope(), true);
            int iIndex = (int)fArg;
            transformer.mSizeTrans[iIndex] = LoadSizeTrans(scope);
            if (iIndex + 1 > transformer.mNumSizeTrans)
            {
                transformer.mNumSizeTrans = iIndex + 1;
            }
            break;
        }
        case 0x21ac415fu:
        {
            PblConfig scope(config.ReadNextScope(), true);
            int iIndex = (int)fArg;
            transformer.mRotationTrans[iIndex] = LoadRotationTrans(scope);
            if (iIndex + 1 > transformer.mNumRotationTrans)
            {
                transformer.mNumRotationTrans = iIndex + 1;
            }
            break;
        }
        case 0x3d7e6258u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            int iIndex = (int)fArg;
            transformer.mColorTrans[iIndex] = LoadColorTrans(scope);
            if (iIndex + 1 > transformer.mNumColorTrans)
            {
                transformer.mNumColorTrans = iIndex + 1;
            }
            break;
        }
        case 0x934f4e0au:
        {
            PblConfig scope(config.ReadNextScope(), true);
            transformer.mPositionTrans = LoadPositionTrans(scope);
            break;
        }
        case 0x5212abeeu:
            transformer.mLifeTime = fArg;
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00619140
void PE_ChunkLoader::LoadGeometry(ParticleGeometry& geometry, PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x5127f14du:
            switch (PblHash(data.GetStringArg(0)))
            {
            case 0x596f63f3u:
                geometry.mType = ParticleGeometry::PT_STREAK;
                break;
            case 0x576b09cdu:
                geometry.mType = ParticleGeometry::PT_EMITTER;
                break;
            case 0x6f35dfbcu:
                geometry.mType = ParticleGeometry::PT_ANIMATED;
                break;
            case 0x2a6d8591u:
                geometry.mType = ParticleGeometry::PT_GEOMETRY;
                break;
            case 0xe506fa64u:
                geometry.mType = ParticleGeometry::PT_BILLBOARD;
                break;
            case 0x3f2f216au:
                geometry.mType = ParticleGeometry::PT_SPARK;
                break;
            case 0x8a8743bfu:
            default:
                geometry.mType = ParticleGeometry::PT_PARTICLE;
                break;
            }
            break;
        case 0xfa784eabu:
        {
            unsigned int uiMode = PblHash(data.GetStringArg(0));
            if (uiMode == 0x06a6a0a5u)
            {
                geometry.mBlendMode = BLEND_ADDITIVE;
            }
            else
            {
                geometry.mBlendMode = uiMode == 0x6bfbd198u ? BLEND_BLUR : BLEND_NORMAL;
            }
            break;
        }
        case 0x3c6468f4u:
        {
            geometry.mTextureHash = PblHash(data.GetStringArg(0));
            RedTexture* pTexture = RedTexture::Find(geometry.mTextureHash);
            geometry.mTextureHeightLog = 0;
            geometry.mTextureWidthLog = 0;
            if (pTexture)
            {
                int iSize = pTexture->GetWidth() >> 1;
                if (iSize > 0)
                {
                    int iLog = 0;
                    do
                    {
                        iSize >>= 1;
                        ++iLog;
                    } while (iSize > 0);
                    geometry.mTextureWidthLog = iLog;
                }
                iSize = pTexture->GetHeight() >> 1;
                if (iSize > 0)
                {
                    int iLog = 0;
                    do
                    {
                        iSize >>= 1;
                        ++iLog;
                    } while (iSize > 0);
                    geometry.mTextureHeightLog = iLog;
                }
            }
            break;
        }
        case 0xb08b665au:
            geometry.mTextureHash = PblHash(data.GetStringArg(0));
            break;
        case 0xcebb215fu:
        {
            PblConfig scope(config.ReadNextScope(), true);
            geometry.mEmitterClass = LoadEmitter(scope);
            break;
        }
        case 0x413495b0u:
            geometry.mTimePerFrame = data.GetFloatArg(0);
            break;
        case 0xfaf59ea7u:
        {
            int iSize = data.GetInt32Arg(0);
            geometry.mFrameSizeLog = 0.0f;
            for (iSize >>= 1; iSize > 0; iSize >>= 1)
            {
                geometry.mFrameSizeLog += 1.0f;
            }
            break;
        }
        case 0x55799c97u:
            geometry.mTotalFrames = data.GetFloatArg(0);
            break;
        case 0xfc1fd229u:
            geometry.mBlurValue = data.GetFloatArg(0);
            break;
        case 0x7971e4d6u:
            geometry.mBlurParam = data.GetInt32Arg(0);
            break;
        case 0x530aaa4eu:
            geometry.mSparkLength = data.GetFloatArg(0);
            break;
        case 0x7593992du:
            geometry.mLooping = data.GetFloatArg(0) != 0.0f;
            break;
        }
    }
    geometry.Setup();
}

// FUNCTION: SWBF2 0x00619600
ParticleTransformer::PositionTransformer* PE_ChunkLoader::LoadPositionTrans(PblConfig& config)
{
    ParticleTransformer::PositionTransformer* pTrans = new ParticleTransformer::PositionTransformer();
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x5212abeeu:
        {
            float fLifeTime = data.GetFloatArg(0);
            pTrans->mLifeTime = fLifeTime;
            pTrans->mTimeScale = fLifeTime > 0.0f ? 1.0f / fLifeTime : 0.0f;
            break;
        }
        case 0x5cb68de8u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            pTrans->mNextState = LoadPositionTrans(scope);
            break;
        }
        case 0x9d9f16f0u:
            pTrans->mData.mAccelerate.mAccX = data.GetFloatArg(0);
            pTrans->mData.mAccelerate.mAccY = data.GetFloatArg(1);
            pTrans->mType = ParticleTransformer::PositionTransformer::ACCELERATE;
            pTrans->mData.mAccelerate.mAccZ = data.GetFloatArg(2);
            break;
        case 0x4cfd5078u:
            pTrans->mData.mReach.mVelX = data.GetFloatArg(0);
            pTrans->mData.mReach.mVelY = data.GetFloatArg(1);
            pTrans->mType = ParticleTransformer::PositionTransformer::REACH;
            pTrans->mData.mReach.mVelZ = data.GetFloatArg(2);
            break;
        case 0x82971c71u:
            pTrans->mType = ParticleTransformer::PositionTransformer::SCALE;
            pTrans->mData.mScale.mScale = data.GetFloatArg(0);
            break;
        }
    }
    return pTrans;
}

// FUNCTION: SWBF2 0x00619790
ParticleTransformer::SizeTransformer* PE_ChunkLoader::LoadSizeTrans(PblConfig& config)
{
    ParticleTransformer::SizeTransformer* pTrans = new ParticleTransformer::SizeTransformer();
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x5212abeeu:
        {
            float fLifeTime = data.GetFloatArg(0);
            pTrans->mLifeTime = fLifeTime;
            pTrans->mTimeScale = fLifeTime > 0.0f ? 1.0f / fLifeTime : 0.0f;
            break;
        }
        case 0x5cb68de8u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            pTrans->mNextState = LoadSizeTrans(scope);
            break;
        }
        default:
            LoadDataTrans(pTrans->mData, data);
            break;
        }
    }
    return pTrans;
}

// FUNCTION: SWBF2 0x006198a0
ParticleTransformer::RotationTransformer* PE_ChunkLoader::LoadRotationTrans(PblConfig& config)
{
    ParticleTransformer::RotationTransformer* pTrans = new ParticleTransformer::RotationTransformer();
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x5212abeeu:
        {
            float fLifeTime = data.GetFloatArg(0);
            pTrans->mLifeTime = fLifeTime;
            pTrans->mTimeScale = fLifeTime > 0.0f ? 1.0f / fLifeTime : 0.0f;
            break;
        }
        case 0x5cb68de8u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            pTrans->mNextState = LoadRotationTrans(scope);
            break;
        }
        default:
            LoadDataTrans(pTrans->mData, data);
            break;
        }
    }
    return pTrans;
}

// FUNCTION: SWBF2 0x006199b0
ParticleTransformer::ColorTransformer* PE_ChunkLoader::LoadColorTrans(PblConfig& config)
{
    ParticleTransformer::ColorTransformer* pTrans = new ParticleTransformer::ColorTransformer();
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x184b0014u:
        case 0x4cfd5078u:
            pTrans->mType = data.GetId() == 0x4cfd5078u ? ParticleTransformer::ColorTransformer::REACH
                                                         : ParticleTransformer::ColorTransformer::MOVE;
            pTrans->mR = data.GetFloatArg(0) * (1.0f / 255.0f);
            pTrans->mG = data.GetFloatArg(1) * (1.0f / 255.0f);
            pTrans->mB = data.GetFloatArg(2) * (1.0f / 255.0f);
            pTrans->mA = data.GetFloatArg(3) * (1.0f / 255.0f);
            break;
        case 0x5212abeeu:
        {
            float fLifeTime = data.GetFloatArg(0);
            pTrans->mLifeTime = fLifeTime;
            pTrans->mTimeScale = fLifeTime > 0.0f ? 1.0f / fLifeTime : 0.0f;
            break;
        }
        case 0x5cb68de8u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            pTrans->mNextState = LoadColorTrans(scope);
            break;
        }
        }
    }
    return pTrans;
}

// FUNCTION: SWBF2 0x00619af0
void PE_ChunkLoader::LoadDataTrans(ParticleTransformer::DataTransformer& trans, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case 0x184b0014u:
        trans.mType = ParticleTransformer::DataTransformer::MOVE;
        trans.mData = data.GetFloatArg(0);
        break;
    case 0x4cfd5078u:
        trans.mType = ParticleTransformer::DataTransformer::REACH;
        trans.mData = data.GetFloatArg(0);
        break;
    case 0x82971c71u:
        trans.mType = ParticleTransformer::DataTransformer::SCALE;
        trans.mData = data.GetFloatArg(0);
        break;
    }
}

// FUNCTION: SWBF2 0x00618710
void ParticleEmitterEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}
