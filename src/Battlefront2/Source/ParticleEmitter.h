#pragma once

#include <stddef.h>

#include "FLEffect.h"
#include "GameSound.h"
#include "ParticleGeometry.h"
#include "ParticleSpawner.h"
#include "ParticleTransformer.h"
#include "VarianceValue.h"
#include "ParticleEmitterInfo.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class RedLodData;

// VTABLE: SWBF2 0x007ad2fc
class ParticleEmitterClass : public FLEffectClass
{
public:
    ParticleEmitterClass();

    virtual ~ParticleEmitterClass();
    virtual FLEffectObject* BuildEffect();

    void SetNextSeed(unsigned int uSeed);

    VarianceValue mStartDelay;
    VarianceValue mBurstDelay;
    VarianceValue mBurstCount;
    VarianceValue mMaxParticles;
    VarianceValue mRed;
    VarianceValue mGreen;
    VarianceValue mBlue;
    VarianceValue mAlpha;
    bool mUseHSV;
    VarianceValue mSize;
    unsigned int mNextSeed;
    ParticleSpawner mSpawner;
    ParticleTransformer mTransformer;
    ParticleGeometry mGeometry;
    RedLodData* mLodData;
    float mMinDistLOD;
    float mMaxDistLOD;
    float mBoundingRadius;
    bool mNoRegisterStep;
    bool mWorldAlignedFlag;
    ParticleEmitterClass* mNext;
    unsigned int mSoundNameHash;
    GameSound mAmbientSound;
};
 
 
 
 
 
 
 
 
 
 
 
 

class ParticleEmitter
{
public:
    ParticleEmitter(const ParticleEmitterClass* pClass);
    ~ParticleEmitter();

    static void* operator new(size_t size)
    {
        return sMemoryPool.Allocate((unsigned int)size);
    }

    static void operator delete(void* p)
    {
        sMemoryPool.Free(p);
    }

    void Reset(const ParticleEmitterClass* pClass);
    void StopAndFinish(float fStopTime);

    void Register(ParticleEmitterInfoData* pInfo);
    ParticleEmitter* DeleteAndReturnNext();
    bool Update(float fTime);

    void Render(float fTime);
    void RenderAsEmitter(float fTime);
    void SetSpawnerSeed(unsigned int uSeed);

    void SetColor(const RedColorValue& color);

    void ScaleMultiply(float fScale)
    {
        for (ParticleEmitter* p = this; p; p = p->mNext)
        {
            p->mSize *= fScale;
        }
    }
    void SetInheritVelocityFactor(float fFactor)
    {
        for (ParticleEmitter* p = this; p; p = p->mNext)
        {
            p->mInheritVelocityFactor = fFactor;
        }
    }

    short mParticleCount;
    short mMaxParticles;
    float mSpawnDelay;
    RedColorValue mColor;
    float mSize;
    float mInheritVelocityFactor;
    ParticleEmitterClass* mClass;
    unsigned int mSeed;
    unsigned int mSpawnerSeed;
    ParticleEmitterInfoData* mFirstInfo;
    float mStopTime;
    ParticleEmitter* mNext;
    GameSoundControllable mAmbientSound;
    unsigned char mSoundStarted;

    static MemoryPool sMemoryPool;

    static void Cleanup();
    static ParticleEmitter* Create(const ParticleEmitterClass* pClass);
};

 
 
 
 
 
