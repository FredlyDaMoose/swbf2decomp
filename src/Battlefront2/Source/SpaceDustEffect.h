#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblVector.h"

// VTABLE: SWBF2 0x007af278
class SpaceDustEffect : public FLEffect
{
public:

    struct ParticleData
    {
        ParticleData() { mLifeLeft = -1.0f; }

        PblVector3 mCoord;
        float mLifeLeft;
        float mScale;
    };

    struct DustEmitterData
    {

        DustEmitterData();

        ~DustEmitterData();

        void Init(bool bFreeParticles, bool bAllocParticles);

        bool mEnabled;
        float mTimeSinceLastSpawned;
        float mParticleLife;
        float mMaxParticles;
        ParticleData* mParticle;
    };

    SpaceDustEffect();

    virtual ~SpaceDustEffect();

    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Init();

    static void Init(bool bAllocate);
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    static void Start(int iEmitter);
    static void Stop(int iEmitter);

    static SpaceDustEffect* s_pInstance;

    static DustEmitterData m_emitter[1];

    static float mSpawnDistance;
    static float mMaxRandomSideOffset;
    static float mCenterDeadZoneRadius;
    static float mSpawnDelay;
    static float mReferenceSpeed;
    static float mDustParticleSpeed;
    static unsigned int mTextureHash;
    static float mMinParticleScale;
    static float mMaxParticleScale;
    static float mSpeedParticleMinLength;
    static float mSpeedParticleMaxLength;
    static float mParticleLengthMinSpeed;
    static float mParticleLengthMaxSpeed;
};
 
 
 
 
 
 
 