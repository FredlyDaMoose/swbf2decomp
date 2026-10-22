#pragma once

#include <stddef.h>

#include "RedEngineFL/Application/pc/pcRedParticle.h"

class Particle;
class ParticleEmitterClass;
class RedModel;

class ParticleGeometry
{
public:

    enum ParticleEmitterType
    {
        PT_PARTICLE = 0,
        PT_STREAK = 1,
        PT_EMITTER = 2,
        PT_ANIMATED = 3,
        PT_GEOMETRY = 4,
        PT_BILLBOARD = 5,
        PT_SPARK = 6
    };

    ParticleGeometry();

    ~ParticleGeometry() {}

    void Setup();

    void Render(Particle& particle, bool bLast) const;
    void Begin() const;

    static int mParticleCount;

    mutable bool mFirstTime;
    unsigned int mTextureHash;
    int mTextureWidthLog;
    int mTextureHeightLog;
    RedBlendMode mBlendMode;
    ParticleEmitterType mType;
    RedModel* m_pModel;
    ParticleEmitterClass* mEmitterClass;
    float mTimePerFrame;
    float mFrameSizeLog;
    float mTotalFrames;
    float mBlurValue;
    float mBlurRes;
    int mBlurParam;
    float mSparkLength;
    bool mLooping;
};
 
 
 
 
 
