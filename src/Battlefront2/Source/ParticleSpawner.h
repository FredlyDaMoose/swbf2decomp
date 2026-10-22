#pragma once

#include <stddef.h>

#include "VarianceValue.h"

class Particle;
class ParticleSpawnCache;
class PblMatrix;
class PblRandom;
class PblVector3;

class ParticleSpawner
{
public:
    enum SpawnType
    {
        SPAWN_NONE = 0,
        SPAWN_SPREAD = 1,
        SPAWN_VORTEX = 2,
        SPAWN_CIRCLE = 3,
        SPAWN_ROTATOR = 4
    };

    struct SpreadData
    {
        VarianceValue mPositionX;
        VarianceValue mPositionY;
        VarianceValue mPositionZ;
    };

    struct RotationData
    {
        VarianceValue mStartX;
        VarianceValue mStartMoveX;
        VarianceValue mMoveX;
        VarianceValue mStartY;
        VarianceValue mMoveY;
        VarianceValue mStartMoveY;
        VarianceValue mStartZ;
        VarianceValue mMoveZ;
        VarianceValue mStartMoveZ;
    };

    ParticleSpawner();

    ~ParticleSpawner() {}

    void SetupParticle(Particle& particle, const PblMatrix& mat, unsigned int uSeed, int iIndex, float fAge,
                       float fSize, const PblVector3& vVelocity, float fInheritVelocity,
                       bool bNoRegisterStep) const;
    void SetupLocalParticle(Particle& particle, PblRandom& rand, int iIndex) const;

    union
    {
        SpreadData mSpreadData;
        RotationData mRotationData;
    } mDirectionData;
    SpawnType mDirectionType;
    SpreadData mOffset;
    VarianceValue mPositionDirectionScale;
    VarianceValue mVelocityDirectionScale;
    VarianceValue mInheritVelocityFactor;
    int mNumSizes;
    VarianceValue mSize[1];
    bool mDirectionalColorFlag;
    int mNumColors;
    VarianceValue mRed[1];
    VarianceValue mGreen[1];
    VarianceValue mBlue[1];
    VarianceValue mAlpha[1];
    bool mUseHSV;
    float mFadeInTime;
    int mNumRotations;
    VarianceValue mStartRotation[1];
    VarianceValue mRotVelocity[1];
    ParticleSpawnCache* mCache;
    unsigned int mSeed;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 

void SetupRotationResult(PblVector3& vDir, const ParticleSpawner::RotationData& data,
                float fTime, int iIndex, PblRandom& rand);
