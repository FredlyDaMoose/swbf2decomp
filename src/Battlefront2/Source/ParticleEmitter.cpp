#include "ParticleEmitter.h"
#include "Particle.h"
#include "ParticleEmitterObject.h"
#include "Battlefront2Lod.h"
#include "PebbleFL/Common/PblRandom.h"

extern NoLodData noloddata;

extern SmallEffectLodData smallEffectLodData;

// GLOBAL: SWBF2 0x01ef26c8
// SYNTHETIC: SWBF2 0x00405c60
// `dynamic initializer for 'ParticleEmitter::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769210
// `dynamic atexit destructor for 'ParticleEmitter::sMemoryPool''
MemoryPool ParticleEmitter::sMemoryPool("ParticleEmitter", 0x40);

// FUNCTION: SWBF2 0x00609ed0
ParticleEmitterClass::ParticleEmitterClass()
{
    mType = 1;
    mNext = 0;
    mLodData = &smallEffectLodData;
    mNoRegisterStep = false;
    mWorldAlignedFlag = false;
    mRed.Setup(1.0f);
    mGreen.Setup(1.0f);
    mBlue.Setup(1.0f);
    mAlpha.Setup(1.0f);
    mSize.Setup(1.0f);
    mNextSeed = 0;
    mMinDistLOD = 10.0f;
    mMaxDistLOD = 50.0f;
    mBoundingRadius = 5.0f;
}

// FUNCTION: SWBF2 0x00609fa0
ParticleEmitterClass::~ParticleEmitterClass()
{
    delete mNext;
}

// SYNTHETIC: SWBF2 0x00609ea0
// ParticleEmitterClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0060a010
FLEffectObject* ParticleEmitterClass::BuildEffect()
{
    ParticleEmitterObject* pObject = ParticleEmitterObject::Create(this);
    if (pObject)
    {
        pObject->ActivateEffect();
    }
    return pObject;
}

// FUNCTION: SWBF2 0x0060a030
ParticleEmitter::ParticleEmitter(const ParticleEmitterClass* pClass)
{
    mFirstInfo = 0;
    mNext = 0;
    mSoundStarted = 0;
    Reset(pClass);
}

// FUNCTION: SWBF2 0x00609d00
ParticleEmitter* ParticleEmitter::Create(const ParticleEmitterClass* pClass)
{
    if (ParticleEmitter::sMemoryPool.mUsed >= ParticleEmitter::sMemoryPool.mCount)
    {
        return 0;
    }
    return new ParticleEmitter(pClass);
}

// FUNCTION: SWBF2 0x00609c40
float VarianceValue::GetValue(PblRandom& rand) const
{
    if (mVariance == 0.0f)
    {
        return mValue;
    }
    return rand.GetFloat() * mVariance + mValue;
}

// FUNCTION: SWBF2 0x0060a130
void ParticleEmitter::Reset(const ParticleEmitterClass* pClass)
{
    while (mFirstInfo)
    {
        ParticleEmitterInfoData* pNext = mFirstInfo->mNext;
        mFirstInfo->Release();
        mFirstInfo = pNext;
    }
    delete mNext;

    mClass = (ParticleEmitterClass*)pClass;
    unsigned int uSeed = pClass->mNextSeed;
    if (uSeed == 0)
    {
        uSeed = PblRandom::GetGlobalInt();
    }
    mSeed = uSeed;

    PblRandom rand(uSeed ^ 0xC71C2830u);
    mMaxParticles = (short)mClass->mMaxParticles.GetValue(rand);
    if (mMaxParticles > 128)
    {
        mMaxParticles = 128;
    }
    mSpawnDelay = mClass->mStartDelay.GetValue(rand);
    mParticleCount = 0;
    mFirstInfo = 0;
    mStopTime = 10000.0f;
    mColor.r = mClass->mRed.GetValue(rand);
    mColor.g = mClass->mGreen.GetValue(rand);
    mColor.b = mClass->mBlue.GetValue(rand);
    mColor.a = mClass->mAlpha.GetValue(rand);
    mSize = mClass->mSize.GetValue(rand);
    mInheritVelocityFactor = 0.0f;
    mSoundStarted = 0;

    if (pClass->mNext)
    {
        mNext = ParticleEmitter::Create(pClass->mNext);
    }
    else
    {
        mNext = 0;
    }

    mSpawnerSeed = PblRandom::GetGlobalInt();
}

// FUNCTION: SWBF2 0x0060a090
ParticleEmitter::~ParticleEmitter()
{
    while (mFirstInfo)
    {
        ParticleEmitterInfoData* pNext = mFirstInfo->mNext;
        mFirstInfo->Release();
        mFirstInfo = pNext;
    }

    delete mNext;
}

// FUNCTION: SWBF2 0x0060a530
void ParticleEmitter::StopAndFinish(float fStopTime)
{
    if (mStopTime > fStopTime)
    {
        mStopTime = fStopTime;
    }
    if (mNext)
    {
        mNext->StopAndFinish(fStopTime);
    }
    mAmbientSound.Stop(true);
}

// FUNCTION: SWBF2 0x0060a560
void ParticleEmitter::SetColor(const RedColorValue& color)
{
    for (ParticleEmitter* p = this; p; p = p->mNext)
    {
        p->mColor = color;
    }
}

// FUNCTION: SWBF2 0x0060a4f0
void ParticleEmitter::Register(ParticleEmitterInfoData* pInfo)
{
    for (ParticleEmitter* p = this; p; p = p->mNext)
    {
        pInfo->AddRef();
        if (!p->mFirstInfo)
        {
            p->mFirstInfo = pInfo;
        }
    }
}

// FUNCTION: SWBF2 0x00609d80
ParticleEmitterObject::ParticleEmitterObject()
    : RedSceneObject(&noloddata)
{
    mbAddedToScene = true;
    mReflectionEnable = true;
}

// FUNCTION: SWBF2 0x00609e60
bool ParticleEmitterObject::IsReflected() const
{
    return mReflectionEnable;
}

// SYNTHETIC: SWBF2 0x00609e70
// ParticleEmitterObject::`scalar deleting destructor'

// GLOBAL: SWBF2 0x01ef4730
// SYNTHETIC: SWBF2 0x00405c80
// `dynamic initializer for 'sPEO''
// SYNTHETIC: SWBF2 0x00769220
// `dynamic atexit destructor for 'sPEO''
ParticleEmitterObject sPEO;

// FUNCTION: SWBF2 0x0060b8f0
void ParticleEmitter::Cleanup()
{
    sPEO.DeleteEmitter();
}

// FUNCTION: SWBF2 0x00609cb0
void ParticleEmitterClass::SetNextSeed(unsigned int uSeed)
{
    for (ParticleEmitterClass* p = this; p; p = p->mNext)
    {
        p->mNextSeed = uSeed;
    }
}

// GLOBAL: SWBF2 0x01ef2730
// SYNTHETIC: SWBF2 0x00405be0
// `dynamic initializer for 'sParticleCache''
Particle sParticleCache[128];

// GLOBAL: SWBF2 0x01ef4840
// SYNTHETIC: SWBF2 0x00405c20
// `dynamic initializer for 'sEmitterCache''
Particle sEmitterCache[32];

// SYNTHETIC: SWBF2 0x0048ebf0
// RedColorValue::RedColorValue
